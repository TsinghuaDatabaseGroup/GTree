/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008
---------------------------------------------------------------------------- */

#include "bound.h"
#include <math.h>

// constructor/destructor
Bound::Bound(const Point& a_lower, const Point& a_upper):
m_lower(a_lower), m_upper(a_upper)
{}

Bound::~Bound()
{}

// search
int Bound::dimen() const
{
	return m_lower.m_dimen;
}

Point Bound::center() const
{
	return Point::midpoint(m_lower, m_upper);
}

// is this bound containing another (in param)
bool Bound::contain(const Bound& a_bound) const
{
    for (int i=0; i<dimen(); i++)
    {
        if (m_lower[i] > a_bound.m_lower[i]) return false;
        if (m_upper[i] < a_bound.m_upper[i]) return false;
    }
    return true;
}

// is this bound containing another (in param)
bool Bound::contain(const Point& a_pt) const
{
    for (int i=0; i<dimen(); i++)
    {
        if (m_lower[i] > a_pt[i]) return false;
        if (m_upper[i] < a_pt[i]) return false;
    }
    return true;
}

bool Bound::equal(const Bound& a_bd) const
{
    for (int i=0; i<dimen(); i++)
    {
        if (m_lower[i] != a_bd.m_lower[i]) return false;
        if (m_upper[i] != a_bd.m_upper[i]) return false;
    }
    return true;
}

float Bound::mindist(const Point& a_pt) const
{
    float dist=0;
    for (int i=0; i<dimen(); i++)
    {
        float d = 0;
        if (a_pt[i] < m_lower[i])
            d = m_lower[i] - a_pt[i];
        else if (a_pt[i] > m_upper[i])
            d = a_pt[i] - m_upper[i];
        dist += d*d;
    }
    return sqrt(dist);
}


// info
int Bound::size(const int a_dimen)
{
	return Point::size(a_dimen)*2;
}
