/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008

    Copyright(c) Ken C. K. Lee 2008

    This library is for non-commerical use only.
    This header file includes class shortest path quad-tree search declaration
---------------------------------------------------------------------------- */
#ifndef spqdtreesearch_defined
#define spqdtreesearch_defined

class SPQuadtree;
class SPQuadtreeNode;
class Point;

class SPQuadtreeSearch
{
public:
    static SPQuadtreeNode* ptSearch(SPQuadtree& a_tree, Point& a_pt);
};

#endif

