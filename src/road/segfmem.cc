/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "segfmem.h"
#include <string.h>

/*
    File structure:
                   freelist
                   |
                   V
    +-------------+------+-----------------------+-----------------------------
    |             |      |                       |
    | Header (    | size | content               | 
    | nodehash,   |      | OR                    |                      ooooooo
    | freelist    |      | nextptr (if this      |
    | memsize)    |      | segment is free)      |
    |             |      |                       |
    +-------------+------+-----------------------+-----------------------------
                  >----------------------------------------------------------->
                  memory size (memsize)

    Note: all addresses should be offset by the header.
*/


#define NIL -1

typedef struct
{
    int m_nodehash;
    int m_freelist;
    int m_memsize;
} Header;

// constructor/destructor
SegFMemory::SegFMemory(const char* a_fname, 
                       const int a_initsize,
                       const int a_exp,
                       const int a_minsize,
                       const bool a_newfile):
m_memsize(a_initsize),
m_exp(a_exp),
m_minsize(a_minsize),
m_freelistheader(0),
m_newfile(a_newfile)
{
    // ------------------------------------------------------------------------
    // allocate buffer to read from file
    // ------------------------------------------------------------------------
    m_buffer = new char[m_buflen = a_minsize];
    if (a_newfile)
    {
        m_memfile = fopen(a_fname, "wb+");
        flush();
    }
    else
    {
        m_memfile = fopen(a_fname, "rb+");
        Header hdr;
        fseek(m_memfile, 0, SEEK_SET);
        fread(&hdr, sizeof(hdr), 1, m_memfile);
        m_header        = hdr.m_nodehash;
        m_freelistheader= hdr.m_freelist;
        m_memsize       = hdr.m_memsize;
    }
}

SegFMemory::~SegFMemory()
{
    // ------------------------------------------------------------------------
    // refresh the header and put it back to memory
    // ------------------------------------------------------------------------
    Header hdr;
    hdr.m_nodehash = m_header;
    hdr.m_freelist = m_freelistheader;
    hdr.m_memsize  = m_memsize;

    fseek(m_memfile, 0, SEEK_SET);
    fwrite((char*)&hdr, sizeof(hdr), 1, m_memfile);
    fclose(m_memfile);

    // ------------------------------------------------------------------------
    // clean the buffer
    // ------------------------------------------------------------------------
    delete[] m_buffer;
}

void SegFMemory::flush()
{
    // ------------------------------------------------------------------------
    // clean the header
    // ------------------------------------------------------------------------
    Header hdr;
    hdr.m_nodehash = m_header = -1;
    hdr.m_freelist = m_freelistheader = sizeof(Header);
    hdr.m_memsize  = m_memsize;

    fseek(m_memfile, 0, SEEK_SET);
    fwrite((char*)&hdr, sizeof(hdr), 1, m_memfile);
    
    // ------------------------------------------------------------------------
    // expand the file up to memsize
    // ------------------------------------------------------------------------
    for (int x=0; x<m_memsize; x+= m_minsize)
        fwrite(m_buffer, m_minsize, 1, m_memfile);

    
    // ------------------------------------------------------------------------
    // create the first segment
    // ------------------------------------------------------------------------
    fseek(m_memfile, sizeof(Header), SEEK_SET);
    int segsize = m_memsize - sizeof(int);  // first segment
    int nextptr = NIL;
    fwrite(&segsize, sizeof(int), 1, m_memfile);
    fwrite(&nextptr, sizeof(int), 1, m_memfile);

    // ------------------------------------------------------------------------
    // adjust the freelist
    // ------------------------------------------------------------------------
    m_freelistheader = sizeof(Header);
}

// read/allocate/free
void* SegFMemory::read(int a_pos)
{
    // ------------------------------------------------------------------------
    // read segment from the file
    // ------------------------------------------------------------------------
    int size=0, p=-1;
    fseek(m_memfile, a_pos - sizeof(int), SEEK_SET);
    fread(&size,sizeof(size),1,m_memfile);

    if ((int)(size+sizeof(int)) > m_buflen)
    {
        delete[] m_buffer;
        m_buflen = size + sizeof(int);
        m_buffer = new char[m_buflen];
    }
    memset(m_buffer,0,m_buflen);
    fread(m_buffer,size,1,m_memfile);

    // ------------------------------------------------------------------------
    // update the access history
    // ------------------------------------------------------------------------
    int start = a_pos - sizeof(int);
    int end   = start + size + sizeof(int);
    m_history.append(start,end);

    // ------------------------------------------------------------------------
    // done!
    // ------------------------------------------------------------------------
    return m_buffer;
}

int SegFMemory::allocate(void* a_content, const int a_size)
{
    // ------------------------------------------------------------------------
    // find a segment to fit the content in
    // ------------------------------------------------------------------------
    const int size = a_size > m_minsize ? a_size : m_minsize;
    int address = find(size);
    if (address == -1)
    {
        expand(m_exp > (int)(size+sizeof(int)) ? m_exp : size+sizeof(int));
        address = find(size);
    }

    // ------------------------------------------------------------------------
    // write the content to the segment
    // ------------------------------------------------------------------------
    fseek(m_memfile, address+sizeof(int), SEEK_SET);
    fwrite((char*)a_content, sizeof(char)*size, 1, m_memfile);

    // ------------------------------------------------------------------------
    // release unsed portion of the segment as a segment
    // ------------------------------------------------------------------------
    split(address, a_size);
    return address + sizeof(int);
}

void SegFMemory::free(int a_pos)
{
    // ------------------------------------------------------------------------
    // release memory for later use
    // ------------------------------------------------------------------------
    int sz = 0;
    fseek(m_memfile, a_pos - sizeof(int), SEEK_SET);
    fread((char*)&sz, sizeof(int),1,m_memfile);
    maintain(a_pos-sizeof(int),sz);
}

// ----------------------------------------------------------------------------
int SegFMemory::find(const int a_size)
{
    // logic: largest block strategy
    // 1. scan the free list for a free block whose block size is the largest
    // 2. start from freelistheader
    // 3. best is the address to the candidate
    // 4. ptrbest is the point to the address to the candidate (used to
    //    maintain free list)
    // 5. return NIL if no good candidate is found
    // 6. return the address and update the freelist
    //
    int smallest = -1;
    int bref= -1;
    int ref = m_freelistheader;
    int best=NIL;
    int bbest=NIL;
    while (ref != NIL)
    {
        int sz = 0;
        fseek(m_memfile, ref, SEEK_SET);
        fread(&sz, sizeof(int), 1, m_memfile);
        if (sz >= a_size && (sz < smallest || smallest == -1))
        {
            smallest = sz;
            best = ref;
            bbest = bref;
        }
        bref = ref;
        fread((char*)&ref, sizeof(int), 1, m_memfile);
        if ((ref != -1 && ref < 0) || ref == bref)
        {
            int debug=10;
        }
    }
    if (best == NIL) return NIL;

    // make the previous free segment points to the next segment
    int nextptr;
    fseek(m_memfile, best+sizeof(int), SEEK_SET);
    fread((char*)&nextptr,sizeof(int),1,m_memfile);
    if (bbest == -1)
    {
        m_freelistheader = nextptr;
    }
    else
    {
        fseek(m_memfile, bbest+sizeof(int), SEEK_SET); 
        fwrite((char*)&nextptr,sizeof(int), 1, m_memfile);
    }
    return best;
}

void SegFMemory::expand(const int a_size)
{
    // logic:
    // 1. expand the memory by allocating a replacement memory
    // 2. initialize the expanded memory as a segment
    // 3. attach the segment to the free list
    // 4. update the memory size
    //

    // expand the file by a_size
    fseek(m_memfile,m_memsize + sizeof(Header),SEEK_SET);
    for (int x=0; x<a_size; x+=m_minsize)
        fwrite(m_buffer, m_minsize, 1, m_memfile);

    //
    fseek(m_memfile,m_memsize + sizeof(Header), SEEK_SET);
    int segsize = a_size - sizeof(int);
    fwrite((char*)&segsize, sizeof(int), 1, m_memfile);
    
    //
    maintain(m_memsize + sizeof(Header),a_size-sizeof(int));

    //
    m_memsize += a_size;
}

void SegFMemory::split(const int a_address, const int a_size)
{
    fseek(m_memfile,a_address,SEEK_SET);
    int segsize=0;
    fread((char*)&segsize, sizeof(int), 1, m_memfile);

    if (segsize - a_size > m_minsize)
    {
        // adjust the segment
        int adjustsize = a_size;
        fseek(m_memfile,a_address, SEEK_SET);
        fwrite((char*)&adjustsize,sizeof(int), 1, m_memfile);

        // spawn a new segment
        int newaddress =
            a_address + a_size + sizeof(int);
        int newsize = segsize - a_size - sizeof(int);
        fseek(m_memfile,newaddress,SEEK_SET);
        fwrite((char*)&newsize, sizeof(int), 1, m_memfile);
        //m_memfile.write((char*)&m_freelistheader, sizeof(int));
        //m_freelistheader = newaddress;
        maintain(newaddress,newsize);
    }
}

void SegFMemory::maintain(const int a_address, const int a_size)
{
    if (m_freelistheader == -1 || m_freelistheader > a_address)
    {
        fseek(m_memfile,a_address + sizeof(int),SEEK_SET);
        fwrite((char*)&m_freelistheader, sizeof(int), 1, m_memfile);
        m_freelistheader = a_address;
        return;
    }

    int pointer = m_freelistheader;
    while (pointer < a_address)
    {
        int size = 0;
        int next = 0;
        fseek(m_memfile,pointer,SEEK_SET);
        fread((char*)&size,sizeof(int),1,m_memfile);
        if (pointer + size + sizeof(int) == a_address)
        {
            size += sizeof(int)+a_size;
            fseek(m_memfile,pointer,SEEK_SET);
            fwrite((char*)&size,sizeof(int), 1, m_memfile);
            return;
        }

        fread((char*)&next,sizeof(int),1,m_memfile);
        if (next == a_address + a_size + sizeof(int))
        {
            fseek(m_memfile,next,SEEK_SET);
            int nextsize=0;
            int nextnext=0;
            fread((char*)&nextsize,sizeof(int),1,m_memfile);
            fread((char*)&nextnext,sizeof(int),1,m_memfile);

            // point to new free block
            fseek(m_memfile,pointer+sizeof(int),SEEK_SET);
            fwrite((char*)&a_address,sizeof(int), 1, m_memfile);
            //
            int newsize = a_size + sizeof(int) + nextsize;
            fseek(m_memfile,a_address,SEEK_SET);
            fwrite((char*)&newsize, sizeof(int), 1, m_memfile);
            fwrite((char*)&nextnext, sizeof(int), 1, m_memfile);
            return;
        }
        if (next == -1 || next > a_address)
        {
            // point to new free block
            fseek(m_memfile,pointer+sizeof(int),SEEK_SET);
            fwrite((char*)&a_address, sizeof(int), 1, m_memfile);
            //
            fseek(m_memfile,a_address+sizeof(int),SEEK_SET);
            fwrite((char*)&next, sizeof(int), 1, m_memfile);
            return;
        }
        pointer = next;
    }
}

int SegFMemory::checkfreespace()
{
    int freesize = 0;
    int ptr = m_freelistheader;
    while (ptr != -1)
    {
        int sz,next;
        fseek(m_memfile,ptr,SEEK_SET);
        fread((char*)&sz, sizeof(int),1,m_memfile);
        fread((char*)&next, sizeof(int),1,m_memfile);

        freesize += sz + sizeof(int);
        ptr = next;
    }
    return freesize;
}

int SegFMemory::size() const
{
    return m_memsize;
}

