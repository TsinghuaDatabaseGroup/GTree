/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2008

    This library contains an Access class declaration.
    It provides facility to keep track memory access
---------------------------------------------------------------------------- */
#ifndef access_defined
#define access_defined

#include "collection.h"

class Access
{
protected:
    Array   m_start;    // start addresses
    Array   m_end;      // end addresses
public:
    // constructor/destructor
    Access();
    virtual ~Access();
    //
    int length() const;
    void get(const int i, int& a_start, int& a_end) const;
    void append(const int a_start, const int a_end);
    void clean();
};

#endif

