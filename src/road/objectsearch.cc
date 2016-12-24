/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "objectsearch.h"
#include "graph.h"
#include "node.h"
#include "edge.h"
#include "nodemap.h"
#include "graphplot.h"

//-----------------------------------------------------------------------------
// single point range search
//-----------------------------------------------------------------------------
void ObjectSearch::rangeSearch(Graph& a_graph, NodeMapping& a_map,
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
    Set visited(1000);
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

        Node* node = a_graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new ObjectSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        delete node;

        // --------------------------------------------------------------------
        // check if object is found
        // --------------------------------------------------------------------
        const Array* objs = a_map.findObject(c->m_nid);
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
void ObjectSearch::kNNSearch(Graph& a_graph, NodeMapping& a_map,
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
    Set visited(1000);
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

        Node* node = a_graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new ObjectSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        delete node;

        // --------------------------------------------------------------------
        // check if object is found
        // --------------------------------------------------------------------
        const Array* objs = a_map.findObject(c->m_nid);
        if (objs != 0)
        {
            c->addObjects(*objs);
            a_result.append(c);
            rescnt += objs->size();
            if (rescnt >= k)   // once k objects are found, terminate!
                break;
        }
        else
        {
            delete c;
        }
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
void ObjectSearch::kNNSearch(Graph& a_graph, NodeMapping& a_map,
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
    Set visited(1000);
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

        Node* node = a_graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new ObjectSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        delete node;

        // --------------------------------------------------------------------
        // check if object is found
        // --------------------------------------------------------------------
        const Array* objs = a_map.findObject(c->m_nid);
        if (objs != 0)
        {
            c->addObjects(*objs);
            a_result.append(c);
            rescnt += objs->size();
            if (rescnt >= k)   // once k objects are found, terminate!
                break;
        }
        else
        {
            delete c;
        }
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
// multiple point range search
//-----------------------------------------------------------------------------
void ObjectSearch::groupRangeSearch(Graph& a_graph, NodeMapping& a_map,
                                    const int* a_src, const float* a_range,
                                    const int a_cnt,
                                    Array& a_result,
                                    int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // local data structures:
    //  carrier - node to be explored next and to be stored in priority queue
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_q;        // about which query (q)
        const int   m_node;     // node to be explored
        const float m_cost;     // distance from the source of q
        Array       m_path;     // shortest path from q
    public:
        carrier(const int a_q, const int a_node, const float a_cost):
            m_q(a_q), m_node(a_node), m_cost(a_cost)
            {
                m_path.append((void*)a_node);
            };
        carrier(const int a_q, const int a_node,
            const float a_cost, Array& a_path):
            m_q(a_q), m_node(a_node), m_cost(a_cost), m_path(a_path)
            {
                m_path.append((void*)a_node);
            };
        virtual ~carrier(){};
        static int compare(const void* a0, const void* a1)
        {
            carrier* n0 = *(carrier**)a0;
            carrier* n1 = *(carrier**)a1;
            if (n0->m_cost < n1->m_cost) return -1;
            if (n0->m_cost > n1->m_cost) return +1;
            if (n0->m_q < n1->m_q) return -1;
            if (n0->m_q > n1->m_q) return +1;
            if (n0->m_node < n1->m_node) return -1;
            if (n0->m_node > n1->m_node) return -1;
            return 0;
        }
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    Hash objects;

    Array src[MAXQUERY], dest[MAXQUERY];
    Set visited[MAXQUERY];
    BinHeap h(carrier::compare);
    for (int i=0; i<a_cnt; i++)
        h.insert(new carrier(i, a_src[i], 0));

    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();
        
        // --------------------------------------------------------------------
        // if the node from a corresponding query is farther than a threshold,
        // skip it!
        // --------------------------------------------------------------------
        if (c->m_cost > a_range[c->m_q])
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // if the node is visited by a corresponding query, skip it!
        // --------------------------------------------------------------------
        if (visited[c->m_q].in((void*)c->m_node))
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // exploring the node
        // --------------------------------------------------------------------
        a_nodeaccess++;
        visited[c->m_q].insert((void*)c->m_node);

        Node* node = a_graph.getNode(c->m_node);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new carrier(c->m_q, edge->m_neighbor, c->m_cost+edge->m_cost, c->m_path));
            src[c->m_q].append((void*)c->m_node);
            dest[c->m_q].append((void*)edge->m_neighbor);
            a_edgeaccess++;
        }
        delete node;

        // --------------------------------------------------------------------
        // keep the asscoiated objects if any
        // --------------------------------------------------------------------
        const Array* objs = a_map.findObject(c->m_node);
        for (int j=0; objs!=0 && j<objs->size(); j++)
        {
            int oid = (long)objs->get(j);
            GroupObjectSearchResult* obj =
                (GroupObjectSearchResult*)objects.get(oid);
            if (obj == 0)
                objects.put(oid, obj =
                new GroupObjectSearchResult(c->m_node, oid, a_cnt));
            obj->m_cost[c->m_q] = c->m_cost;
            obj->m_path[c->m_q].copy(c->m_path);
        }

        delete c;
    }

    //GraphPlot::plot("c:\\objectsearch.ps", a_graph,
    //    src[0], dest[0], src[1], dest[1], a_src[0], a_src[1]);

    for (HashReader rdr(objects); !rdr.isEnd(); rdr.next())
    {
        GroupObjectSearchResult* obj = (GroupObjectSearchResult*)rdr.getVal();
        bool reachedByAll = true;
        for (int i=0; i<a_cnt && reachedByAll; i++)
            reachedByAll = (obj->m_cost[i] != -1);
        if (reachedByAll)
            a_result.append(obj);
        else
            delete obj;
    }
    objects.clean();
}

//-----------------------------------------------------------------------------
// multiple point kNN search
//-----------------------------------------------------------------------------
void ObjectSearch::groupKNNSearch(Graph& a_graph, NodeMapping& a_map,
                                  const int* a_src, const int a_cnt,
                                  const int a_k,
                                  Array& a_result,
                                  int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // local data structures:
    //  carrier - node to be explored next and to be stored in priority queue
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_q;        // about which query (q)
        const int   m_node;     // node to be explored
        const float m_cost;     // distance from the source of q
        Array       m_path;     // shortest path from q
    public:
        carrier(const int a_q, const int a_node, const float a_cost):
            m_q(a_q), m_node(a_node), m_cost(a_cost)
            {
                m_path.append((void*)a_node);
            };
        carrier(const int a_q, const int a_node,
            const float a_cost, Array& a_path):
            m_q(a_q), m_node(a_node), m_cost(a_cost), m_path(a_path)
            {
                m_path.append((void*)a_node);
            };
        virtual ~carrier(){};
        static int compare(const void* a0, const void* a1)
        {
            carrier* n0 = *(carrier**)a0;
            carrier* n1 = *(carrier**)a1;
            if (n0->m_cost < n1->m_cost) return -1;
            if (n0->m_cost > n1->m_cost) return +1;
            if (n0->m_q < n1->m_q) return -1;
            if (n0->m_q > n1->m_q) return +1;
            if (n0->m_node < n1->m_node) return -1;
            if (n0->m_node > n1->m_node) return -1;
            return 0;
        }
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    Hash objects;

    Set visited[MAXQUERY];
    BinHeap h(carrier::compare);
    for (int i=0; i<a_cnt; i++)
        h.insert(new carrier(i, a_src[i], 0));

    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();
        
        // --------------------------------------------------------------------
        // if k answer objects are found, terminate
        // --------------------------------------------------------------------
        if (a_result.size() >= a_k)
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // if the node is visited by a corresponding query, skip it!
        // --------------------------------------------------------------------
        if (visited[c->m_q].in((void*)c->m_node))
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // exploring the node
        // --------------------------------------------------------------------
        a_nodeaccess++;
        visited[c->m_q].insert((void*)c->m_node);

        Node* node = a_graph.getNode(c->m_node);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new carrier(c->m_q, edge->m_neighbor, c->m_cost+edge->m_cost, c->m_path));
            a_edgeaccess++;
        }
        delete node;

        // --------------------------------------------------------------------
        // keep the asscoiated objects if any
        // --------------------------------------------------------------------
        const Array* objs = a_map.findObject(c->m_node);
        for (int j=0; objs!=0 && j<objs->size(); j++)
        {
            int oid = (long)objs->get(j);
            GroupObjectSearchResult* obj =
                (GroupObjectSearchResult*)objects.get(oid);
            if (obj == 0)
                objects.put(oid, obj =
                new GroupObjectSearchResult(c->m_node, oid, a_cnt));
            obj->m_cost[c->m_q] = c->m_cost;
            obj->m_path[c->m_q].copy(c->m_path);
            if (obj->allreached(a_cnt))
            {
                a_result.append(obj);
                objects.remove(oid);
            }
        }

        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    for (HashReader rdr(objects); !rdr.isEnd(); rdr.next())
    {
        GroupObjectSearchResult* obj = (GroupObjectSearchResult*)rdr.getVal();
        delete obj;
    }
    objects.clean();
}

void ObjectSearch::groupKNNSearch(Graph& a_graph, NodeMapping& a_map,
                                  const int* a_src, const int a_cnt,
                                  const int a_k,
                                  Array& a_result,
                                  int& a_nodeaccess, int& a_edgeaccess,
                                  Array& a_visited)
{
    // ------------------------------------------------------------------------
    // local data structures:
    //  carrier - node to be explored next and to be stored in priority queue
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_q;        // about which query (q)
        const int   m_node;     // node to be explored
        const float m_cost;     // distance from the source of q
        Array       m_path;     // shortest path from q
    public:
        carrier(const int a_q, const int a_node, const float a_cost):
            m_q(a_q), m_node(a_node), m_cost(a_cost)
            {
                m_path.append((void*)a_node);
            };
        carrier(const int a_q, const int a_node,
            const float a_cost, Array& a_path):
            m_q(a_q), m_node(a_node), m_cost(a_cost), m_path(a_path)
            {
                m_path.append((void*)a_node);
            };
        virtual ~carrier(){};
        static int compare(const void* a0, const void* a1)
        {
            carrier* n0 = *(carrier**)a0;
            carrier* n1 = *(carrier**)a1;
            if (n0->m_cost < n1->m_cost) return -1;
            if (n0->m_cost > n1->m_cost) return +1;
            if (n0->m_q < n1->m_q) return -1;
            if (n0->m_q > n1->m_q) return +1;
            if (n0->m_node < n1->m_node) return -1;
            if (n0->m_node > n1->m_node) return -1;
            return 0;
        }
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    Hash objects;

    Set visited[MAXQUERY];
    BinHeap h(carrier::compare);
    for (int i=0; i<a_cnt; i++)
        h.insert(new carrier(i, a_src[i], 0));

    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();
        
        // --------------------------------------------------------------------
        // if k answer objects are found, terminate
        // --------------------------------------------------------------------
        if (a_result.size() >= a_k)
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // if the node is visited by a corresponding query, skip it!
        // --------------------------------------------------------------------
        if (visited[c->m_q].in((void*)c->m_node))
        {
            delete c;
            continue;
        }

        // --------------------------------------------------------------------
        // exploring the node
        // --------------------------------------------------------------------
        a_nodeaccess++;
        visited[c->m_q].insert((void*)c->m_node);
        a_visited.append((void*)c->m_node);

        Node* node = a_graph.getNode(c->m_node);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new carrier(c->m_q, edge->m_neighbor, c->m_cost+edge->m_cost, c->m_path));
            a_edgeaccess++;
        }
        delete node;

        // --------------------------------------------------------------------
        // keep the asscoiated objects if any
        // --------------------------------------------------------------------
        const Array* objs = a_map.findObject(c->m_node);
        for (int j=0; objs!=0 && j<objs->size(); j++)
        {
            int oid = (long)objs->get(j);
            GroupObjectSearchResult* obj =
                (GroupObjectSearchResult*)objects.get(oid);
            if (obj == 0)
                objects.put(oid, obj =
                new GroupObjectSearchResult(c->m_node, oid, a_cnt));
            obj->m_cost[c->m_q] = c->m_cost;
            obj->m_path[c->m_q].copy(c->m_path);
            if (obj->allreached(a_cnt))
            {
                a_result.append(obj);
                objects.remove(oid);
            }
        }

        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    for (HashReader rdr(objects); !rdr.isEnd(); rdr.next())
    {
        GroupObjectSearchResult* obj = (GroupObjectSearchResult*)rdr.getVal();
        delete obj;
    }
    objects.clean();
}

