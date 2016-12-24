/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008
---------------------------------------------------------------------------- */

#include "spqdtree.h"
#include "spqdtreenode.h"
#include "spqdtreerec.h"
#include "bound.h"
#include "point.h"

SPQuadtree::SPQuadtree(const int a_oid, const Bound& a_bound):
m_oid(a_oid), m_bound(a_bound), m_root(0) 
{}

SPQuadtree::~SPQuadtree()
{
    if (m_root != 0)
        delete m_root;
}

SPQuadtreeNode* SPQuadtree::findNode(const Point& a_pt)
{
    if (m_root == 0)
        m_root = new SPQuadtreeNode(0, m_bound);

    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();
        if (node->m_bound.contain(a_pt))
        {
            if (node->numChild() > 0)
            {
                for (int i=0; i<node->numChild(); i++)
                    s.push(node->getChild(i));
            }
            else
            {
                return node;
            }
        }
    }
    return 0;
}

SPQuadtreeNode* SPQuadtree::findNode(const Bound& a_bd)
{
    if (m_root == 0)
        m_root = new SPQuadtreeNode(0, m_bound);

    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();
        if (node->m_bound.contain(a_bd))
        {
            if (node->numChild() > 0)
            {
                for (int i=0; i<node->numChild(); i++)
                    s.push(node->getChild(i));
            }
            else
            {
                return node;
            }
        }
    }
    return 0;
}

float SPQuadtree::mindist(const Point& a_pt)
{
    SPQuadtreeNode* node = findNode(a_pt);
    return node->mindist();
}

int SPQuadtree::next(const Point& a_pt)
{
    SPQuadtreeNode* node = findNode(a_pt);
    return node->next();
}

int SPQuadtree::addObject(SPQuadtreeRec* a_rec)
{
    SPQuadtreeNode* node = findNode(a_rec->m_point);
    while (node->next() != a_rec->m_firstnode)
    {
        if (node->next() == -1)
            break;
        node->createChild();
        node = findNode(a_rec->m_point);
    }
    return node->addRec(a_rec);
}

int SPQuadtree::addNode(const SPQuadtreeNode& a_node)
{
    SPQuadtreeNode* node = findNode(a_node.m_bound);
    while (!node->m_bound.equal(a_node.m_bound))
    {
        node->createChild();
        node = findNode(a_node.m_bound);
    }
    node->m_firstnode = a_node.m_firstnode;
    node->m_mindist = a_node.m_mindist;
    return 0;
}

void SPQuadtree::finalize()
{
    if (m_root == 0) return;

    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();
        node->removeAllRec();
        for (int i=0; i<node->numChild(); i++)
            s.push(node->getChild(i));
    }
}

void SPQuadtree::dump(ostream& a_out)
{
    if (m_root == 0) return;

    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();
        node->dump(a_out);
        for (int i=0; i<node->numChild(); i++)
            s.push(node->getChild(i));
    }
    return;
}

void SPQuadtree::toMem(char* a_mem, int& a_len) const
{
    if (m_root == 0) return;

    //-------------------------------------------------------------------------
    // keep track of the no. of entries at the head
    //-------------------------------------------------------------------------
    int initlen = 0;
    int numentry = 0;   // the number of leaf entries
    *(int*)&a_mem[initlen = a_len] = numentry;      a_len += sizeof(int);

    //-------------------------------------------------------------------------
    // get all leaf entries and put into memory
    //-------------------------------------------------------------------------
    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();
        if (node->numChild() > 0)
        {
            for (int i=0; i<node->numChild(); i++)
                s.push(node->getChild(i));
        }
        else
        {
            if (node->next() != -1)
            {
                // store the leaf node
                *(int*)&a_mem[a_len] = node->next();                a_len += sizeof(int);
                *(float*)&a_mem[a_len] = node->mindist();           a_len += sizeof(float);
                *(float*)&a_mem[a_len] = node->m_bound.m_lower[0];  a_len += sizeof(float);
                *(float*)&a_mem[a_len] = node->m_bound.m_lower[1];  a_len += sizeof(float);
                *(float*)&a_mem[a_len] = node->m_bound.m_upper[0];  a_len += sizeof(float);
                *(float*)&a_mem[a_len] = node->m_bound.m_upper[1];  a_len += sizeof(float);
                numentry++;
            }
        }
    }
    *(int*)&a_mem[initlen] = numentry;  // keep the number of leaf entries
/*
if (numentry == 1)
{
int debug=10;
}
*/
    /*
    if (m_root == 0) return;

    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();

        *(int*)&a_mem[a_len] = node->next();        a_len += sizeof(int);
        *(float*)&a_mem[a_len] = node->mindist();   a_len += sizeof(float);
        *(int*)&a_mem[a_len] = node->numChild();    a_len += sizeof(int);

        for (int i=0; i<node->numChild(); i++)
            s.push(node->getChild(i));
    }
    */
}

void SPQuadtree::fromMem(const char* a_mem, int& a_len)
{
    //-------------------------------------------------------------------------
    // (re-)create a root
    //-------------------------------------------------------------------------
    if (m_root != 0) delete m_root;    
    m_root = new SPQuadtreeNode(0, m_bound);

    int numentry = 0;
    numentry = *(int*)&a_mem[a_len];                a_len += sizeof(int);
    for (int i=0; i<numentry; i++)
    {
        int nextnode = *(int*)&a_mem[a_len];        a_len += sizeof(int);
        float mindist = *(float*)&a_mem[a_len];     a_len += sizeof(float);
        float p0[2], p1[2];
        p0[0] = *(float*)&a_mem[a_len];             a_len += sizeof(float);
        p0[1] = *(float*)&a_mem[a_len];             a_len += sizeof(float);
        p1[0] = *(float*)&a_mem[a_len];             a_len += sizeof(float);
        p1[1] = *(float*)&a_mem[a_len];             a_len += sizeof(float);

        Point ptl(2,p0), ptu(2,p1);
        Bound bd(ptl,ptu);
        SPQuadtreeNode node(0, bd, nextnode, mindist);

        addNode(node);
    }

    /*
    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();

        node->m_firstnode = *(int*)&a_mem[a_len];   a_len += sizeof(int);
        node->m_mindist   = *(float*)&a_mem[a_len]; a_len += sizeof(float);
        int numchild = *(int*)&a_mem[a_len];        a_len += sizeof(int);
        if (numchild > 0)
        {
            node->createChild();
            for (int i=0; i<numchild; i++)
                s.push(node->getChild(i));
        }
    }
    */
}

bool SPQuadtree::operator==(const SPQuadtree& a_tree) const
{
    return *m_root == *a_tree.m_root;
}

int SPQuadtree::size() const
{
    if (m_root == 0)
        return 0;

    int sz = 0;
    Stack s;
    s.push(m_root);
    while (!s.isEmpty())
    {
        SPQuadtreeNode* node = (SPQuadtreeNode*)s.pop();
        sz += node->size();
        for (int i=0; i<node->numChild(); i++)
            s.push(node->getChild(i));
    }
    return sz;
}

