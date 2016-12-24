/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class ShortcutTreeNode declaration.
    This presents a node that contains
    - shortcut/edge with a cost to a neighboring bordernode/node.
---------------------------------------------------------------------------- */
#ifndef shortcuttreenode_defined
#define shortcuttreenode_defined

// DESCRIPTION:
//    this organizes shortcuts in different level of a graph as a tree.

#include "collection.h"

class ShortcutTreeNode
{
public:
    const int   m_subnetid;
    Array       m_edges;        // edges to neighboring bordernodes/nodes
    Array       m_child;        // child shortcut
public:
    ShortcutTreeNode(const int a_subnetid);
    virtual ~ShortcutTreeNode();

    void condense(Array& remain, Array& del);
};

#endif


