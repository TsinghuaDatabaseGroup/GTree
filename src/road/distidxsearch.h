/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains object search algorithms based on distance index.
---------------------------------------------------------------------------- */
#ifndef distidxsearch_defined
#define distidxsearch_defined

#include "objectsearch.h"
class Graph;
class DistIndex;

class DistIndexSearch
{
public:
    // ------------------------------------------------------------------------
    // single-point range search
    // ------------------------------------------------------------------------
    static void rangeSearch(
        Graph& a_graph, DistIndex& a_distidx,
        const int a_src, const float a_range,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // single-point kNN search
    // ------------------------------------------------------------------------
    static void kNNSearch(
        Graph& a_graph, DistIndex& a_distidx,
        const int a_src, const int a_k,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);
    static void kNNSearch(
        Graph& a_graph, DistIndex& a_distidx,
        const int a_src, const int a_k,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess,
        Array& a_visited);

    // ------------------------------------------------------------------------
    // multi-point range search
    // ------------------------------------------------------------------------
    static void groupRangeSearch(
        Graph& a_graph, DistIndex& a_distidx,
        const int* a_src, const float* a_range,
        const int a_cnt,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);

    // ------------------------------------------------------------------------
    // multi-point kNN search
    // ------------------------------------------------------------------------
    static void groupKNNSearch(
        Graph& a_graph, DistIndex& a_distidx,
        const int* a_src, const int a_cnt,
        const int a_k,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);
    static void groupKNNSearch(
        Graph& a_graph, DistIndex& a_distidx,
        const int* a_src, const int a_cnt,
        const int a_k,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess,
        Array& a_visited);
};

#endif

