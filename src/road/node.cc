/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "node.h"
#include "edge.h"
#include <string.h>

// constructor/destructor
Node::Node(const int a_id, float a_x, float a_y):
m_id(a_id), m_x(a_x), m_y(a_y)
{}

Node::~Node()
{
    // ------------------------------------------------------------------------
    // clean up edges (free the memory)
    // ------------------------------------------------------------------------
    int sz = m_edges.size();
    for (int i=0; i<sz; i++)
        delete (Edge*)m_edges.get(i);
    m_edges.clean();
}

// search
float Node::cost(const int a_node)
{
    for (int i=0; i<m_edges.size(); i++)
    {
        Edge* e = (Edge*)m_edges.get(i);
        if (e->m_neighbor == a_node)
            return e->m_cost;
    }
    return 0;
}


// edge manipulations
void Node::addEdge(Edge& a_edge)
{
    // ------------------------------------------------------------------------
    // add an edge to neighbor
    // * if an edge to the same neighbor exists, update its cost.
    // ------------------------------------------------------------------------
    int sz = m_edges.size();
    for (int i=0; i<sz; i++)
    {
        Edge* e = (Edge*)m_edges.get(i);
        if (e->m_neighbor == a_edge.m_neighbor)
        {
            e->m_cost = a_edge.m_cost;
            return;
        }
    }
    m_edges.append(new Edge(a_edge.m_neighbor,a_edge.m_cost));
}

void Node::delEdge(Edge& a_edge)
{
    // ------------------------------------------------------------------------
    // remove an edge
    // ------------------------------------------------------------------------
    int sz = m_edges.size();
    for (int i=0; i<sz; i++)
    {
        Edge* e = (Edge*)m_edges.get(i);
        if (e->m_neighbor == a_edge.m_neighbor)
        {
            m_edges.remove(e);
            return;
        }
    }
}

// memory operations
void Node::toMem(char* a_mem, int& a_len) const
{
    int numedge = m_edges.size();
    *(int*)&a_mem[a_len] = m_id;    a_len += sizeof(m_id);
    *(float*)&a_mem[a_len] = m_x;   a_len += sizeof(m_x);
    *(float*)&a_mem[a_len] = m_y;   a_len += sizeof(m_y);
    *(int*)&a_mem[a_len] = numedge; a_len += sizeof(int);
    for (int i=0; i<numedge; i++)
    {
        Edge* e = (Edge*)m_edges.get(i);
        *(int*)&a_mem[a_len] = e->m_neighbor;   a_len += sizeof(e->m_neighbor);
        *(float*)&a_mem[a_len] = e->m_cost;     a_len += sizeof(e->m_cost);
    }
}

void Node::fromMem(char* a_mem, int& a_len)
{
    int sz=0;
    m_id = *(int*)&a_mem[a_len];    a_len += sizeof(m_id);
    m_x = *(float*)&a_mem[a_len];   a_len += sizeof(m_x);
    m_y = *(float*)&a_mem[a_len];   a_len += sizeof(m_y);
    sz = *(int*)&a_mem[a_len];      a_len += sizeof(sz);
    for (int i=0; i<sz; i++)
    {
        int nid;
        float cost;
        nid = *(int*)&a_mem[a_len];     a_len += sizeof(nid);
        cost= *(float*)&a_mem[a_len];   a_len += sizeof(cost);
        m_edges.append(new Edge(nid,cost));
    }
}

int Node::compareid(const void* a0, const void* a1)
{
    Node* n0 = *(Node**)a0;
    Node* n1 = *(Node**)a1;
    if (n0->m_id < n1->m_id) return -1;
    if (n0->m_id > n1->m_id) return +1;
    return 0;
}
