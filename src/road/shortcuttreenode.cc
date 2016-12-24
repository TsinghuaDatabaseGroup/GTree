/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "edge.h"
#include "shortcuttreenode.h"

ShortcutTreeNode::ShortcutTreeNode(const int a_subnetid):
m_subnetid(a_subnetid),
m_child(2,2),
m_edges(5,5)
{}

ShortcutTreeNode::~ShortcutTreeNode()
{
    for (int i=0; i<m_edges.size(); i++)
        delete (Edge*)m_edges.get(i);
    for (int i=0; i<m_child.size(); i++)
        delete (ShortcutTreeNode*)m_child.get(i);
}

void ShortcutTreeNode::condense(Array& remain, Array& del)
{
    if (m_edges.size() == 0)
    {
        for (int i=0; i<m_child.size(); i++)
        {
            ShortcutTreeNode* sc = (ShortcutTreeNode*)m_child.get(i);
            sc->condense(remain,del);
        }
        del.append(this);
    }
    else
    {
        remain.append(this);
    }
}
