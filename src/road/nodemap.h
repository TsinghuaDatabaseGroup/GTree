/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class Mapping declaration.
    That maps objects to a node and vice versa
---------------------------------------------------------------------------- */
#ifndef nodemapping_defined
#define nodemapping_defined

#include "collection.h"

class NodeMapping
{
public:
    Hash    m_node2obj;
    Hash    m_obj2node;
public:
    // constructor/destructor
    NodeMapping();
    virtual ~NodeMapping();
    //
    // update
    void addObject(const int a_nodeid, const int a_objid);
    void delObject(const int a_nodeid, const int a_objid);
    //
    // search
    const Array* findObject(const int a_nodeid) const;
};

#endif

