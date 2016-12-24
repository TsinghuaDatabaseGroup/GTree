/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2008

    This library contains an abstract SegmentMemory class declaration.
    It provides facilities to read/allocate/free space for records of any size.
---------------------------------------------------------------------------- */
#ifndef segmem_defined
#define segmem_defined

#include "access.h"

class SegMemory
{
public:
    int     m_header;       // position of header infomation
    Access  m_history;      // access history
public:
    // constructor/destructor
    SegMemory(): m_header(-1) {};
    virtual ~SegMemory() {};
    //
    // read/allocate/free
    virtual void* read(int a_pos)=0;
    virtual int allocate(void* a_content, const int a_size)=0;
    virtual void free(int a_pos)=0;
};

#endif

