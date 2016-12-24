/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "nodemap.h"

// constructor/destructor
NodeMapping::NodeMapping()
{}

NodeMapping::~NodeMapping()
{}

void NodeMapping::addObject(const int a_nodeid, const int a_objid)
{
    // ------------------------------------------------------------------------
    // associate an object to a node
    // ------------------------------------------------------------------------
    Array* a = (Array*)m_node2obj.get(a_nodeid);
    if (a == 0)
        m_node2obj.put(a_nodeid, a = new Array());
    a->append((void*)a_objid);
    a->sort();

    // ------------------------------------------------------------------------
    // associate a node to an object
    // ------------------------------------------------------------------------
    int nodeid = (long)m_obj2node.get(a_objid);
    if (nodeid == 0)
        m_obj2node.put(a_nodeid,(void*)a_nodeid);
    return;
}

void NodeMapping::delObject(const int a_nodeid, const int a_objid)
{
    // ------------------------------------------------------------------------
    // delete an object from a node
    // ------------------------------------------------------------------------
    Array* a = (Array*)m_node2obj.get(a_nodeid);
    if (a != 0)
        a->remove((void*)a_objid);

    // ------------------------------------------------------------------------
    // delete a node from an object
    // ------------------------------------------------------------------------
    m_obj2node.remove(a_objid);
}

const Array* NodeMapping::findObject(const int a_nodeid) const
{
    // ------------------------------------------------------------------------
    // find objects from a node
    // ------------------------------------------------------------------------
    return (const Array*)m_node2obj.get(a_nodeid);
}
