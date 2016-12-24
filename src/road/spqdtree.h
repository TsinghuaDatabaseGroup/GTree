/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class SPQuadtree declaration.
    That represents a shortest path quad-tree for a network node
---------------------------------------------------------------------------- */
#ifndef spquadtree_defined
#define spquadtree_defined

#include "collection.h"
#include "bound.h"
#include <iostream>
class SPQuadtreeNode;
class SPQuadtreeRec;
class Point;

using namespace std;

class SPQuadtree
{
public:
    const int       m_oid;      // a node
    const Bound     m_bound;    // area bound
    SPQuadtreeNode* m_root;     // the root of the tree
protected:
    SPQuadtreeNode* findNode(const Point& a_pt);   // find a node containing this point
    SPQuadtreeNode* findNode(const Bound& a_bd);   // find a node containing this bound
public:
    // constructor/destructor
    SPQuadtree(const int a_oid, const Bound& a_bound);
    virtual ~SPQuadtree();
    //
    // search
    virtual int next(const Point& a_pt);        // determine the next node to go for a point
    virtual float mindist(const Point& a_pt);   // determine the mindist from the node
    //
    // update
    virtual int addObject(SPQuadtreeRec* a_rec);
    virtual int addNode(const SPQuadtreeNode& a_node);
    virtual void finalize();                    // remove all objects and remain info
                                                // for object search
    // display
    virtual void dump(ostream& a_out);
    //
    // memory operations
    void toMem(char* a_mem, int& a_len) const;
    void fromMem(const char* a_mem, int& a_len);
    //
    bool operator==(const SPQuadtree& a_tree) const;
    //
    // info
    virtual int size() const;
};

#endif
