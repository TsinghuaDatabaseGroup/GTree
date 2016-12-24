/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "bordernode.h"
#include "edge.h"
#include "shortcuttreenode.h"


BorderNode::BorderNode(const int a_id, const float a_x, const float a_y):
Node(a_id,a_x,a_y),
m_numlinks(0),
m_shortcuttree(2,2),
m_isBorder(false)
{}

BorderNode::~BorderNode()
{
    for (int i=0; i<m_shortcuttree.size(); i++)
        delete (ShortcutTreeNode*)m_shortcuttree.get(i);
}

ShortcutTreeNode* BorderNode::findShortcut(const Array& a_subnet)
{
    // ------------------------------------------------------------------------
    // locate shortcut
    // ------------------------------------------------------------------------
    Array* a = &m_shortcuttree;
    for (int level=0; level<a_subnet.size(); level++)
    {
        int subnetid = (long)a_subnet.get(level);
        ShortcutTreeNode* target = 0;
        for (int i=0; i<a->size() && target==0; i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            if (sc->m_subnetid != subnetid) continue;
            target = sc;
        }
        if (target == 0)
        {
            target = new ShortcutTreeNode(subnetid);
            a->append(target);
            if (a->size() > 1) m_isBorder = true;
        }
        if (level == a_subnet.size()-1)
            return target;
        a = &target->m_child;
    }
    return 0;
}

void BorderNode::findSubnets(Array& a_subnet)
{
    Queue q;
    q.enqueue(&m_shortcuttree);
    while (!q.isEmpty())
    {
        Array* a = (Array*)q.dequeue();
        for (int i=0; i<a->size(); i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            if (sc->m_subnetid == 0) continue;
            a_subnet.append((void*)sc->m_subnetid);
            q.enqueue(&sc->m_child);
        }
    }
    a_subnet.removeDuplicate();
    return;
}


void BorderNode::addEdge(const Array& a_subnet, const Edge& a_edge)
{
    // ------------------------------------------------------------------------
    // find the taget shortcut entry
    // ------------------------------------------------------------------------
    ShortcutTreeNode* sc = findShortcut(a_subnet);

    // ------------------------------------------------------------------------
    // add an edge
    // ------------------------------------------------------------------------
    int sz = sc->m_edges.size();
    for (int i=0; i<sz; i++)
    {
        Edge* e = (Edge*)sc->m_edges.get(i);
        if (e->m_neighbor == a_edge.m_neighbor)
        {
            e->m_cost = a_edge.m_cost;
            return;
        }
    }
    sc->m_edges.append(new Edge(a_edge.m_neighbor, a_edge.m_cost));
    m_numlinks++;
}

void BorderNode::delEdge(const Array& a_subnet, const Edge& a_edge)
{
    // ------------------------------------------------------------------------
    // find the taget shortcut entry
    // ------------------------------------------------------------------------
    ShortcutTreeNode* sc = findShortcut(a_subnet);

    // ------------------------------------------------------------------------
    // remove an edge
    // ------------------------------------------------------------------------
    int sz = sc->m_edges.size();
    for (int i=0; i<sz; i++)
    {
        Edge* e = (Edge*)sc->m_edges.get(i);
        if (e->m_neighbor == a_edge.m_neighbor)
        {
            sc->m_edges.remove(e);
            m_numlinks--;
            return;
        }
    }
}

void findSubnet(const Edge& a_edge, Array& a_subnet)
{
    /*
    class carrier
    {
    public:
        Array   m_subnet;
        Array*  m_shortcuttree;
    public:
        carrier(const Array& a_subnet, const int a_subnetid, Array* a_tree):
    };

    Array* a = &m_shortcuttree;
    */
}

void BorderNode::toMem(char* a_mem, int& a_len) const
{
    *(int*)&a_mem[a_len] = m_id;    a_len += sizeof(m_id);
    *(float*)&a_mem[a_len] = m_x;   a_len += sizeof(m_x);
    *(float*)&a_mem[a_len] = m_y;   a_len += sizeof(m_y);
    Stack s;
    s.push((void*)&m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        int branch = a->size();
        *(int*)&a_mem[a_len] = branch;                  a_len += sizeof(int);
        for (int i=0; i<branch; i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            int subnetid = sc->m_subnetid;
            int numedge = sc->m_edges.size();
            *(int*)&a_mem[a_len] = subnetid;            a_len += sizeof(int);
            *(int*)&a_mem[a_len] = numedge;             a_len += sizeof(int);
            for (int j=0; j<numedge; j++)
            {
                Edge* e = (Edge*)sc->m_edges.get(j);
                *(int*)&a_mem[a_len] = e->m_neighbor;   a_len += sizeof(int);
                *(float*)&a_mem[a_len] = e->m_cost;     a_len += sizeof(float);
            }
            int numchild = sc->m_child.size();
            *(int*)&a_mem[a_len] = numchild;            a_len += sizeof(int);
            if (numchild > 0)
                s.push(&sc->m_child);
        }
    }
    return;
}

void BorderNode::fromMem(char* a_mem, int& a_len)
{
    m_id = *(int*)&a_mem[a_len];    a_len += sizeof(m_id);
    m_x = *(float*)&a_mem[a_len];   a_len += sizeof(m_x);
    m_y = *(float*)&a_mem[a_len];   a_len += sizeof(m_y);
    m_isBorder = false;
    Stack s;
    s.push((void*)&m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        int branch = *(int*)&a_mem[a_len];          a_len += sizeof(int);
        if (branch > 1) m_isBorder = true;
        for (int i=0; i<branch; i++)
        {
            int subnetid = *(int*)&a_mem[a_len];    a_len += sizeof(int);
            ShortcutTreeNode* sc = new ShortcutTreeNode(subnetid);
            a->append(sc);
            int numedge = *(int*)&a_mem[a_len];     a_len += sizeof(int);
            for (int j=0; j<numedge; j++)
            {
                int n = *(int*)&a_mem[a_len];       a_len += sizeof(int);
                float c = *(float*)&a_mem[a_len];   a_len += sizeof(float);
                sc->m_edges.append(new Edge(n,c));
                m_numlinks++;                       // record the number of links
                if (subnetid == 0)                  // record the original edges
                    m_edges.append(new Edge(n,c));
            }
            int numchild = *(int*)&a_mem[a_len];    a_len += sizeof(int);
            if (numchild > 0)
                s.push(&sc->m_child);
        }
    }
}

int BorderNode::size() const
{
    int sz = sizeof(int)*3;
    Stack s;
    s.push((void*)&m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        int branch = a->size();
        sz += sizeof(int);
        for (int i=0; i<branch; i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            sz += sizeof(int)*3;
            sz += sizeof(int)*sc->m_edges.size();
            if (sc->m_child.size() > 0)
                s.push(&sc->m_child);
        }
    }
    return sz;
}


void BorderNode::display(std::ostream& out)
{
    out << m_id << ":";

    using namespace std;

    Stack s;
    s.push(&m_shortcuttree);
    while (!s.isEmpty())
    {
        Array* a = (Array*)s.pop();
        for (int i=0; i<a->size(); i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)a->get(i);
            if (sc->m_edges.size() > 0)
            {
                out << sc->m_subnetid << "-{";
                for (int j=0; j<sc->m_edges.size(); j++)
                {
                    Edge* e = (Edge*)sc->m_edges.get(j);
                    out << e->m_neighbor << "," << e->m_cost;;
                }
                out << "};";
            }
            s.push(&sc->m_child);
        }
    }
    out << endl;
    return;
}
