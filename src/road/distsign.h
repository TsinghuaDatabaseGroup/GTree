/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class DistSignature declaration.
    That represents a distance signature for an object at a node
---------------------------------------------------------------------------- */
#ifndef distsign_defined
#define distsign_defined

#include "collection.h"

class DistSignature
{
public:
    const int   m_oid;  // an object
    const float m_cost; // distance to the object
    const int   m_prev; // a connecting node closer to the object
public:
    DistSignature(
        const int a_oid, const float a_cost, const int a_prev):
        m_oid(a_oid), m_cost(a_cost), m_prev(a_prev) {};
    virtual ~DistSignature() {};
    static int compare(const void* a0, const void* a1)
    {
        DistSignature* o0 = *(DistSignature**)a0;
        DistSignature* o1 = *(DistSignature**)a1;
        if (o0->m_cost < o1->m_cost) return -1;
        if (o0->m_cost > o1->m_cost) return +1;
        if (o0->m_oid < o1->m_oid) return -1;
        if (o0->m_oid > o1->m_oid) return +1;
        return 0;
    };
    static int compareID(const void* a0, const void* a1)
    {
        DistSignature* o0 = *(DistSignature**)a0;
        DistSignature* o1 = *(DistSignature**)a1;
        if (o0->m_oid < o1->m_oid) return -1;
        if (o0->m_oid > o1->m_oid) return +1;
        return 0;
    };
};

#endif
