/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "graphsearch.h"
#include "graph.h"
#include "node.h"
#include "edge.h"
#include <math.h>

float GraphSearch::diameter(Graph& a_graph, const int a_src,
                            int& a_nodeaccess,
                            int& a_edgeaccess)
{
    class carrier
    {
    public:
        const int   m_nodeid;
        const float m_cost;
    public:
        carrier(const int a_nodeid, const float a_cost):
            m_nodeid(a_nodeid), m_cost(a_cost) {};
        ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            return 0;
        };
    };

    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    float cost=0;
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // Dijkstra's shorest path search (best first)
    // ------------------------------------------------------------------------
    Set visited(1000);
    BinHeap h(carrier::compare);
    h.insert(new carrier(a_src,0));
    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();
        // --------------------------------------------------------------------
        // check if the node is visited. If so, skip it
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nodeid))
        {
            delete c;
            continue;
        }

        cost = c->m_cost;
        visited.insert((void*)c->m_nodeid);

        // --------------------------------------------------------------------
        // expand the current scope
        // --------------------------------------------------------------------
        Node* node = a_graph.getNode(c->m_nodeid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(new carrier(edge->m_neighbor, c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        a_nodeaccess++;

        delete node;
        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (carrier*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return cost;
}

// ----------------------------------------------------------------------------
// find spanning tree towards a node (for distance index creation)
// ----------------------------------------------------------------------------
void GraphSearch::spanSearch(Graph& a_graph, const int a_src,
                             Array& a_nodes2src)
{
    class carrier
    {
    public:
        const int   m_prev;
        const int   m_nid;
        const float m_cost;
    public:
        carrier(const int a_prev, const int a_nid, const float a_cost):
            m_prev(a_prev), m_nid(a_nid), m_cost(a_cost) {};
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_nid < c1->m_nid) return -1;
            if (c0->m_nid > c1->m_nid) return +1;
            return 0;
        };
    };

    Set visited(1000);
    BinHeap h(carrier::compare);
    h.insert(new carrier(a_src,a_src,0));
    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();

        // --------------------------------------------------------------------
        // filter out visited node
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }
        visited.insert((void*)c->m_nid);

        // --------------------------------------------------------------------
        // result collection
        // --------------------------------------------------------------------
        GraphSearchResult* res = new GraphSearchResult(c->m_nid, c->m_cost);
        res->m_path.clean();
        res->m_path.append((void*)c->m_prev);
        a_nodes2src.append(res);

        // --------------------------------------------------------------------
        // further expansion
        // --------------------------------------------------------------------
        Node* n = (Node*)a_graph.getNode(c->m_nid);
        for (int i=0; i<n->m_edges.size(); i++)
        {
            Edge* e = (Edge*)n->m_edges.get(i);
            h.insert(new carrier(c->m_nid,e->m_neighbor,e->m_cost + c->m_cost));
        }
        delete n;

        // --------------------------------------------------------------------
        // clean up
        // --------------------------------------------------------------------
        delete c;
    }
}


// ----------------------------------------------------------------------------
// find spanning tree from a node (for shortest path quadtree creation)
// ----------------------------------------------------------------------------
void GraphSearch::diffuseSearch(Graph& a_graph, const int a_src,
                                Array& a_nodes2src)
{
    class carrier
    {
    public:
        const int   m_first;
        const int   m_nid;
        const float m_cost;
    public:
        carrier(const int a_first, const int a_nid, const float a_cost):
            m_first(a_first), m_nid(a_nid), m_cost(a_cost) {};
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_nid < c1->m_nid) return -1;
            if (c0->m_nid > c1->m_nid) return +1;
            return 0;
        };
    };

    Set visited(10000);
    BinHeap h(carrier::compare);
    h.insert(new carrier(a_src,a_src,0));
    while (!h.isEmpty())
    {
        carrier* c = (carrier*)h.removeTop();

        // --------------------------------------------------------------------
        // filter out visited node
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }
        visited.insert((void*)c->m_nid);

        // --------------------------------------------------------------------
        // result collection
        // --------------------------------------------------------------------
        GraphSearchResult* res = new GraphSearchResult(c->m_nid, c->m_cost);
        res->m_path.clean();
        res->m_path.append((void*)c->m_first);
        a_nodes2src.append(res);

        // --------------------------------------------------------------------
        // further expansion
        // --------------------------------------------------------------------
        Node* n = (Node*)a_graph.getNode(c->m_nid);
        for (int i=0; i<n->m_edges.size(); i++)
        {
            Edge* e = (Edge*)n->m_edges.get(i);
            int firstnode = c->m_first == a_src ? e->m_neighbor : c->m_first;
            h.insert(new carrier(firstnode,e->m_neighbor,e->m_cost + c->m_cost));
        }
        delete n;

        // --------------------------------------------------------------------
        // clean up
        // --------------------------------------------------------------------
        delete c;
    }
}

// ----------------------------------------------------------------------------
// shortest path search for a single destination
// based on Dijsktra's algorithm
// ----------------------------------------------------------------------------
GraphSearchResult* GraphSearch::shortestPathSearch(Graph& graph,
                                                   const int a_src,
                                                   const int a_dest,
                                                   int& a_nodeaccess,
                                                   int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    GraphSearchResult* res = 0;
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // Dijkstra's shorest path search (best first)
    // ------------------------------------------------------------------------
    Set visited(1000);
    BinHeap h(GraphSearchResult::compare);
    h.insert(new GraphSearchResult(a_src,0));
    while (!h.isEmpty())
    {
        GraphSearchResult* c = (GraphSearchResult*)h.removeTop();
        // --------------------------------------------------------------------
        // check if the node is visited. If so, skip it
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        visited.insert((void*)c->m_nid);

        // --------------------------------------------------------------------
        // check if destination is found
        // --------------------------------------------------------------------
        if (c->m_nid == a_dest)
        {
            res = c;
            break;
        }

        // --------------------------------------------------------------------
        // expand the current scope
        // --------------------------------------------------------------------
        Node* node = graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new GraphSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        a_nodeaccess++;
        delete node;

        // --------------------------------------------------------------------
        // clean up
        // --------------------------------------------------------------------
        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (GraphSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return res;
}

// ----------------------------------------------------------------------------
// shortest path search for a single destination
// based on A* algorithm
// ----------------------------------------------------------------------------
GraphSearchResult* GraphSearch::aStar(Graph& a_graph,
                                      const int a_src, const int a_dest,
                                      int& a_nodeaccess,
                                      int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    GraphSearchResult* res = 0;
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    Node* dest = a_graph.getNode(a_dest);
    const float destx = dest->m_x;
    const float desty = dest->m_y;
    delete dest;

    // ------------------------------------------------------------------------
    // Disjkstra's shorest path search (best first)
    // ------------------------------------------------------------------------
    Set visited(1000);
    BinHeap h(GraphSearchResult::acompare);
    h.insert(new GraphSearchResult(a_src,0,0));
    while (!h.isEmpty())
    {
        GraphSearchResult* c = (GraphSearchResult*)h.removeTop();
        // --------------------------------------------------------------------
        // check if the node is visited. If so, skip it
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        visited.insert((void*)c->m_nid);
        // --------------------------------------------------------------------
        // check if destination is found
        // --------------------------------------------------------------------
        if (a_dest == c->m_nid)
        {
            res = c;
            break;
        }

        // --------------------------------------------------------------------
        // expand the current scope
        // --------------------------------------------------------------------
        Node* node = a_graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            Node* next = a_graph.getNode(edge->m_neighbor);
            float heu =
                sqrt((next->m_x - destx)*(next->m_x - destx) +
                (next->m_y - desty)*(next->m_y - desty));
            h.insert(
                new GraphSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost, c->m_cost + edge->m_cost + heu));
            delete next;
            a_edgeaccess++;
        }
        a_nodeaccess++;

        delete node;
        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (GraphSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return res;
}

// ----------------------------------------------------------------------------
// shortest path search for a single destination
// based on A* algorithm with trace
// ----------------------------------------------------------------------------
GraphSearchResult* GraphSearch::aStar(Graph& a_graph,
                                      const int a_src, const int a_dest,
                                      int& a_nodeaccess,
                                      int& a_edgeaccess,
                                      Array& a_visited)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    GraphSearchResult* res = 0;
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    Node* dest = a_graph.getNode(a_dest);
    const float destx = dest->m_x;
    const float desty = dest->m_y;
    delete dest;

    // ------------------------------------------------------------------------
    // Disjkstra's shorest path search (best first)
    // ------------------------------------------------------------------------
    Set visited(1000);
    BinHeap h(GraphSearchResult::acompare);
    h.insert(new GraphSearchResult(a_src,0,0));
    while (!h.isEmpty())
    {
        GraphSearchResult* c = (GraphSearchResult*)h.removeTop();
        // --------------------------------------------------------------------
        // check if the node is visited. If so, skip it
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        visited.insert((void*)c->m_nid);
        a_visited.append((void*)c->m_nid);

        // --------------------------------------------------------------------
        // check if destination is found
        // --------------------------------------------------------------------
        if (a_dest == c->m_nid)
        {
            res = c;
            break;
        }

        // --------------------------------------------------------------------
        // expand the current scope
        // --------------------------------------------------------------------
        Node* node = a_graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            Node* next = a_graph.getNode(edge->m_neighbor);
            float heu =
                sqrt((next->m_x - destx)*(next->m_x - destx) +
                (next->m_y - desty)*(next->m_y - desty));
            h.insert(
                new GraphSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost, c->m_cost + edge->m_cost + heu));
            delete next;
            a_edgeaccess++;
        }
        a_nodeaccess++;

        delete node;
        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (GraphSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return res;
}

// ----------------------------------------------------------------------------
// shortest path search for multiple destinations
// based on Dijsktra's algorithm
// ----------------------------------------------------------------------------
void GraphSearch::shortestPathSearch(Graph& graph,
                                     const int a_src, Set& a_dest,
                                     Hash& a_result,
                                     int& a_nodeaccess,
                                     int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // Disjkstra's shorest path search (best first)
    // ------------------------------------------------------------------------
    Set visited(1000);
    BinHeap h(GraphSearchResult::compare);
    h.insert(new GraphSearchResult(a_src,0));
    while (!h.isEmpty())
    {
        GraphSearchResult* c = (GraphSearchResult*)h.removeTop();
        // --------------------------------------------------------------------
        // check if the node is visited. If so, skip it
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        visited.insert((void*)c->m_nid);
        // --------------------------------------------------------------------
        // check if destination is found
        // --------------------------------------------------------------------
        if (a_dest.in((void*)c->m_nid))
        {
            if (a_result.get(c->m_nid) == 0)
                a_result.put(
                    c->m_nid,
                    new GraphSearchResult(c->m_nid, c->m_path, c->m_cost));
            if (a_result.size() == a_dest.size())
            {
                delete c;
                break;
            }
        }

        // --------------------------------------------------------------------
        // expand the current scope
        // --------------------------------------------------------------------
        Node* node = graph.getNode(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new GraphSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        a_nodeaccess++;

        delete node;
        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (GraphSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return;
}

// ----------------------------------------------------------------------------
// shortest path search for multple destinations
// ----------------------------------------------------------------------------
void GraphSearch::shortestPathSearch(Hash& a_nodes,
                                     const int a_src, Set& a_dest,
                                     Hash& a_result,
                                     int& a_nodeaccess,
                                     int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // Disjkstra's shorest path search (best first)
    // ------------------------------------------------------------------------
    Set visited(10000);
    BinHeap h(GraphSearchResult::compare);
    h.insert(new GraphSearchResult(a_src,0));
    while (!h.isEmpty())
    {
        GraphSearchResult* c = (GraphSearchResult*)h.removeTop();
        // --------------------------------------------------------------------
        // check if the node is visited. If so, skip it
        // --------------------------------------------------------------------
        if (visited.in((void*)c->m_nid))
        {
            delete c;
            continue;
        }

        visited.insert((void*)c->m_nid);
        // --------------------------------------------------------------------
        // check if destination is found
        // --------------------------------------------------------------------
        if (a_dest.in((void*)c->m_nid))
        {
            if (a_result.get(c->m_nid) == 0)
                a_result.put(
                    c->m_nid,
                    new GraphSearchResult(c->m_nid, c->m_path, c->m_cost));
            if (a_result.size() == a_dest.size())
            {
                delete c;
                break;
            }
        }

        // --------------------------------------------------------------------
        // expand the current scope
        // --------------------------------------------------------------------
        Node* node = (Node*)a_nodes.get(c->m_nid);
        for (int e=0; e<node->m_edges.size(); e++)
        {
            Edge* edge = (Edge*)node->m_edges.get(e);
            h.insert(
                new GraphSearchResult(edge->m_neighbor, c->m_path,
                c->m_cost + edge->m_cost));
            a_edgeaccess++;
        }
        a_nodeaccess++;
        delete c;
    }

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    while (!h.isEmpty())
        delete (GraphSearchResult*)h.removeTop();

    // ------------------------------------------------------------------------
    // all done
    // ------------------------------------------------------------------------
    return;
}

