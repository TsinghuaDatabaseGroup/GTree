/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains object search algorithms based on distance browsing
    (shortest path quadtree).
---------------------------------------------------------------------------- */
#ifndef distbrwssearch_defined
#define distbrwssearch_defined

#include "objectsearch.h"
class Graph;
class DistBrws;
class Bound;

class DistBrwsSearch
{
public:
    // ------------------------------------------------------------------------
    // single-point range search
    // ------------------------------------------------------------------------
    static void rangeSearch(
        Graph& a_graph,
        DistBrws& a_distbrws, const Bound& a_bound,
        const Array& a_nodes,
		const int a_src, const float a_range,
        Array &a_result,
        int &a_nodeaccess, int &a_edgeaccess);

    // ------------------------------------------------------------------------
    // single-point kNN search
    // ------------------------------------------------------------------------
    static void nnSearch(
        Graph& a_graph,
        DistBrws& a_distbrws, const Bound& a_bound,
        const Array& a_nodes,
        const int a_src, const int a_k,
        Array &a_result,
        int &a_nodeaccess, int &a_edgeaccess);
};

#endif
