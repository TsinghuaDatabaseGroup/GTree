/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2008

    This header file declares Bound class.
	This defines the volume bound.
---------------------------------------------------------------------------- */
#ifndef BOUND_DEFINED
#define BOUND_DEFINED

#include "point.h"

class Bound
{
public:
	const Point	m_lower;	// lower bound
	const Point m_upper;	// upper bound
public:
	// constructor/destructor
	Bound(const Point& a_lower, const Point& a_upper);
	virtual ~Bound();
	//
	// search
	virtual int dimen() const;
	virtual Point center() const;
    //
    // is this bound containing another (in param)
    virtual bool contain(const Bound& a_bound) const;
    virtual bool contain(const Point& a_point) const;
    virtual float mindist(const Point& a_pt) const;
    virtual bool equal(const Bound& a_bound) const;
    //
    // info
    static int size(const int a_dimen);	// storage size
};

#endif 

