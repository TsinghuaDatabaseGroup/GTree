/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class ObjectGen declaration.
    It is used to generate object on a network.
---------------------------------------------------------------------------- */
#ifndef objectgen_defined
#define objectgen_defined

#include "collection.h"
class Node;

class NodeObject
{
public:
    const int   m_nodeid;
    const int   m_objid;
public:
    NodeObject(const int a_nodeid, const int a_objid):
        m_nodeid(a_nodeid), m_objid(a_objid){};
    virtual ~NodeObject() {};
};

class ObjectGen
{
public:
    static void uniform(
        const int a_numobj, Array& a_allnodes,
        Array& a_map);
    static void cluster(
        const int a_numcluster, const int a_numobj,
        Array& a_allnodes,
        Array& a_map);
};

#endif
