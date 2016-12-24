/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class Graph declaration.
    This provide accesses to graph nodes.
---------------------------------------------------------------------------- */
#ifndef graph_defined
#define graph_defined

class Node;
class SegMemory;
#include "collection.h"

class Graph
{
protected:
    SegMemory&  m_nodeMem;  // handle of memory for edges
public:
    Hash        m_nodes;    // hash of node to its content in memory
public:
    // constructor/destructor
    Graph(SegMemory& a_nodeMem);
    virtual ~Graph();
    //
    // update
    virtual int writeNode(const int a_nid, const Node& a_node);
    //
    // search
    virtual Node* getNode(const int a_nid);
};

#endif
