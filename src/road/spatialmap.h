/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class SpatialMapping declaration.
    That maps objects to a coordinate and vice versa
---------------------------------------------------------------------------- */
#ifndef spatialmapping_defined
#define spatialmapping_defined

#include "collection.h"
#include <math.h>

class ObjectCoor
{
public:
    const int   m_oid;
    const int   m_nid;
    const float m_x, m_y;
public:
    ObjectCoor(const int a_oid, const int a_nid, const float a_x, const float a_y):
        m_oid(a_oid), m_nid(a_nid), m_x(a_x), m_y(a_y) {};
    ~ObjectCoor() {};
    float distance(const float a_x, const float a_y)
    {
        return sqrt((a_x-m_x)*(a_x-m_x) + (a_y-m_y)*(a_y-m_y));
    }
    float distance(const float* a_x, const float* a_y, const int a_cnt)
    {
        float d=0;
        for (int i=0; i<a_cnt; i++)
        {
            float dd = sqrt((a_x[i]-m_x)*(a_x[i]-m_x) + (a_y[i]-m_y)*(a_y[i]-m_y));
            d = d > dd ? d : dd;
        }
        return d;
    }
    static const float* m_refx;
    static const float* m_refy;
    static int          m_cnt;
    static int compare(const void* a0, const void* a1)
    {
        ObjectCoor* c0 = *(ObjectCoor**)a0;
        ObjectCoor* c1 = *(ObjectCoor**)a1;
        float dist0 = c0->distance(m_refx,m_refy,m_cnt);
        float dist1 = c1->distance(m_refx,m_refy,m_cnt);
        if (dist0 < dist1) return -1;
        if (dist0 > dist1) return +1;
        if (c0->m_oid < c1->m_oid) return -1;
        if (c0->m_oid > c1->m_oid) return +1;
        return 0;
    };
};

class SpatialMapping
{
public:
    Array   m_obj2node;
public:
    SpatialMapping();
    virtual ~SpatialMapping();
    //
    // update
    void addObject(const int a_objid, const int a_nodeid, const float m_x, const float m_y);
    void delObject(const int a_objid);
    //
    // search
    const Array* distOrder(const float a_x, const float a_y);
    const Array* distOrder(const float* a_x, const float* a_y, const int a_cnt);
};

#endif
