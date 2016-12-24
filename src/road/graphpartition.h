/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This file contains graph partitioning algorithms.
---------------------------------------------------------------------------- */
#ifndef graphpartition_defined
#define graphpartition_defined

#include "collection.h"
class HierGraph;

class GraphPartition
{
public:
    static void geoPartition(
        Hash& a_nodes, const int a_k, const int a_level,
        HierGraph& a_hiergraph,
        float& a_parttime,     // partitioning time
        float& a_shorttime);   // shortcut calculation time.
};

#endif

