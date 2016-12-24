/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "hiergraph.h"
#include "bordernode.h"
#include "segmem.h"

HierGraph::HierGraph(SegMemory& a_nodeMem):
Graph(a_nodeMem)
{};

HierGraph::~HierGraph()
{}

int HierGraph::writeNode(const int a_nid, const BorderNode& a_bnode)
{
    // ------------------------------------------------------------------------
    // write a node into memory
    // ------------------------------------------------------------------------
    //char mem[4096];
    int sz = a_bnode.size();
    char* mem = new char[sz*2];
    int len=0;
    a_bnode.toMem(mem,len);  // content marshalling
    int pos = m_nodeMem.allocate(mem,len);
    m_nodes.put(a_nid, (void*)pos);
    delete[] mem;
    return 0;
}

Node* HierGraph::getNode(const int a_nid)
{
    return getBorderNode(a_nid);
}

BorderNode* HierGraph::getBorderNode(const int a_nid)
{
    // ------------------------------------------------------------------------
    // retrieve a node from memory
    // ------------------------------------------------------------------------
    int pos = (long)m_nodes.get(a_nid);
    char* mem = (char*)m_nodeMem.read(pos);
    int len=0;
    BorderNode* n = new BorderNode(a_nid);
    n->fromMem(mem,len);    // content unmarshalling
    return n;
}
