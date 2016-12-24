/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "access.h"

// constructor/destructor
Access::Access()
{}

Access::~Access()
{
    clean();
}

int Access::length() const
{
    return m_start.size();
}

void Access::get(const int i, int& a_start, int& a_end) const
{
    if (i >= m_start.size())
        a_start = a_end = -1;
    a_start = (long)m_start.get(i);
    a_end = (long)m_end.get(i);
}

void Access::append(const int a_start, const int a_end)
{
    m_start.append((void*)a_start);
    m_end.append((void*)a_end);
}

void Access::clean()
{
    m_start.clean();
    m_end.clean();
};
