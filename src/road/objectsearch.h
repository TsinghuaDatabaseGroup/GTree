/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains object search algorithms for graph.
---------------------------------------------------------------------------- */
#ifndef objectsearch_defined
#define objectsearch_defined

#include "collection.h"
class Graph;
class NodeMapping;

#define MAXQUERY    10

class ObjectSearchResult
{
public:
    const int   m_nid;
    const float m_cost;
    Array       m_path;
    Array       m_objects;
public:
    ObjectSearchResult(const int a_nid, const float a_cost):
        m_nid(a_nid), m_cost(a_cost)
        { m_path.append((void*)m_nid); };
    ObjectSearchResult(const int a_nid, const Array& a_path, const float a_cost):
        m_nid(a_nid), m_path(a_path), m_cost(a_cost)
        { m_path.append((void*)m_nid); };
    ~ObjectSearchResult()
        { m_path.clean(); };
    void addObjects(const Array& a_objs)
    {
        for (int i=0; i<a_objs.size(); i++)
            m_objects.append(m_objects.get(i));
    }
    static int compare(const void* a0, const void* a1)
    {
        ObjectSearchResult* r0 = *(ObjectSearchResult**)a0;
        ObjectSearchResult* r1 = *(ObjectSearchResult**)a1;
        if (r0->m_cost < r1->m_cost) return -1;
        if (r0->m_cost > r1->m_cost) return +1;
        return 0;
    }
};

class GroupObjectSearchResult
{
public:
    const int   m_oid;
    int         m_nid;
    float       m_cost[MAXQUERY];
    Array       m_path[MAXQUERY];
public:
    GroupObjectSearchResult(const int a_nid, const int a_oid, const int a_cnt):
        m_nid(a_nid), m_oid(a_oid)
        {
            for (int i=0; i<a_cnt+1; i++)   // initialize the cost to -1
                m_cost[i] = -1;             // to signal the object is not
                                            // found currently
        };
    ~GroupObjectSearchResult(){};
    float sumcost() const
    {
        float ret = 0;
        for (int i=0; i<MAXQUERY; i++)
        {
            if (m_cost[i] == -1) break;
            ret = ret > m_cost[i] ? ret : m_cost[i];
        }
        return ret;
    };
    bool allreached(const int a_cnt) const
    {
        for (int i=0; i<a_cnt; i++)
            if (m_cost[i] == -1) return false;
        return true;
    };
    static int compare(const void* a0, const void* a1)
    {
        GroupObjectSearchResult* r0 = *(GroupObjectSearchResult**)a0;
        GroupObjectSearchResult* r1 = *(GroupObjectSearchResult**)a1;
        float dist0 = r0->sumcost();
        float dist1 = r1->sumcost();
        if (dist0 < dist1) return -1;
        if (dist0 > dist1) return +1;
        if (r0->m_oid < r1->m_oid) return -1;
        if (r0->m_oid > r1->m_oid) return +1;
        return 0;
    };
};

class ObjectSearch
{
public:
    // ------------------------------------------------------------------------
    // single-point range search
    // ------------------------------------------------------------------------
    static void rangeSearch(
        Graph& a_graph, NodeMapping& a_map,
        const int a_src, const float a_range,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // single-point kNN search
    // ------------------------------------------------------------------------
    static void kNNSearch(
        Graph& a_graph, NodeMapping& a_map,
        const int a_src, const int k,
        Array& a_result,int& nodeaccess, int& edgeaccess);
    static void kNNSearch(
        Graph& a_graph, NodeMapping& a_map,
        const int a_src, const int k,
        Array& a_result,int& nodeaccess, int& edgeaccess,
        Array& a_access);

    // ------------------------------------------------------------------------
    // multi-point range search
    // ------------------------------------------------------------------------
    static void groupRangeSearch(
        Graph& a_graph, NodeMapping& a_map,
        const int* a_src, const float* a_range,
        const int a_cnt,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // multi-point kNN search
    // ------------------------------------------------------------------------
    static void groupKNNSearch(
        Graph& a_graph, NodeMapping& a_map,
        const int* a_src, const int a_cnt,
        const int a_k,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);
    static void groupKNNSearch(
        Graph& a_graph, NodeMapping& a_map,
        const int* a_src, const int a_cnt,
        const int a_k,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess,
        Array& a_visited);
};

#endif


