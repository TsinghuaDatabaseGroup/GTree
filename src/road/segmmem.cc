/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "segmmem.h"
#include <string.h>

#define NIL -1

SegMMemory::SegMMemory(const int a_initsize,
                       const int a_exp,
                       const int a_minsize):
m_memory(new char[a_initsize]),
m_memsize(a_initsize),
m_exp(a_exp),
m_minallsize(a_minsize),
m_freelistheader(0)
{
    memset(m_memory,0,sizeof(char)*a_initsize);
    Segment* start = (Segment*)&m_memory[0];
    start->m_segsize = a_initsize - sizeof(start->m_segsize);
    start->m_body.m_nextptr = NIL;
}

SegMMemory::~SegMMemory()
{
    delete[] m_memory;
}

void* SegMMemory::read(int a_pos)
{
    int start = a_pos - sizeof(int);        // get start address;
    int size  = *(int*)&m_memory[start];
    int end   = start + size + sizeof(int); // get end address;
    //m_history.append(start,end);            // added to access history

    return (void*)&m_memory[a_pos];
}

int SegMMemory::allocate(void* a_content, const int a_size)
{
    const int size = a_size > m_minallsize ? a_size : m_minallsize;
    int address = find(size);
    if (address == -1)
    {
        expand(m_exp > size+sizeof(Segment) ? m_exp : size+sizeof(Segment));
        address = find(size);
    }
    Segment* seg = (Segment*)&m_memory[address];
    split(seg, size);
    seg->m_segsize = size;
    memcpy(seg->m_body.m_content, a_content, size);
    return (long)seg->m_body.m_content - (long)m_memory;
}

void SegMMemory::free(int a_pos)
{
    Segment* seg = (Segment*)&m_memory[a_pos - sizeof(int)];
    seg->m_body.m_nextptr = m_freelistheader;
    m_freelistheader = (long)seg-(long)m_memory;
}

// ----------------------------------------------------------------------------
int SegMMemory::find(const int a_size)
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
    int largest = a_size;
    int* ref = &m_freelistheader;
    int best = NIL;
    int* ptrbest = 0;
    while (*ref != NIL)
    {
        Segment* seg = (Segment*)&m_memory[*ref];
        if (seg->m_segsize >= largest)
        {
            largest = seg->m_segsize;
            best = *ref;
            ptrbest = ref;
        }
        ref = &seg->m_body.m_nextptr;
    }
    if (best == NIL) return NIL;

    Segment* seg = (Segment*)&m_memory[best];
    *ptrbest = seg->m_body.m_nextptr;
    return best;
}

void SegMMemory::expand(const int a_size)
{
    // logic:
    // 1. expand the memory by allocating a replacement memory
    // 2. initialize the expanded memory as a segment
    // 3. attach the segment to the free list
    // 4. update the memory size
    //

    char* m_newmem = new char[m_memsize + a_size];
    memcpy(m_newmem, m_memory, m_memsize);
    delete m_memory;
    m_memory = m_newmem;
    //
    Segment* seg = (Segment*)&m_memory[m_memsize];
    seg->m_segsize = a_size - sizeof(seg->m_segsize);
    seg->m_body.m_nextptr = m_freelistheader;
    //
    m_freelistheader = m_memsize;
    //
    m_memsize += a_size;
}

void SegMMemory::split(Segment* a_seg, const int a_size)
{
    int diff = a_seg->m_segsize - a_size;
    if (diff > m_minallsize)
    {
        int address =
            (long)a_seg - (long)m_memory +
            sizeof(a_seg->m_segsize) + a_size;
        Segment* seg = (Segment*)&m_memory[address];
        seg->m_segsize = a_seg->m_segsize - sizeof(a_seg->m_segsize) - a_size;
        seg->m_body.m_nextptr = m_freelistheader;
        m_freelistheader = address;
    }
}
