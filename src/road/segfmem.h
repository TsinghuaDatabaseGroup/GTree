/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2008

    This library contains a class SegmentFileMemory declaration.
    It provides facility to read/allocate/free a record of any size in a file.
---------------------------------------------------------------------------- */
#ifndef segfmem_defined
#define segfmem_defined

#include <stdio.h>
#include "segmem.h"

class SegFMemory: public SegMemory
{
protected:
    FILE*   m_memfile;          // memory (in file)
    int     m_memsize;          // bound of memory
    int     m_exp;              // expansion
    int     m_minsize;          // min. allocation size
    int     m_freelistheader;   // pointer to the head of free block list
    char*   m_buffer;
    int     m_buflen;
    bool    m_newfile;

    int find(const int a_size);
    void expand(const int a_size);
    void split(const int a_address, const int a_size);
    void maintain(const int a_address, const int a_size);
public:
    // constructor/destructor
    SegFMemory(
        const char* a_fname,
        const int a_initsize, const int a_exp, const int a_minsize,
        bool a_newfile);
    virtual ~SegFMemory();
    //
    // reset the file
    virtual void flush();
    //
    // read/allocate/free
    virtual void* read(int a_pos);
    virtual int allocate(void* a_content, const int a_size);
    virtual void free(int a_pos);
    int checkfreespace();
    //
    virtual int size() const;
};

#endif



