/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2008

    This library contains a class SegmentMainMemory declaration.
    It provides facilities to read/allocate/free a record of any size
    in main memory.
---------------------------------------------------------------------------- */
#ifndef segmmem_defined
#define segmmem_defined

#include "segmem.h"

class SegMMemory: public SegMemory
{
private:
    char*   m_memory;       // memory (an array of bytes)
    int     m_memsize;      // bound of memory
    int     m_exp;          // expansion
    int     m_minallsize;   // min. allocation size
    int     m_freelistheader;

    // segment
    typedef struct
    {
        int m_segsize;  // size of the segment of content
        union
        {
            int m_nextptr;      // next free block (used if this block is free)
            char m_content[1];  // body content
        } m_body;
    } Segment;

    int find(const int a_size);
    void expand(const int a_size);
    void split(Segment* a_seg, const int a_size);

public:
    // constructor/destructor
    SegMMemory(const int a_initsize, const int a_exp, const int a_minsize);
    virtual ~SegMMemory();
    //
    // read/allocate/free
    virtual void* read(int a_pos);
    virtual int allocate(void* a_content, const int a_size);
    virtual void free(int a_pos);
};

#endif

