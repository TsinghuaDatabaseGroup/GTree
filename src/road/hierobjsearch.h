/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains object search algorithms for hiergraph.
---------------------------------------------------------------------------- */
#ifndef hierobjectsearch_defined
#define hierobjectsearch_defined

#include "objectsearch.h"
class HierGraph;
class GraphMapping;

class HierObjectSearch
{
public:
    // ------------------------------------------------------------------------
    // single point range search
    // ------------------------------------------------------------------------
    static void rangeSearch(
        HierGraph& a_graph, NodeMapping& a_nmap, GraphMapping& a_gmap,
        const int a_src, const float a_range,
        Array& a_result, int& a_nodeaccess, int& a_edgeaccess);


    // ------------------------------------------------------------------------
    // single point kNN search
    // ------------------------------------------------------------------------
    static void kNNSearch(
        HierGraph& a_graph, NodeMapping& a_map, GraphMapping& a_gmap,
        const int a_src, const int k,
        Array& a_result,int& nodeaccess, int& edgeaccess);
    static void kNNSearch(
        HierGraph& a_graph, NodeMapping& a_map, GraphMapping& a_gmap,
        const int a_src, const int k,
        Array& a_result,int& nodeaccess, int& edgeaccess,
        Array& visited);


    // ------------------------------------------------------------------------
    // multi-point range search
    // ------------------------------------------------------------------------
    static void groupRangeSearch(
        HierGraph& a_graph, NodeMapping& a_map, GraphMapping& a_gmap,
        const int* a_src, const float* a_range,
        const int a_cnt,
        Array& a_result,int& nodeaccess, int& edgeaccess);

    // ------------------------------------------------------------------------
    // multi-point kNN search
    // ------------------------------------------------------------------------
    static void groupKNNSearch(
        HierGraph& a_graph, NodeMapping& a_map, GraphMapping& a_gmap,
        const int* a_src, const int a_cnt,
        const int a_k,
        Array& a_result,int& nodeaccess, int& edgeaccess);
    static void groupKNNSearch(
        HierGraph& a_graph, NodeMapping& a_map, GraphMapping& a_gmap,
        const int* a_src, const int a_cnt,
        const int a_k,
        Array& a_result,int& nodeaccess, int& edgeaccess,
        Array& a_visited);
};

#endif

