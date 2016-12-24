/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "spatialmap.h"

const float* ObjectCoor::m_refx=0;
const float* ObjectCoor::m_refy=0;
int ObjectCoor::m_cnt=0;

// constructor/destructor
SpatialMapping::SpatialMapping()
{
    m_obj2node.clean();
}

SpatialMapping::~SpatialMapping()
{
    for (int i=0; i<m_obj2node.size(); i++)
        delete (ObjectCoor*)m_obj2node.get(i);
    m_obj2node.clean();
}

void SpatialMapping::addObject(const int a_objid, const int a_nodeid,
                               const float m_x, const float m_y)
{
    m_obj2node.append(new ObjectCoor(a_objid, a_nodeid, m_x, m_y));
}

void SpatialMapping::delObject(const int a_objid)
{
    for (int i=0; i<m_obj2node.size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)m_obj2node.get(i);
        if (c->m_oid == a_objid)
        {
            m_obj2node.remove(c);
            delete c;
            break;
        }
    }
}

const Array* SpatialMapping::distOrder(const float a_x, const float a_y)
{
    ObjectCoor::m_refx = &a_x;
    ObjectCoor::m_refy = &a_y;
    ObjectCoor::m_cnt = 1;
    m_obj2node.sort(ObjectCoor::compare);
    return &m_obj2node;
}

const Array* SpatialMapping::distOrder(const float* a_x, const float* a_y, const int a_cnt)
{
    ObjectCoor::m_refx = a_x;
    ObjectCoor::m_refy = a_y;
    ObjectCoor::m_cnt = a_cnt;
    m_obj2node.sort(ObjectCoor::compare);
    return &m_obj2node;
}
