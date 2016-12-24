/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class GraphMapping declaration.
    That maps objects to a subgraph and vice versa
---------------------------------------------------------------------------- */
#ifndef graphmapping_defined
#define graphmapping_defined

#include "collection.h"

class GraphMapping
{
public:
    Hash    m_graph2obj;
    Hash    m_obj2graph;
public:
    GraphMapping();
    virtual ~GraphMapping();
    //
    // update
    void addObject(const int a_graphid, const int a_objid);
    void delObject(const int a_graphid, const int a_objid);
    //
    // search
    const Array* findObject(const int a_graphid) const;
};

#endif




