/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class DistBrows declaration.
    That captures individual nodes and indexes their paths to other nodes
    based on shortest path quadtrees
---------------------------------------------------------------------------- */
#ifndef distbrws_defined
#define distbrws_defined

#include "collection.h"

class SegMemory;
class SPQuadtree;
class Bound;

class DistBrws
{
protected:
    SegMemory&  m_nodeMem;  // handle of memory of distance signature
public:
    Hash        m_nodes;    // hash of distance signatures in memory
public:
    // constructor/destructor
    DistBrws(SegMemory& a_nodeMem);
    virtual ~DistBrws();
    //
    // update
    int writeNode(const int a_nid, SPQuadtree* a_spqdtree);
    //
    // search
    virtual SPQuadtree* getNode(const int a_nid, const Bound& a_bound);
};

#endif
