/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "graph.h"
#include "node.h"
#include "segmem.h"

// constructor/destructor
Graph::Graph(SegMemory& a_nodeMem):
m_nodeMem(a_nodeMem),
m_nodes(10000)
{
    if (m_nodeMem.m_header == -1) return;

    // ------------------------------------------------------------------------
    // load hash table of nodes from memory
    // ------------------------------------------------------------------------
    char* nodepos = (char*)m_nodeMem.read(m_nodeMem.m_header);
    int len=0;
    int sz = *(int*)&nodepos[len];              len+=sizeof(int);
    for (int i=0; i<sz; i++)
    {
        int nodeid = *(int*)&nodepos[len];      len+=sizeof(int);
        int pos = *(int*)&nodepos[len];         len+=sizeof(int);
        m_nodes.put(nodeid,(void*)pos);
    }

    // ------------------------------------------------------------------------
    // reset memory access history (which is used to estimate IO cost)
    // ------------------------------------------------------------------------
    m_nodeMem.m_history.clean();
};

Graph::~Graph()
{
    // ------------------------------------------------------------------------
    // clean the header (meta data) in memory
    // ------------------------------------------------------------------------
    int sz = m_nodes.size();
    if (sz == 0) return;
    if (m_nodeMem.m_header != -1)           // remove header
        m_nodeMem.free(m_nodeMem.m_header);

    // ------------------------------------------------------------------------
    // write the header (meta data) into memory
    // ------------------------------------------------------------------------
    int len=0;
    char* mem = new char[sz*sizeof(int)*2 + sizeof(int)];
    *(int*)&mem[len] = sz;                      len += sizeof(int);
    for (HashReader rdr(m_nodes); !rdr.isEnd(); rdr.next())
    {
        *(int*)&mem[len] = (int)rdr.getKey();   len += sizeof(int);
        *(int*)&mem[len] = (long)rdr.getVal();   len += sizeof(int);
    }
    m_nodeMem.m_header = m_nodeMem.allocate(mem,len);
    delete[] mem;
}

// update
int Graph::writeNode(const int a_nid, const Node& a_node)
{
    // ------------------------------------------------------------------------
    // write a node into memory
    // ------------------------------------------------------------------------
    char* mem = new char[a_node.m_edges.size() * 8 + 30];
    int len=0;
    a_node.toMem(mem,len);  // content marshalling
    int pos = m_nodeMem.allocate(mem,len);
    m_nodes.put(a_nid, (void*)pos);
    delete[] mem;
    return 0;
}

// search
Node* Graph::getNode(const int a_nid)
{
    // ------------------------------------------------------------------------
    // retrieve a node from memory
    // ------------------------------------------------------------------------
    int pos = (long)m_nodes.get(a_nid);
    char* mem = (char*)m_nodeMem.read(pos);
    int len=0;
    Node* n = new Node(a_nid);
    n->fromMem(mem,len);    // content unmarshalling
    return n;
}
