/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008
---------------------------------------------------------------------------- */

#include "spqdtreenode.h"
#include "spqdtreerec.h"
#include <math.h>
#include <iostream>

using namespace std;

SPQuadtreeNode::SPQuadtreeNode(const int a_level,
                               const Bound& a_bound,
                               const int a_firstnode,
                               const float a_mindist):
m_level(a_level), m_bound(a_bound), m_firstnode(a_firstnode), m_mindist(a_mindist),
m_numchild(0), m_child(0)
{}

SPQuadtreeNode::~SPQuadtreeNode()
{
    // ------------------------------------------------------------------------
    // delete all objects
    // ------------------------------------------------------------------------
    for (int i=0; i<m_recs.size(); i++)
        delete (SPQuadtreeRec*)m_recs.get(i);

    // ------------------------------------------------------------------------
    // delete all children nodes
    // ------------------------------------------------------------------------
    if (m_numchild > 0)
    {
        for (int i=0; i<m_numchild; i++)
            delete m_child[i];
        delete[] m_child;
    }
}

// node info for object search
int SPQuadtreeNode::next() const
{
    return m_firstnode;
}

float SPQuadtreeNode::mindist() const
{
    return m_mindist;
}


// child manipulation
int SPQuadtreeNode::numChild() const
{
    return m_numchild;
}

SPQuadtreeNode* SPQuadtreeNode::getChild(const int a_i)
{
    return m_child[a_i];
}

void SPQuadtreeNode::createChild()
{
    if (m_numchild != 0) return;

    // ------------------------------------------------------------------------
    // create a set of child nodes
    // ------------------------------------------------------------------------
    m_numchild = (int)pow(2.0f, m_bound.dimen());
    m_child = new SPQuadtreeNode*[m_numchild];

    // ------------------------------------------------------------------------
    // create individual child nodes
    // find the bound for each child
    // ------------------------------------------------------------------------
    const Point center = m_bound.center();
    const int dimen = center.m_dimen;
    for (int i=0; i<m_numchild; i++)
    {
        float l[MAXDIMEN];
        float u[MAXDIMEN];
        int coor = i;
        for (int d=0; d<dimen; d++)
        {
            if (coor % 2 == 0)
            {
                l[d] = m_bound.m_lower[d];
                u[d] = center[d];
            }
            else
            {
                l[d] = center[d];
                u[d] = m_bound.m_upper[d];
            }
            coor /= 2;
        }
        Point ptl(dimen, l), ptu(dimen, u);
        Bound bound(ptl, ptu);
        m_child[i] = new SPQuadtreeNode(m_level+1, bound);
    }

    // ------------------------------------------------------------------------
    // reallocate all records to child nodes
    // ------------------------------------------------------------------------
    for (int i=0; i<m_recs.size(); i++)
    {
        SPQuadtreeRec* rec = (SPQuadtreeRec*)m_recs.get(i);
        for (int j=0; j<m_numchild; j++)
        {
            if (m_child[j]->m_bound.contain(rec->m_point))
            {
                m_child[j]->addRec(rec);
                break;
            }
        }
    }
    m_recs.clean();
    m_firstnode = -1;
    m_mindist = INFTY;
}

// object manipulation
int SPQuadtreeNode::numRec() const
{
    return m_recs.size();
}

int SPQuadtreeNode::addRec(SPQuadtreeRec* a_obj)
{
    // ------------------------------------------------------------------------
    // update the node info (first node to reach this area and mindist)
    // ------------------------------------------------------------------------
    m_firstnode = a_obj->m_firstnode;
    m_mindist = m_mindist < a_obj->m_pathlen ? m_mindist : a_obj->m_pathlen;

    return m_recs.append(a_obj);
}

SPQuadtreeRec* SPQuadtreeNode::getRec(const int a_i)
{
    return (SPQuadtreeRec*)m_recs.get(a_i);
}

void SPQuadtreeNode::removeAllRec()
{
    for (int i=0; i<m_recs.size(); i++)
        delete (SPQuadtreeNode*)m_recs.get(i);
    m_recs.clean();
}

void SPQuadtreeNode::dump(ostream& a_out)
{
    if (m_recs.size() > 10)
    {
        int debug=10;
    }

    if (m_firstnode == -1) return;

    for (int i=0; i<m_level; i++)
        a_out << " ";
    for (int d=0; d<m_bound.dimen(); d++)
        a_out << "(" << m_bound.m_lower[d] << "," << m_bound.m_upper[d] << ") ";
    a_out << "#rec:" << m_recs.size() << " f.n.:" << m_firstnode << " mdist:" << m_mindist << endl;
    /*
    // debug
    for (int i=0; i<m_numchild; i++)
    {
        for (int d=0; d<2; d++)
        {
            cout << "(";
            cout << m_child[i]->m_bound.m_lower[d];
            cout << ",";
            cout << m_child[i]->m_bound.m_upper[d];
            cout << ")-";
        }
        cout << endl;
    }
    */

}

bool SPQuadtreeNode::operator==(const SPQuadtreeNode& a_node) const
{
    if (m_level != a_node.m_level) return false;
    if (m_firstnode != a_node.m_firstnode) return false;
    if (m_mindist != a_node.m_mindist) return false;
    if (m_numchild != a_node.m_numchild) return false;

    for (int i=0; i<m_numchild; i++)
        if (!(*m_child[i] == *a_node.m_child[i])) return false;
    return true;
}


// info
int SPQuadtreeNode::size() const
{
    int sz =
        sizeof(m_level) +
        Bound::size(m_bound.dimen()) +
        sizeof(m_firstnode) +
        sizeof(SPQuadtreeNode*) * m_numchild;
    if (m_recs.size() > 0)
        sz += m_recs.size() * ((SPQuadtreeRec*)m_recs.get(0))->size();
    return sz;
}
