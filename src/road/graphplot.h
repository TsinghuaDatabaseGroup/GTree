/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains search algorithms for graph.
---------------------------------------------------------------------------- */
#ifndef graphplot_defined
#define graphplot_defined

#include "collection.h"
class Graph;

class GraphPlot
{
public:
    static void plot(               // plotting a plain graph
        const char* a_filename,
        Graph& a_graph);
    static void plot(               // plotting multiple graphs
        const char* a_filename,
        Graph** a_graph,
        const int a_cnt);
    static void plot(               // plotting a graph with object locations
        const char* a_filename, 
        Hash& a_nodes, 
        Array& a_node2objects);
    static void plot(               // plotting a graph with object and query locations
        const char* a_filename, 
        Hash& a_nodes, 
        Array& a_node2objects,
        Array& a_node2queries);

    static void plot(
        const char* a_filename, Graph& a_graph,
        Array& src1, Array& dest1,
        Array& src2, Array& dest2,
        const int src, const int dest);

    // this plot the search paths made from source to destination objects
    static void plot(   
        const char* a_filename, Graph& a_graph,
        const int a_querynode, Array& a_objnode,
        Array& a_visitednode);

    // this plot the search paths made from source to destination objects
    static void plot(   
        const char* a_filename, Graph& a_graph,
        const int* a_querynode, const int a_sz, Array& a_objnode,
        Array& a_visitednode);
};

#endif

