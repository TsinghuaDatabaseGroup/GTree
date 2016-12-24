#include "graphmap.h"

GraphMapping::GraphMapping()
{}

GraphMapping::~GraphMapping()
{
    for (HashReader rdr(m_graph2obj); !rdr.isEnd(); rdr.next())
        delete (Array*)rdr.getVal();
    m_graph2obj.clean();
    m_obj2graph.clean();
}

void GraphMapping::addObject(const int a_graphid, const int a_objid)
{
    // ------------------------------------------------------------------------
    // associate an object to a subgraph
    // ------------------------------------------------------------------------
    Array* a = (Array*)m_graph2obj.get(a_graphid);
    if (a == 0)
        m_graph2obj.put(a_graphid, a=new Array());
    a->append((void*)a_objid);
    a->removeDuplicate();

    // ------------------------------------------------------------------------
    // associate a graph to an object
    // ------------------------------------------------------------------------
    int graphid = (long)m_obj2graph.get(a_objid);
    if (graphid == 0)
        m_obj2graph.put(a_objid,(void*)a_graphid);
    return;
}

void GraphMapping::delObject(const int a_graphid, const int a_objid)
{
    // ------------------------------------------------------------------------
    // delete an object from a node
    // ------------------------------------------------------------------------
    Array* a = (Array*)m_graph2obj.get(a_graphid);
    if (a != 0)
        a->remove((void*)a_objid);

    // ------------------------------------------------------------------------
    // delete a node from an object
    // ------------------------------------------------------------------------
    m_obj2graph.remove(a_objid);
}

const Array* GraphMapping::findObject(const int a_graphid) const
{
    // ------------------------------------------------------------------------
    // find objects from a node
    // ------------------------------------------------------------------------
    return (const Array*)m_graph2obj.get(a_graphid);
}
