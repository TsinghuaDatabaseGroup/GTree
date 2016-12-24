/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008

    Copyright(c) Ken C. K. Lee 2008

    This library is for non-commerical use only.
    This header file includes class shortest path quad-tree node declaration
---------------------------------------------------------------------------- */
#ifndef spqdtreenode_defined
#define spqdtreenode_defined

#include "collection.h"
#include "bound.h"
#include <iostream>
class SPQuadtreeNode;
class SPQuadtreeRec;

using namespace std;

#define INFTY   1e10

class SPQuadtreeNode
{
friend class SPQuadtree;            // allow SPQuadtree to access the attributes
public:
	const int       m_level;        // level in a quadtree
	const Bound		m_bound;        // area covered by this node (cell)
protected:
    int                 m_firstnode;    // first visited network node (-1: undefined)
    int                 m_numchild;     // number of children
	SPQuadtreeNode**    m_child;        // pointers to children nodes
	Array			    m_recs;         // array of records
    float               m_mindist;      // minimum distance to a node
public:
    // constructor/destructor
    SPQuadtreeNode(
        const int a_level,
        const Bound& a_bound,
        const int a_firstvertex=-1,
        const float a_mindist=INFTY);
    virtual ~SPQuadtreeNode();
    //
    // node info for object search
    virtual int next() const;
    virtual float mindist() const;
    //
    // child manipulation
    virtual int numChild() const;
    virtual SPQuadtreeNode* getChild(const int a_i);
    virtual void createChild();
    //
    // object manipulation
    virtual int numRec() const;
	virtual int addRec(SPQuadtreeRec* a_rec);
    virtual SPQuadtreeRec* getRec(const int a_i);
    virtual void removeAllRec();
    //
    virtual void dump(ostream& a_out);
    //
    virtual bool operator==(const SPQuadtreeNode& a_node) const;
    //
    // info
    virtual int size() const;
};

#endif

