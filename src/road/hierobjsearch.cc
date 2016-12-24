#include "hierobjsearch.h"
#include "hiergraph.h"
#include "bordernode.h"
#include "shortcuttreenode.h"
#include "edge.h"
#include "nodemap.h"
#include "graphmap.h"
#include "graphplot.h"
#include <math.h>

void findPath(BorderNode* node, GraphMapping& a_gmap, Array& links)
{
    Stack s;
    s.push((void*)&node->m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        for (int i=0; i<a->size(); i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            if (sc->m_subnetid == 0)
            {
                for (int j=0; j<sc->m_edges.size(); j++)
                    links.append(sc->m_edges.get(j));
            }
            else
            {
                const Array* obj = a_gmap.findObject(sc->m_subnetid);
                if (sc->m_edges.size() > 0 && obj == 0)
                {
                    for (int j=0; j<sc->m_edges.size(); j++)
                        links.append(sc->m_edges.get(j));
                }
                else
                {
                    s.push(&sc->m_child);
                }
            }

        }
    }
}

//-----------------------------------------------------------------------------
// single point range search
//-----------------------------------------------------------------------------
void HierObjectSearch::rangeSearch(HierGraph& a_graph,
                                   NodeMapping& a_nmap, GraphMapping& a_gmap,
                                   const int a_src, const float a_range,
                                   Array& a_result,
                                   int& a_nodeaccess, int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    //-------------------------------------------------------------------------
    // Dijkstra's shortest path search (best first)
    //-------------------------------------------------------------------------
    Set visited;
    Hash pending;
    BinHeap h(ObjectSearchResult::compare);
    h.insert(new ObjectSearchResult(a_src,0));
    while (!h.isEmpty())
    {
        ObjectSearchResult* c = (ObjectSearchResult*)h.removeTop();

        //---------------------------------------------------------------------
        // Check if the node is beyond the search range. If so, terminate!!!
        //---------------------------------------------------------------------
        if (c->m_cost > a_range)
        {
            delete c;
            break;
        }

        //---------------------------------------------------------------------
        // Check if the node is visited. If so, skip it.
        //---------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }
        a_nodeaccess++;

        visited.insert((void*)c->m_nid);

        BorderNode* bnode = a_graph.getBorderNode(c->m_nid);
        Array links;
        findPath(bnode, a_gmap, links);
        for (int e=0; e<links.size(); e++)
        {
            Edge* edge = (Edge*)links.get(e);

            float cost = c->m_cost + edge->m_cost;
            int idist = (long)pending.get(edge->m_neighbor);
            if (idist != 0 && idist < floor(cost * 1000))  // there exists a same node is the queue
                continue;

            h.insert(
                new ObjectSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            idist = (int)ceil(cost * 1000);
            pending.put(edge->m_neighbor, (void*)idist);
            a_edgeaccess++;
        }
        delete bnode;

        // --------------------------------------------------------------------
        // check if object is found
        // --------------------------------------------------------------------
        const Array* objs = a_nmap.findObject(c->m_nid);
        if (objs != 0)
        {
            c->addObjects(*objs);
            a_result.append(c);
        }
        else
            delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (ObjectSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return;
}

//-----------------------------------------------------------------------------
// single point kNN search
//-----------------------------------------------------------------------------
void HierObjectSearch::kNNSearch(HierGraph& a_graph,
                                 NodeMapping& a_nmap, GraphMapping& a_gmap,
                                 const int a_src, const int k,
                                 Array& a_result,
                                 int& a_nodeaccess, int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    int rescnt = 0;
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    //-------------------------------------------------------------------------
    // Dijkstra's shortest path search (best first)
    //-------------------------------------------------------------------------
    Set visited;
    Hash pending;
    BinHeap h(ObjectSearchResult::compare);
    h.insert(new ObjectSearchResult(a_src,0));
    while (!h.isEmpty())
    {
        ObjectSearchResult* c = (ObjectSearchResult*)h.removeTop();

        //---------------------------------------------------------------------
        // Check if the node is visited. If so, skip it.
        //---------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }
        a_nodeaccess++;

        visited.insert((void*)c->m_nid);

        BorderNode* bnode = a_graph.getBorderNode(c->m_nid);
        Array links;
        findPath(bnode, a_gmap, links);
        for (int e=0; e<links.size(); e++)
        {
            Edge* edge = (Edge*)links.get(e);

            float cost = c->m_cost + edge->m_cost;
            int idist = (long)pending.get(edge->m_neighbor);
            if (idist != 0 && idist < floor(cost * 1000))  // there exists a same node is the queue
                continue;

            h.insert(
                new ObjectSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            idist = (long)ceil(cost * 1000);
            pending.put(edge->m_neighbor, (void*)idist);
            a_edgeaccess++;
        }
        delete bnode;

        // --------------------------------------------------------------------
        // check if object is found
        // --------------------------------------------------------------------
        const Array* objs = a_nmap.findObject(c->m_nid);
        if (objs != 0)
        {
            c->addObjects(*objs);
            a_result.append(c);
            rescnt += objs->size();
            if (rescnt >= k)   // once k objects are found, terminate!
                break;
        }
        else
            delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (ObjectSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return;
}

//-----------------------------------------------------------------------------
// single point kNN search with trace
//-----------------------------------------------------------------------------
void HierObjectSearch::kNNSearch(HierGraph& a_graph,
                                 NodeMapping& a_nmap, GraphMapping& a_gmap,
                                 const int a_src, const int k,
                                 Array& a_result,
                                 int& a_nodeaccess, int& a_edgeaccess,
                                 Array& a_visited)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    int rescnt = 0;
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    //-------------------------------------------------------------------------
    // Dijkstra's shortest path search (best first)
    //-------------------------------------------------------------------------
    Set visited;
    Hash pending;
    BinHeap h(ObjectSearchResult::compare);
    h.insert(new ObjectSearchResult(a_src,0));
    while (!h.isEmpty())
    {
        ObjectSearchResult* c = (ObjectSearchResult*)h.removeTop();

        //---------------------------------------------------------------------
        // Check if the node is visited. If so, skip it.
        //---------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }
        a_nodeaccess++;

        visited.insert((void*)c->m_nid);
        a_visited.append((void*)c->m_nid);

        BorderNode* bnode = a_graph.getBorderNode(c->m_nid);
        Array links;
        findPath(bnode, a_gmap, links);
        for (int e=0; e<links.size(); e++)
        {
            Edge* edge = (Edge*)links.get(e);

            float cost = c->m_cost + edge->m_cost;
            int idist = (long)pending.get(edge->m_neighbor);
            if (idist != 0 && idist < floor(cost * 1000))  // there exists a same node is the queue
                continue;

            h.insert(
                new ObjectSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            idist = (int)ceil(cost * 1000);
            pending.put(edge->m_neighbor, (void*)idist);
            a_edgeaccess++;
        }
        delete bnode;

        // --------------------------------------------------------------------
        // check if object is found
        // --------------------------------------------------------------------
        const Array* objs = a_nmap.findObject(c->m_nid);
        if (objs != 0)
        {
            c->addObjects(*objs);
            a_result.append(c);
            rescnt += objs->size();
            if (rescnt >= k)   // once k objects are found, terminate!
                break;
        }
        else
            delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (ObjectSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return;
}

void updateQueryMap(GraphMapping& a_qmap, GraphMapping& a_gmap,
                    BorderNode* a_node, const int a_q, const int a_cnt,
                    Array& a_hits)
{
    Stack s;
    s.push(&a_node->m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        for (int i=0; i<a->size(); i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            if (sc->m_subnetid != 0 && a_gmap.findObject(sc->m_subnetid) != 0)
            {
                Array* query = 0;
                query = (Array*)a_qmap.findObject(sc->m_subnetid);
                int oldcnt = query == 0 ? 0 : query->size();
                if (oldcnt != a_cnt)
                {
                    a_qmap.addObject(sc->m_subnetid,a_q);   // associate q to subnet
                    query = (Array*)a_qmap.findObject(sc->m_subnetid);
                    int newcnt = query == 0 ? 0 : query->size();
                    if (newcnt > oldcnt && newcnt == a_cnt)
                        a_hits.append((void*)sc->m_subnetid);
                }
            }
            s.push(&sc->m_child);
        }
    }
}

void findPath(BorderNode* a_node,
              GraphMapping& a_gmap, GraphMapping& a_qmap,
              const int a_cnt,
              Array& a_links, Array& a_wait)
{
    Stack s;
    s.push(&a_node->m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        for (int i=0; i<a->size(); i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            if (sc->m_subnetid == 0)
            {
                for (int j=0; j<sc->m_edges.size(); j++)
                    a_links.append(sc->m_edges.get(j));
            }
            else
            {
                if (sc->m_edges.size() > 0)
                {
                    Array* objs = (Array*)a_gmap.findObject(sc->m_subnetid);
                    if (objs == 0)
                    {
                        for (int j=0; j<sc->m_edges.size(); j++)
                            a_links.append(sc->m_edges.get(j));
                    }
                    else
                    {
                        Array* query = (Array*)a_qmap.findObject(sc->m_subnetid);
                        if (query->size() < a_cnt)
                        {
                            for (int j=0; j<sc->m_edges.size(); j++)
                                a_links.append(sc->m_edges.get(j));
                            a_wait.append((void*)sc->m_subnetid);
                        }
                        else
                        {
                            s.push(&sc->m_child);
                        }
                    }
                        
                    /*
                    Array* objs = (Array*)a_gmap.findObject(sc->m_subnetid);
                    if (objs != 0)
                    {   // go down for objects
                        Array* query = (Array*)a_qmap.findObject(sc->m_subnetid);
                        if (query->size() == a_cnt)
                        {
                            for (int j=0; j<sc->m_edges.size(); j++)
                                a_links.append(sc->m_edges.get(j));
                            a_wait.append((void*)sc->m_subnetid);
                        }
                        else
                            s.push(&sc->m_child);
                    }
                    else
                        s.push(&sc->m_child);
                    */
                }
                else
                    s.push(&sc->m_child);
            }
        }
    }
}


// ----------------------------------------------------------------------------
// multi-point range search
// ----------------------------------------------------------------------------
void HierObjectSearch::groupRangeSearch(HierGraph& a_graph,
                                        NodeMapping& a_nmap, GraphMapping& a_gmap,
                                        const int* a_src, const float* a_range,
                                        const int a_cnt,
                                        Array& a_result,
                                        int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // data structure for queuing nodes
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_q;
        const int   m_nid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_q, const int a_nid, const float a_cost):
            m_q(a_q), m_nid(a_nid), m_cost(a_cost)
            {
                m_path.append((void*)m_nid);
            };
        carrier(const int a_q, const int a_nid, const float a_cost, const Array& a_path):
            m_q(a_q), m_nid(a_nid), m_cost(a_cost), m_path(a_path)
            {
                m_path.append((void*)m_nid);
            };
        virtual ~carrier(){};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            return 0;
        }
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    GraphMapping qmap;  // associate query in the hierarchical graph
    Hash standby;       // a collection of border nodes that are pending for objects
    Hash foundobjs;     // identified objects
    BinHeap h(carrier::compare);
    Hash** pending = new Hash*[a_cnt];
    Set** visited = new Set*[a_cnt];
    for (int i=0; i<a_cnt; i++)
    {
        visited[i] = new Set(1000);
        pending[i] = new Hash(1000);
    }

    // ------------------------------------------------------------------------
    // start the search at individual query points
    // ------------------------------------------------------------------------
    for (int i=0; i<a_cnt; i++)
        h.insert(new carrier(i, a_src[i], 0));

    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();

        // --------------------------------------------------------------------
        // skip the expansion if the node is beyond the search range
        // --------------------------------------------------------------------
        if (c->m_cost > a_range[c->m_q])
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // skip the expansion if the node has been already visited
        // --------------------------------------------------------------------
        if (visited[c->m_q]->in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // mark the node visited
        // --------------------------------------------------------------------
        visited[c->m_q]->insert((void*)c->m_nid);

        // --------------------------------------------------------------------
        // exploring the node
        // --------------------------------------------------------------------
        BorderNode* bnode = a_graph.getBorderNode(c->m_nid);
        a_nodeaccess++;
        
        // --------------------------------------------------------------------
        // update query map
        // --------------------------------------------------------------------
        Array hits;
        updateQueryMap(qmap, a_gmap, bnode, c->m_q, a_cnt, hits);
        
        // --------------------------------------------------------------------
        // fire pending traversal
        // --------------------------------------------------------------------
        for (int i=0; i<hits.size(); i++)
        {
            int subnetid = (long)hits.get(i);
            Array* queued = (Array*)standby.get(subnetid);
            if (queued != 0)
            {
                for (int j=0; j<queued->size(); j++)
                {
                    carrier* newc = (carrier*)queued->get(j);
                    visited[newc->m_q]->remove((void*)newc->m_nid);
                    h.insert(newc);
                }
                queued->clean();
                delete queued;
                standby.remove(subnetid);
            }
        }

        // --------------------------------------------------------------------
        // expanding the search range
        // --------------------------------------------------------------------
        Array links;
        Array wait;
        findPath(bnode, a_gmap, qmap, a_cnt, links, wait);
        for (int i=0; i<links.size(); i++)
        {
            Edge* edge = (Edge*)links.get(i);

            float cost = c->m_cost + edge->m_cost;
            int idist = (long)pending[c->m_q]->get(edge->m_neighbor);
            if (idist != 0 && idist < floor(cost * 1000))  // there exists a same node is the queue
                continue;

            h.insert(new carrier(c->m_q, edge->m_neighbor, c->m_cost + edge->m_cost, c->m_path));
            idist = (int)ceil(cost * 1000);
            pending[c->m_q]->put(edge->m_neighbor, (void*)idist);
            a_edgeaccess++;
        }

        // --------------------------------------------------------------------
        // expanding the search range
        // --------------------------------------------------------------------
        for (int i=0; i<wait.size(); i++)
        {
            int subnetid = (long)wait.get(i);
            Array* queued = (Array*)standby.get(subnetid);
            if (queued == 0)
                standby.put(subnetid, queued = new Array);
            queued->append(new carrier(c->m_q, c->m_nid, c->m_cost, c->m_path));
        }
        delete bnode;

        // --------------------------------------------------------------------
        // keep the asscoiated objects if any
        // --------------------------------------------------------------------
        const Array* objs = a_nmap.findObject(c->m_nid);
        for (int j=0; objs!=0 && j<objs->size(); j++)
        {
            int oid = (long)objs->get(j);
            GroupObjectSearchResult* obj =
                (GroupObjectSearchResult*)foundobjs.get(oid);
            if (obj == 0)
                foundobjs.put(oid, obj =
                new GroupObjectSearchResult(c->m_nid, oid, a_cnt));
            obj->m_cost[c->m_q] = c->m_cost;
            obj->m_path[c->m_q].copy(c->m_path);
            if (obj->allreached(a_cnt))
            {
                a_result.append(obj);
                foundobjs.remove(oid);
            }
        }

        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    for (HashReader rdr(standby); !rdr.isEnd(); rdr.next())
    {
        Array* queued = (Array*)rdr.getVal();
        if (queued != 0)
        {
            for (int i=0; i<queued->size(); i++)
                delete (carrier*)queued->get(i);
            delete queued;
        }
    }
    standby.clean();

    for (HashReader rdr(foundobjs); !rdr.isEnd(); rdr.next())
        delete (GroupObjectSearchResult*)rdr.getVal();

    for (int i=0; i<a_cnt; i++)
    {
        delete visited[i];
        delete pending[i];
    }
    delete[] visited;
    delete[] pending;
}

// ----------------------------------------------------------------------------
// multi-point kNN search
// ----------------------------------------------------------------------------
void HierObjectSearch::groupKNNSearch(HierGraph& a_graph,
                                      NodeMapping& a_nmap, GraphMapping& a_gmap,
                                      const int* a_src, const int a_cnt,
                                      const int a_k,
                                      Array& a_result,
                                      int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // data structure for queuing nodes
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_q;
        const int   m_nid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_q, const int a_nid, const float a_cost):
            m_q(a_q), m_nid(a_nid), m_cost(a_cost)
            {
                m_path.append((void*)m_nid);
            };
        carrier(const int a_q, const int a_nid, const float a_cost, const Array& a_path):
            m_q(a_q), m_nid(a_nid), m_cost(a_cost), m_path(a_path)
            {
                m_path.append((void*)m_nid);
            };
        virtual ~carrier(){};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            return 0;
        }
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    GraphMapping qmap;   // associate query in the hierarchical graph
    Hash standby;
    Hash foundobjs;
    BinHeap h(carrier::compare);
    Hash** pending = new Hash*[a_cnt];
    Set** visited = new Set*[a_cnt];
    for (int i=0; i<a_cnt; i++)
    {
        visited[i] = new Set(1000);
        pending[i] = new Hash(1000);
    }

    // ------------------------------------------------------------------------
    // start the search at individual query points
    // ------------------------------------------------------------------------
    for (int i=0; i<a_cnt; i++)
        h.insert(new carrier(i, a_src[i], 0));

    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();

        // --------------------------------------------------------------------
        // if k answer objects are found, skip it and the rest
        // --------------------------------------------------------------------
        if (a_result.size() >= a_k)
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // skip the expansion if the node has been already visited
        // --------------------------------------------------------------------
        if (visited[c->m_q]->in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // mark the node visited
        // --------------------------------------------------------------------
        visited[c->m_q]->insert((void*)c->m_nid);

        // --------------------------------------------------------------------
        // exploring the node
        // --------------------------------------------------------------------
        BorderNode* bnode = a_graph.getBorderNode(c->m_nid);
        a_nodeaccess++;
        
        // --------------------------------------------------------------------
        // update query map
        // --------------------------------------------------------------------
        Array hits;
        updateQueryMap(qmap, a_gmap, bnode, c->m_q, a_cnt, hits);
        
        // --------------------------------------------------------------------
        // fire pending traversal
        // --------------------------------------------------------------------
        for (int i=0; i<hits.size(); i++)
        {
            int subnetid = (long)hits.get(i);
            Array* queued = (Array*)standby.get(subnetid);
            if (queued != 0)
            {
                for (int j=0; j<queued->size(); j++)
                {
                    carrier* newc = (carrier*)queued->get(j);
                    visited[newc->m_q]->remove((void*)newc->m_nid);
                    h.insert(newc);
                }
                queued->clean();
                delete queued;
                standby.remove(subnetid);
            }
        }

        // --------------------------------------------------------------------
        // expanding the search range
        // --------------------------------------------------------------------
        Array links;
        Array wait;
        findPath(bnode, a_gmap, qmap, a_cnt, links, wait);
        for (int i=0; i<links.size(); i++)
        {
            Edge* edge = (Edge*)links.get(i);

            float cost = c->m_cost + edge->m_cost;
            int idist = (long)pending[c->m_q]->get(edge->m_neighbor);
            if (idist != 0 && idist < floor(cost * 1000))  // there exists a same node is the queue
                continue;

            h.insert(new carrier(c->m_q, edge->m_neighbor, c->m_cost + edge->m_cost, c->m_path));
            idist = (int)ceil(cost * 1000);
            pending[c->m_q]->put(edge->m_neighbor, (void*)idist);
            a_edgeaccess++;
        }

        // --------------------------------------------------------------------
        // expanding the search range
        // --------------------------------------------------------------------
        for (int i=0; i<wait.size(); i++)
        {
            int subnetid = (long)wait.get(i);
            Array* queued = (Array*)standby.get(subnetid);
            if (queued == 0)
                standby.put(subnetid, queued = new Array);
            queued->append(new carrier(c->m_q, c->m_nid, c->m_cost, c->m_path));
        }
        delete bnode;

        // --------------------------------------------------------------------
        // keep the asscoiated objects if any
        // --------------------------------------------------------------------
        const Array* objs = a_nmap.findObject(c->m_nid);
        for (int j=0; objs!=0 && j<objs->size(); j++)
        {
            int oid = (long)objs->get(j);
            GroupObjectSearchResult* obj =
                (GroupObjectSearchResult*)foundobjs.get(oid);
            if (obj == 0)
                foundobjs.put(oid, obj =
                new GroupObjectSearchResult(c->m_nid, oid, a_cnt));
            obj->m_cost[c->m_q] = c->m_cost;
            obj->m_path[c->m_q].copy(c->m_path);
            if (obj->allreached(a_cnt))
            {
                a_result.append(obj);
                foundobjs.remove(oid);
            }
        }

        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    for (HashReader rdr(standby); !rdr.isEnd(); rdr.next())
    {
        Array* queued = (Array*)rdr.getVal();
        if (queued != 0)
        {
            for (int i=0; i<queued->size(); i++)
                delete (carrier*)queued->get(i);
            delete queued;
        }
    }
    standby.clean();
    
    for (HashReader rdr(foundobjs); !rdr.isEnd(); rdr.next())
        delete (GroupObjectSearchResult*)rdr.getVal();

    for (int i=0; i<a_cnt; i++)
    {
        delete visited[i];
        delete pending[i];
    }
    delete[] visited;
    delete[] pending;
}


void HierObjectSearch::groupKNNSearch(HierGraph& a_graph,
                                      NodeMapping& a_nmap, GraphMapping& a_gmap,
                                      const int* a_src, const int a_cnt,
                                      const int a_k,
                                      Array& a_result,
                                      int& a_nodeaccess, int& a_edgeaccess,
                                      Array& a_visited)
{
    // ------------------------------------------------------------------------
    // data structure for queuing nodes
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_q;
        const int   m_nid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_q, const int a_nid, const float a_cost):
            m_q(a_q), m_nid(a_nid), m_cost(a_cost)
            {
                m_path.append((void*)m_nid);
            };
        carrier(const int a_q, const int a_nid, const float a_cost, const Array& a_path):
            m_q(a_q), m_nid(a_nid), m_cost(a_cost), m_path(a_path)
            {
                m_path.append((void*)m_nid);
            };
        virtual ~carrier(){};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            return 0;
        }
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    GraphMapping qmap;   // associate query in the hierarchical graph
    Hash standby;
    Hash foundobjs;
    BinHeap h(carrier::compare);
    Hash** pending = new Hash*[a_cnt];
    Set** visited = new Set*[a_cnt];
    for (int i=0; i<a_cnt; i++)
    {
        visited[i] = new Set(1000);
        pending[i] = new Hash(1000);
    }

    // ------------------------------------------------------------------------
    // start the search at individual query points
    // ------------------------------------------------------------------------
    for (int i=0; i<a_cnt; i++)
        h.insert(new carrier(i, a_src[i], 0));

    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();

        // --------------------------------------------------------------------
        // if k answer objects are found, skip it and the rest
        // --------------------------------------------------------------------
        if (a_result.size() >= a_k)
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // skip the expansion if the node has been already visited
        // --------------------------------------------------------------------
        if (visited[c->m_q]->in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // mark the node visited
        // --------------------------------------------------------------------
        bool inserted = false;
        for (int i=0; i<a_cnt && !inserted; i++)
        {
            if (visited[i]->in((void*)c->m_nid))
                inserted = true;
        }
        if (!inserted) a_visited.append((void*)c->m_nid);

        visited[c->m_q]->insert((void*)c->m_nid);


        // --------------------------------------------------------------------
        // exploring the node
        // --------------------------------------------------------------------
        BorderNode* bnode = a_graph.getBorderNode(c->m_nid);
        a_nodeaccess++;
        
        // --------------------------------------------------------------------
        // update query map
        // --------------------------------------------------------------------
        Array hits;
        updateQueryMap(qmap, a_gmap, bnode, c->m_q, a_cnt, hits);
        
        // --------------------------------------------------------------------
        // fire pending traversal
        // --------------------------------------------------------------------
        for (int i=0; i<hits.size(); i++)
        {
            int subnetid = (long)hits.get(i);
            Array* queued = (Array*)standby.get(subnetid);
            if (queued != 0)
            {
                for (int j=0; j<queued->size(); j++)
                {
                    carrier* newc = (carrier*)queued->get(j);
                    visited[newc->m_q]->remove((void*)newc->m_nid);
                    h.insert(newc);
                }
                queued->clean();
                delete queued;
                standby.remove(subnetid);
            }
        }

        // --------------------------------------------------------------------
        // expanding the search range
        // --------------------------------------------------------------------
        Array links;
        Array wait;
        findPath(bnode, a_gmap, qmap, a_cnt, links, wait);
        for (int i=0; i<links.size(); i++)
        {
            Edge* edge = (Edge*)links.get(i);

            float cost = c->m_cost + edge->m_cost;
            int idist = (long)pending[c->m_q]->get(edge->m_neighbor);
            if (idist != 0 && idist < floor(cost * 1000))  // there exists a same node is the queue
                continue;

            h.insert(new carrier(c->m_q, edge->m_neighbor, c->m_cost + edge->m_cost, c->m_path));
            idist = (int)ceil(cost * 1000);
            pending[c->m_q]->put(edge->m_neighbor, (void*)idist);
            a_edgeaccess++;
        }

        // --------------------------------------------------------------------
        // expanding the search range
        // --------------------------------------------------------------------
        for (int i=0; i<wait.size(); i++)
        {
            int subnetid = (long)wait.get(i);
            Array* queued = (Array*)standby.get(subnetid);
            if (queued == 0)
                standby.put(subnetid, queued = new Array);
            queued->append(new carrier(c->m_q, c->m_nid, c->m_cost, c->m_path));
        }
        delete bnode;

        // --------------------------------------------------------------------
        // keep the asscoiated objects if any
        // --------------------------------------------------------------------
        const Array* objs = a_nmap.findObject(c->m_nid);
        for (int j=0; objs!=0 && j<objs->size(); j++)
        {
            int oid = (long)objs->get(j);
            GroupObjectSearchResult* obj =
                (GroupObjectSearchResult*)foundobjs.get(oid);
            if (obj == 0)
                foundobjs.put(oid, obj =
                new GroupObjectSearchResult(c->m_nid, oid, a_cnt));
            obj->m_cost[c->m_q] = c->m_cost;
            obj->m_path[c->m_q].copy(c->m_path);
            if (obj->allreached(a_cnt))
            {
                a_result.append(obj);
                foundobjs.remove(oid);
            }
        }

        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    for (HashReader rdr(standby); !rdr.isEnd(); rdr.next())
    {
        Array* queued = (Array*)rdr.getVal();
        if (queued != 0)
        {
            for (int i=0; i<queued->size(); i++)
                delete (carrier*)queued->get(i);
            delete queued;
        }
    }
    standby.clean();
    
    for (HashReader rdr(foundobjs); !rdr.isEnd(); rdr.next())
        delete (GroupObjectSearchResult*)rdr.getVal();

    for (int i=0; i<a_cnt; i++)
    {
        delete visited[i];
        delete pending[i];
    }
    delete[] visited;
    delete[] pending;
}
