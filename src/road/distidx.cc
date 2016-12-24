/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "distidx.h"
#include "node.h"
#include "segmem.h"
#include "distsign.h"

DistIndex::DistIndex(SegMemory& a_nodeMem):
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
}

DistIndex::~DistIndex()
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

int DistIndex::writeNode(const int a_nid, Array& a)
{
    // ------------------------------------------------------------------------
    // ordering objects by non-decreasing cost
    // ------------------------------------------------------------------------
    a.sort(DistSignature::compareID);

    // ------------------------------------------------------------------------
    // write a node into memory
    // ------------------------------------------------------------------------
    int n = a.size() * 20;
    char* mem = new char[n];
    int len=0;
    *(int*)&mem[len] = a.size();        len += sizeof(int);
    for (int i=0; i<a.size(); i++)
    {
        DistSignature* s = (DistSignature*)a.get(i);
        *(int*)&mem[len] = s->m_oid;    len += sizeof(int);
        *(float*)&mem[len] = s->m_cost; len += sizeof(int);
        *(int*)&mem[len] = s->m_prev;   len += sizeof(int);
    }
    int pos = m_nodeMem.allocate(mem,len);
    m_nodes.put(a_nid, (void*)pos);
    delete[] mem;
    return 0;
}

Array* DistIndex::getNode(const int a_nid)
{
    // ------------------------------------------------------------------------
    // retrieve the distance signature of a node from memory
    // ------------------------------------------------------------------------
    int pos = (long)m_nodes.get(a_nid);
    char* mem = (char*)m_nodeMem.read(pos);
    int len=0;

    Array* a = new Array;
    int sz = *(int*)&mem[len];              len += sizeof(int);
    for (int i=0; i<sz; i++)
    {
        int oid = *(int*)&mem[len];         len += sizeof(int);
        float cost = *(float*)&mem[len];    len += sizeof(float);
        int prev = *(int*)&mem[len];        len += sizeof(int);
        a->append(new DistSignature(oid, cost, prev));
    }
    return a;
}
