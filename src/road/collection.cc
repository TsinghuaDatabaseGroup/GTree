/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2007
---------------------------------------------------------------------------- */
#include "collection.h"
#include <string>
#include <string.h>
#include <stdlib.h>

using namespace Collection;

//-----------------------------------------------------------------------------
// Array
//
// constructor/destructor
Array::Array(const int a_max, const int a_ext):
m_lim(a_max),
m_max(0),
m_ext(a_ext)
{
    m_p = new void*[m_lim];
    memset(m_p,0,sizeof(void*)*m_lim);
}

Array::Array(const Array& a_v):
m_lim(a_v.m_lim),
m_max(a_v.m_max),
m_ext(a_v.m_ext)
{
    m_p = new void*[m_lim];
    memcpy(m_p, a_v.m_p, sizeof(void*)*m_max);
}

Array::~Array()
{
    // dtor
    if (m_lim > 0)
        delete[] m_p;
}

// update
int Array::append(void* a_p)
{
    if (m_max >= m_lim)
    {
        void** tmp = new void*[m_lim + m_ext];
        memcpy(tmp, m_p, sizeof(void*)*m_max);
        delete[] m_p;
        m_lim += m_ext;
        m_p = tmp;
    }
    m_p[m_max++] = a_p;
    return m_max;
}

int Array::remove(void* a_p)
{
    for (int i=0; i<m_max; i++)
        if (m_p[i] == a_p)
            m_p[i] = m_p[--m_max];
    return m_max;
}

int Array::removeAt(int i)
{
    m_p[i] = m_p[--m_max];
    return m_max;
}

int Array::replaceAt(const int a_i, void* a_p)
{
    if (a_i < m_max)
        m_p[a_i] = a_p;
    return m_max;
}

int Array::clean()
{
    memset(m_p,0,sizeof(void*)*m_lim);
    m_max = 0;
    return m_max;
}

int ptrcompare(const void* a0, const void* a1)
{
    int p0 = *(int*)a0;
    int p1 = *(int*)a1;
    if (p0 < p1) return -1;
    if (p0 > p1) return +1;
    return 0;
}

int Array::partialsort(const int a_start, const int a_end,
                       int (*a_compare)(const void* a_p0, const void* a_p1))
{
    if (a_compare != 0)
        qsort(&m_p[a_start], a_end - a_start + 1, sizeof(void*), a_compare);
    else
        qsort(&m_p[a_start], a_end - a_start + 1, sizeof(void*), ptrcompare);
    return m_max;
}

int Array::sort(int (*a_compare)(const void* a_p0, const void* a_p1))
{
    partialsort(0, m_max-1, a_compare);
    return m_max;
}

int Array::reverse()
{
    for (int i=0; i<m_max/2; i++)
    {
        void* tmp = m_p[i];
        m_p[i] = m_p[m_max-i-1];
        m_p[m_max-i-1] = tmp;
    }
    return m_max;
}

int Array::trim(const int a_k)
{
    memset(m_p[a_k], 0, sizeof(void*)*(m_max - a_k));
    m_max = a_k;
    return m_max;
}

int Array::copy(const Array& a_array)
{
    if (m_lim < a_array.m_max)
    {
        delete[] m_p;
        m_p = new void*[a_array.m_max];
        m_lim = a_array.m_max;
    }
    memcpy(m_p, a_array.m_p, sizeof(void*)*a_array.m_max);
    m_max = a_array.m_max;
    m_ext = a_array.m_ext;
    return m_max;
}

void Array::removeDuplicate(int (*a_compare)(const void* a_p0, const void* a_p1))
{
    sort(a_compare);
    int cnt=0;
    for (int i=1; i<m_max; i++)
    {
        if (m_p[i] != m_p[cnt])
            m_p[++cnt] = m_p[i];
    }
    m_max = cnt+1;
}


// search
int Array::size() const
{
    return m_max;
}

void* Array::get(const int a_i) const
{
    return m_p[a_i];
}

void* Array::operator[](const int a_i) const
{
    return m_p[a_i];
}

int Array::find(void* a_p,
                int (*a_compare)(const void* a_p0, const void* a_p1)) const
{
    if (a_compare == 0)
    {
        for (int i=0; i<m_max; i++)
            if (m_p[i] == a_p)
                return i;
    }
    else
    {
        for (int i=0; i<m_max; i++)
            if (a_compare(&m_p[i], &a_p) == 0)
                return i;
    }
    return -1;
}

int Array::binSearch(void* a_p,
                     int (*a_compare)(const void* a_p0, const void* a_p1)) const
{
    int left = 0;
    int right= m_max;
    while (left < right)
    {
        int mid = (left + right)/2;
        int ret = 0;
        if (a_compare == 0) ret = ptrcompare(&m_p[mid], &a_p);
        else                ret = a_compare(&m_p[mid], &a_p);
        if (ret == 0) return mid;
        if (ret < 0)
            left = mid+1;
        else
            right = mid;
    }
    return -1;
}

// validate
bool Array::operator==(const Array& a_array) const
{
    if (m_max != a_array.m_max) return false;
    
    for (int i=0; i<m_max; i++)
        if (m_p[i] != a_array.m_p[i])
            return false;
    return true;
}

//-----------------------------------------------------------------------------
// Stack
//
// constructor/destuctor
Stack::Stack(const int a_max, const int a_ext):
Array(a_max,a_ext)
{}

Stack::Stack(const Stack& a_s):
Array(a_s)
{}

Stack::~Stack()
{}

// update
int Stack::push(void* a_p)
{
    append(a_p);
    return m_max;
}

void* Stack::pop()
{
    return m_p[--m_max];
}

// search
int Stack::height() const
{
    return m_max;
}

void* Stack::top() const
{
    return m_p[m_max-1];
}

bool Stack::isEmpty() const
{
    return m_max == 0;
}

int Stack::size() const
{
    return Array::size();
}


//-----------------------------------------------------------------------------
// Queue
//
// constructor/destuctor
Queue::Queue(const int a_max, const int a_ext):
Array(a_max,a_ext),
m_head(0)
{}

Queue::Queue(const Collection::Queue &a_q):
Array(a_q),
m_head(a_q.m_head)
{}

Queue::~Queue()
{}

int Queue::enqueue(void *a_p)
{
    if (m_max == m_lim && m_head > 0)
    {
        int newmax=0;
        for (int i=m_head; i<m_max; i++)
            m_p[newmax++] = m_p[i];
        m_max = newmax;
        m_head = 0;
    }
    append(a_p);
    return m_max - m_head;
}

void* Queue::dequeue()
{
    if (m_head < m_max)
        return m_p[m_head++];
    return 0;
}

int Queue::length() const
{
    return m_max - m_head;
}

void* Queue::head() const
{
    return m_p[m_head];
}

bool Queue::isEmpty() const
{
    return m_head == m_max;
}

//-----------------------------------------------------------------------------
// Hash
//
// constructor/destuctor
Hash::Hash(const int a_max):
m_slot(a_max),
m_size(0)
{
    m_a = new Array*[m_slot];
    for (int i=0; i<m_slot; i++)
        m_a[i] = new Array;
}

Hash::Hash(const Hash& a_h):
m_slot(a_h.m_slot),
m_size(0)
{
    m_a = new Array*[m_slot];
    for (int i=0; i<m_slot; i++)
        m_a[i] = new Array(*a_h.m_a[i]);
}

Hash::~Hash()
{
    clean();
    for (int i=0; i<m_slot; i++)
        delete m_a[i];
    delete[] m_a;
}

// update
int Hash::put(const int a_key, void* a_p)
{
    HashEntry* e = new HashEntry(a_key,a_p);
    int pos = a_key % m_slot;
    if (m_a[pos]->find(e, HashEntry::comparekey) == -1)
    {
        m_a[pos]->append(e);
        return ++m_size;
    }
    delete e;
    return -1;
}

int Hash::remove(const int a_key)
{
    HashEntry e(a_key,0);
    int pos = a_key % m_slot;
    int found = m_a[pos]->find(&e, HashEntry::comparekey);
    if (found != -1)
    {
        HashEntry* t = (HashEntry*)m_a[pos]->get(found);    // get an old entry
        m_a[pos]->removeAt(found);
        delete t;                       // delete the old entry
        return --m_size;
    }
    return -1;
}

int Hash::replace(const int a_key, void* a_p)
{
    HashEntry* e = new HashEntry(a_key,a_p);
    int pos = a_key % m_slot;
    int found = m_a[pos]->find(e, HashEntry::comparekey);
    if (found != -1)
    {
        HashEntry* t = (HashEntry*)m_a[pos]->get(found);    // get an old entry
        m_a[pos]->replaceAt(found,e);   // replace it with a new one
        delete t;                       // delete the old entry
        return m_size;
    }
    delete e;
    return -1;
}

int Hash::clean()
{
    for (int i=0; i<m_slot; i++)
    {
        for (int j=0; j<m_a[i]->size(); j++)
        {
            HashEntry* e = (HashEntry*)m_a[i]->get(j);
            delete e;
        }
        m_a[i]->clean();
    }
    m_size = 0;
    return m_size;
}

// search
int Hash::size() const
{
    return m_size;
}

void* Hash::get(const int a_key) const
{
    HashEntry e(a_key,0);
    int pos = a_key % m_slot;
    int found = m_a[pos]->find(&e, HashEntry::comparekey);
    if (found != -1 && ((HashEntry*)m_a[pos]->get(found))->m_key == a_key)
        return ((HashEntry*)m_a[pos]->get(found))->m_p;
    return 0;
}

//-----------------------------------------------------------------------------
// HashReader
//
// constructor/destuctor
HashReader::HashReader(const Collection::Hash& a_hash):
m_hash(a_hash),
m_curarray(0),
m_curcontent(0)
{
    first();
}

HashReader::~HashReader()
{}

// update
void HashReader::first()
{
    m_curarray = 0;
    m_curcontent= 0;
    while (m_curarray < m_hash.m_slot && m_hash.m_a[m_curarray]->size() == 0)
        m_curarray++;
}

void HashReader::next()
{
    m_curcontent++;
    if (m_curcontent < m_hash.m_a[m_curarray]->size())
        return;
    m_curcontent = 0;
    m_curarray++;
    while (m_curarray < m_hash.m_slot &&
        m_hash.m_a[m_curarray]->size() == 0)
        m_curarray++;
}

// search
bool HashReader::isEnd() const
{
    return m_curarray == m_hash.m_slot;
}

int HashReader::getKey() const
{
    Hash::HashEntry* e =
        (Hash::HashEntry*)m_hash.m_a[m_curarray]->get(m_curcontent);
    return e->m_key;
}

void* HashReader::getVal() const
{
    Hash::HashEntry* e =
        (Hash::HashEntry*)m_hash.m_a[m_curarray]->get(m_curcontent);
    return e->m_p;
}

//-----------------------------------------------------------------------------
// Set
//
// constructor/destuctor
Set::Set(const int a_max):
Hash(a_max)
{}

Set::~Set()
{}

// update
int Set::insert(void* a_p)
{
    return Hash::put((long)a_p,(void*)1);
}

int Set::remove(void* a_p)
{
    return Hash::remove((long)a_p);
}

// search
int Set::size() const
{
    return Hash::size();
}

bool Set::in(void* a_p) const
{
    int val = (long)Hash::get((long)a_p);
    return val == 1;
}

//-----------------------------------------------------------------------------
// BinHeap
//
BinHeap::BinHeap(int (*_compare)(const void*,const void*)):
m_root(0),
m_compare(_compare),
m_count(0)
{}

BinHeap::~BinHeap()
{
   clean();
}

void BinHeap::insert(void* _p)
{
   BinHeapEntry** r = &m_root;
   while (true)
   {
      if (*r == 0)
      {
         if (m_reuse.isEmpty())
         {
            *r = new BinHeapEntry(_p);
         }
         else
         {
            BinHeapEntry* n = (BinHeapEntry*)m_reuse.head();
            m_reuse.dequeue();
            n->m_p = _p;
            n->m_left = 0;
            n->m_right = 0;
            *r = n;
         }
         m_count++;
         return;
      }
      int ret = m_compare != 0 ?
          m_compare(&(*r)->m_p, &_p) : (long)(*r)->m_p - (long)_p;
      if (ret <= 0)
         r = &(*r)->m_right;
      else
         r = &(*r)->m_left;
   }
}

void* BinHeap::removeTop()
{
   BinHeapEntry* target = 0;
   BinHeapEntry** r = &m_root;
   while (*r != 0)
   {
      if ((*r)->m_left != 0)
      {
         r = &(*r)->m_left;
         continue;
      }
      target = *r;
      *r = (*r)->m_right;
      break;
   }
   if (target == 0)
      return 0;

   m_reuse.enqueue(target);
   m_count--;
   return target->m_p;
}

void BinHeap::clean()
{
    while (true)
    {
        void* p = removeTop();
        if (p == 0) break;
        delete (BinHeapEntry*)p;
    }
    while (!m_reuse.isEmpty())
    {
        delete (BinHeapEntry*)m_reuse.head();
        m_reuse.dequeue();
    }
    m_count=0;
}

void* BinHeap::top() const
{
   const BinHeapEntry* r = m_root;
   while (r != 0)
   {
      if (r->m_left != 0)
      {
         r = r->m_left;
         continue;
      }
      return r->m_p;
   }
   return 0;
}

int BinHeap::size() const
{
    return m_count;
}

bool BinHeap::isEmpty() const
{
   return m_root == 0;
}

bool BinHeap::exist(void* a_p) const
{
   BinHeapEntry* r = m_root;
   while (true)
   {
      if (r == 0) return false;

      int ret = m_compare != 0 ?
          m_compare(r->m_p, &a_p) : (long)r->m_p - (long)a_p;

      if (ret == 0) return true;    // found
      if (ret < 0) r = r->m_right;  // in left branch
      else  r = r->m_left;          // in right branch
   }
   return false;
}

//-----------------------------------------------------------------------------
// BinHeapReader
//
BinHeapReader::BinHeapReader(const BinHeap& a_heap):
m_heap(a_heap)
{
    first();
}

BinHeapReader::~BinHeapReader()
{}

void BinHeapReader::next()
{
    BinHeap::BinHeapEntry* r = (BinHeap::BinHeapEntry*)m_stack.pop();
    r = r->m_right;
    while (r != 0)
    {
        m_stack.push(r);
        r = r->m_left;
    }
}

void BinHeapReader::first()
{
    BinHeap::BinHeapEntry* r = m_heap.m_root;
    while (r != 0)
    {
        m_stack.push(r);
        r = r->m_left;
    }
}

bool BinHeapReader::isEnd() const
{
    return m_stack.isEmpty();
}

void* BinHeapReader::get() const
{
    BinHeap::BinHeapEntry* r = (BinHeap::BinHeapEntry*)m_stack.top();
    return r->m_p;
}
