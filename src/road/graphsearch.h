/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains search algorithms for graph.
---------------------------------------------------------------------------- */
#ifndef graphsearch_defined
#define graphsearch_defined

#include "collection.h"
class Graph;

class GraphSearchResult
{
public:
    const int   m_nid;
    const float m_cost;
    const float m_acost;    // used in A* algorithm
    Array       m_path;      
public:
    GraphSearchResult(
        const int a_nid, const float a_cost):
        m_nid(a_nid), m_path(), m_cost(a_cost), m_acost(0)
        { m_path.append((void*)m_nid); };
    GraphSearchResult(
        const int a_nid,
        const Array& a_path, const float a_cost):
        m_nid(a_nid), m_path(a_path), m_cost(a_cost), m_acost(0)
        { m_path.append((void*)m_nid); };
    GraphSearchResult(
        const int a_nid,
        const float a_cost, const float a_acost):
        m_nid(a_nid), m_path(), m_cost(a_cost), m_acost(a_acost)
        { m_path.append((void*)m_nid); };
    GraphSearchResult(
        const int a_nid, const Array& a_path,
        const float a_cost, const float a_acost):
        m_nid(a_nid), m_path(a_path), m_cost(a_cost), m_acost(a_acost)
        { m_path.append((void*)m_nid); };
    ~GraphSearchResult() {};
    static int compare(const void* a0, const void* a1)
    {
        GraphSearchResult* r0 = *(GraphSearchResult**)a0;
        GraphSearchResult* r1 = *(GraphSearchResult**)a1;
        if (r0->m_cost < r1->m_cost) return -1;
        if (r0->m_cost > r1->m_cost) return +1;
        return 0;
    };
    static int acompare(const void* a0, const void* a1)
    {
        GraphSearchResult* r0 = *(GraphSearchResult**)a0;
        GraphSearchResult* r1 = *(GraphSearchResult**)a1;
        if (r0->m_acost < r1->m_acost) return -1;
        if (r0->m_acost > r1->m_acost) return +1;
        return 0;
    };
};

class GraphSearch
{
public:
    // ------------------------------------------------------------------------
    // find the diameter the longest distance in a graph
    // ------------------------------------------------------------------------
    static float diameter(
        Graph& a_graph,const int a_src,
        int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // find spanning tree towards a node (for distance index creation)
    // ------------------------------------------------------------------------
    static void spanSearch(
        Graph& a_graph, const int a_dest,
        Array& a_nodes2dest);

    // ------------------------------------------------------------------------
    // find spanning tree from a node (for shortest path quadtree creation)
    // ------------------------------------------------------------------------
    static void diffuseSearch(
        Graph& a_graph, const int a_src,
        Array& a_nodes2src);


    // ------------------------------------------------------------------------
    // shortest path search for a single destination
    // ------------------------------------------------------------------------
    static GraphSearchResult* shortestPathSearch(
        Graph& a_graph, const int a_src, const int a_dest,
        int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // A* shortest path search for a single destination
    // ------------------------------------------------------------------------
    static GraphSearchResult* aStar(
        Graph& a_graph, const int a_src, const int a_dest,
        int& a_nodeaccess, int& a_edgeaccess);
    static GraphSearchResult* aStar(
        Graph& a_graph, const int a_src, const int a_dest,
        int& a_nodeaccess, int& a_edgeaccess,
        Array& a_visited);

    // ------------------------------------------------------------------------
    // shortest path search for multple destinations
    // ------------------------------------------------------------------------
    static void shortestPathSearch(
        Graph& a_graph, const int a_src, Set& a_dest,
        Hash& a_result,
        int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // shortest path search for multple destinations
    // ------------------------------------------------------------------------
    static void shortestPathSearch(
        Hash& nodes, const int a_src, Set& a_dest,
        Hash& a_result,
        int& a_nodeaccess, int& a_edgeaccess);
};

#endif

