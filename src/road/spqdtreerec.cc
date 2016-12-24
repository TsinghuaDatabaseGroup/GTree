/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008
---------------------------------------------------------------------------- */

#include "spqdtreerec.h"

SPQuadtreeRec::SPQuadtreeRec(const int a_oid,
                             const Point& a_point,
                             const int a_firstnode,
                             const float a_pathlen):
m_oid(a_oid), m_point(a_point), m_firstnode(a_firstnode), m_pathlen(a_pathlen)
{}

SPQuadtreeRec::~SPQuadtreeRec()
{}

int SPQuadtreeRec::size() const
{
    int sz = 
        sizeof(m_oid) +
        Point::size(m_point.m_dimen) +
        sizeof(m_firstnode) +
        sizeof(m_pathlen);
    return sz;
}


