/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class DistIndex declaration.
    That maps objects to individual nodes and indexes their distance using
    a distance signature
---------------------------------------------------------------------------- */
#ifndef distidx_defined
#define distidx_defined

#include "collection.h"

class SegMemory;

class DistIndex
{
protected:
    SegMemory&  m_nodeMem;  // handle of memory of distance signature
public:
    Hash        m_nodes;    // hash of distance signatures in memory
public:
    // constructor/destructor
    DistIndex(SegMemory& a_nodeMem);
    virtual ~DistIndex();
    //
    // update
    int writeNode(const int a_nid, Array& a);   // array of distance signatures
    //
    // search
    virtual Array* getNode(const int a_nid);
};

#endif
