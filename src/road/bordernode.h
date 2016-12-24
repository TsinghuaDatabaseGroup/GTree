/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class BorderNode declaration.
    It consists of node information and shortcuts/edges to neighbors.
---------------------------------------------------------------------------- */
#ifndef bordernode_defined
#define bordernode_defined

#include "node.h"
#include <iostream>
class ShortcutTreeNode;

class BorderNode: public Node
{
public:
    Array   m_shortcuttree;     // shortcuttree root
    int     m_numlinks;         // the number of shortcuts and edges
    bool    m_isBorder;
public:
    // constructor/destructor
    BorderNode(const int a_id, const float a_x=0, const float a_y=0);
    virtual ~BorderNode();
    //
    // shortcut manipulation
    ShortcutTreeNode* findShortcut(const Array& a_subnet);
    void findSubnets(Array& a_subnet);
    void addEdge(const Array& a_subnet, const Edge& a_edge);
    void delEdge(const Array& a_subnet, const Edge& a_edge);
    //
    void findSubnet(const Edge& a_edge, Array& a_subnet);
    //
    // memory operations
    virtual void toMem(char* a_mem, int& a_len) const;
    virtual void fromMem(char* a_mem, int& a_len);
    virtual int size() const;
    //
    void display(std::ostream& out);
    //
};

#endif
