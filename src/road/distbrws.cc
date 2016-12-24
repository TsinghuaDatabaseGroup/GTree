/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008
---------------------------------------------------------------------------- */

#include "distbrws.h"
#include "node.h"
#include "segmem.h"
#include "spqdtree.h"

DistBrws::DistBrws(SegMemory& a_nodeMem):
m_nodeMem(a_nodeMem),
m_nodes(1000)
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
}

DistBrws::~DistBrws()
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
        *(int*)&mem[len] = (long)rdr.getKey();   len += sizeof(int);
        *(int*)&mem[len] = (long)rdr.getVal();   len += sizeof(int);
    }
    m_nodeMem.m_header = m_nodeMem.allocate(mem,len);
    delete[] mem;
}

int DistBrws::writeNode(const int a_nid, SPQuadtree* a_spqdtree)
{
    // ------------------------------------------------------------------------
    // allocate a memory block
    // ------------------------------------------------------------------------
    int sz = a_spqdtree->size();
	//char* mem = new char[sz];
	static char* mem = 0;
	static int maxsize = 0;
	if (maxsize < sz)
	{
		if (maxsize != 0) delete[] mem;
		mem = new char[maxsize = sz];
	}
    int len = 0;

    // ------------------------------------------------------------------------
    // convert a shortest path quadtree into a memory byte string
    // ------------------------------------------------------------------------
    a_spqdtree->toMem(mem, len);

    // ------------------------------------------------------------------------
    // store the byte string
    // ------------------------------------------------------------------------
    int pos = m_nodeMem.allocate(mem,len);
    m_nodes.put(a_nid, (void*)pos);
    //delete[] mem;
    return 0;
}

SPQuadtree* DistBrws::getNode(const int a_nid, const Bound& a_bound)
{
    // ------------------------------------------------------------------------
    // retrieve the shortest path quad tree for a node from memory
    // ------------------------------------------------------------------------
    int pos = (long)m_nodes.get(a_nid);
    char* mem = (char*)m_nodeMem.read(pos);
    int len=0;

    // ------------------------------------------------------------------------
    // convert a byte string into a shortest path quadtree
    // ------------------------------------------------------------------------
    SPQuadtree* tree = new SPQuadtree(a_nid, a_bound);
    tree->fromMem(mem, len);

    return tree;
}
