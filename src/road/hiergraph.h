/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class HierGraph declaration.
    This provide accesses to graph border nodes.
---------------------------------------------------------------------------- */
#ifndef hiergraph_defined
#define hiergraph_defined

class BorderNode;
#include "collection.h"
#include "graph.h"

class HierGraph: public Graph
{
public:
    // constructor/destructor
    HierGraph(SegMemory& a_nodeMem);
    virtual ~HierGraph();
    //
    // update
    virtual int writeNode(const int a_nid, const BorderNode& a_bnode);
    //
    // search
    virtual Node* getNode(const int a_nid);     // overload getNode in Graph
    virtual BorderNode* getBorderNode(const int a_nid);
};

#endif
