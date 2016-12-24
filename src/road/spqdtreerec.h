/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008

    Copyright(c) Ken C. K. Lee 2008

    This library is for non-commerical use only.
    This header file includes class shortest path quad-tree record declaration
---------------------------------------------------------------------------- */
#ifndef spqdtreerec_defined
#define spqdtreerec_defined

#include "point.h"

class SPQuadtreeRec
{
public:
    const int   m_oid;          // record (destination) id
    Point       m_point;        // coordinate
    int         m_firstnode;    // 1st visited node on a path
    const float m_pathlen;      // path length
public:
    // constructor/destructor
    SPQuadtreeRec(
        const int a_id,
        const Point& a_point,
        const int a_firstnode,
        const float a_pathlen);
    virtual ~SPQuadtreeRec();
    //
    // info
    virtual int size() const;
};

#endif

