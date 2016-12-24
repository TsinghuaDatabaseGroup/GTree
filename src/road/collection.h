/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2007

    This library contains a number of collection classes:
    * they all store (void*) that point to objects
    * user of these classes needs to provide data types compatible to void*, or
      type cast them (e.g. (void*)a)
---------------------------------------------------------------------------- */

#ifndef COLLECTION_DEFINED
#define COLLECTION_DEFINED

#define INITSIZE    10      // default collection size
#define EXTENSION   10      // default extension size if the collection object overflows

namespace Collection
{
    // class declarations
    class Array;                // an array of items
    class Stack;                // LIFO
    class Queue;                // FIFO
    class Hash;                 // hash (key,value) pairs
    class HashReader;           // hash reader
    class Set;                  // set (extending hash)
    class BinHeap;              // binary heap

    //-------------------------------------------------------------------------
    // class Array
    // functions
    // 1. a dynamic memory allocated array upon request
    // 2. support
    //    reverse the array content
    //    partially sort the array
    //    fully sort the array
    //
    class Array
    {
    // data members
    protected:
        void**  m_p;    // pointer to an array of void*
        int     m_lim;  // the array size (max no. of items)
        int     m_max;  // the max current usage of array space
        int     m_ext;  // the extension space if the array is overflow
    // methods
    public:
        // constructor/destructor
        Array(const int a_max=INITSIZE,const int a_ext=EXTENSION);  // ctor
        Array(const Array& a_v);                                    // cp ctor
        virtual ~Array();                                           // dtor
        //
        // update
        virtual int append(void* a_p);
        virtual int remove(void* a_p);
        virtual int removeAt(int i);
        virtual int replaceAt(const int a_i, void* a_p);
        virtual int clean();
        virtual int partialsort(
            const int a_start, const int a_end,
            int (*a_compare)(const void* a_p0, const void* a_p1)=0);
        virtual int sort(
            int (*a_compare)(const void* a_p0, const void* a_p1)=0);
        virtual int reverse();
        virtual int trim(const int _k);
        virtual int copy(const Array& a_array);
        virtual void removeDuplicate(
            int (*a_compare)(const void* a_p0, const void* a_p1)=0);
        //
        // search
        virtual int size() const;
        virtual void* get(const int a_i) const;
        virtual void* operator[](const int a_i) const;
        virtual int find(
            void* a_p,
            int (*a_compare)(const void* a_p0, const void* a_p1)=0) const;
        virtual int binSearch(  // this assumes the array is sorted!!!
            void* a_p,
            int (*a_compare)(const void* a_p0, const void* a_p1)=0) const;
        //
        // validate
        virtual bool operator==(const Array& a_array) const;
    };
    //-------------------------------------------------------------------------


    //-------------------------------------------------------------------------
    //
    // class Stack
    // LIFO data structure
    //
    class Stack: protected Array
    {
    public:
        // constructor/destructor
        Stack(const int a_max=INITSIZE,const int a_ext=EXTENSION);  // ctor
        Stack(const Stack& a_s);                                    // cp ctor
        virtual ~Stack();                                           // dtor
        //
        // update
        virtual int push(void* a_p);
        virtual void* pop();
        //
        // search
        virtual int height() const;
        virtual void* top() const;
        virtual bool isEmpty() const;
        virtual int size() const;
    };
    //-------------------------------------------------------------------------


    //-------------------------------------------------------------------------
    //
    // class Queue
    // FIFO data structure
    //
    class Queue: protected Array
    {
    // data member
    protected:
        int     m_head;     // head position
    // methods
    public:
        // constructor/destructor
        Queue(const int a_max=INITSIZE,const int a_ext=EXTENSION);  // ctor
        Queue(const Queue& a_q);                                    // cp ctor
        virtual ~Queue();                                           // dtor
        //
        // update
        virtual int enqueue(void* a_p);
        virtual void* dequeue();
        //
        // search
        virtual int length() const;
        virtual void* head() const;
        virtual bool isEmpty() const;
    };
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    //
    // class Hash
    // key-value pairs data structure
    //
    class Hash
    {
        friend class HashReader;

        class HashEntry
        {
        public:
            const int   m_key;
            void*       m_p;
        public:
            HashEntry(const int a_key, void* a_p):
              m_key(a_key), m_p(a_p) {};
            virtual ~HashEntry() {};
            static int comparekey(const void* a_p0, const void* a_p1)
            {
                HashEntry* p0 = *(HashEntry**)a_p0;
                HashEntry* p1 = *(HashEntry**)a_p1;
                //if (p0->m_key < p1->m_key) return -1;
                //if (p0->m_key > p1->m_key) return +1;
                //return 0;
                if (p0->m_key == p1->m_key) return 0;
                return -1;
            };
        };
    // data member
    protected:
        const int   m_slot;
        Array**     m_a;
        int         m_size;
    // methods
    public:
        // constructor/destructor
        Hash(const int a_max=INITSIZE);     // ctor
        Hash(const Hash& a_h);              // cp ctor
        virtual ~Hash();                    // dtor
        //
        // update
        virtual int put(const int a_key, void* a_p);
        virtual int remove(const int a_key);
        virtual int replace(const int a_key, void* a_p);
        virtual int clean();
        //
        // search
        virtual int size() const;
        virtual void* get(const int) const;
    };
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    //
    // class HashReader
    // an utility to read hash iteratively
    //
    class HashReader
    {
    // data members
    protected:
        const Hash& m_hash;
        int         m_curarray;
        int         m_curcontent;
    // methods
    public:
        // constructor/destructor
        HashReader(const Hash& a_hash);
        virtual ~HashReader();
        //
        // update
        void next();
        void first();
        //
        // search
        bool isEnd() const;
        int getKey() const;
        void* getVal() const;    
    };
    //-------------------------------------------------------------------------


    //-------------------------------------------------------------------------
    //
    // class Set
    // set data structure based on Hash
    //
    class Set: protected Hash
    {
    public:
        // constructor/destructor
        Set(const int a_max=INITSIZE);
        virtual ~Set();
        //
        // update
        virtual int insert(void* a_p);
        virtual int remove(void* a_p);
        //
        // search
        virtual int size() const;
        virtual bool in(void*) const;
    };
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    //
    // class BinHeap
    // a binary heap to organize data in binary search tree (BST)
    //
    class BinHeap
    {
        friend class BinHeapReader;

        class BinHeapEntry
        {
        // data members
        public:
            void*           m_p;
            BinHeapEntry*   m_left;     // left branch
            BinHeapEntry*   m_right;    // right branch
        // methods
        public:
            // constructor/destructor
            BinHeapEntry(void* a_p): m_p(a_p), m_left(0), m_right(0) {};
            virtual ~BinHeapEntry() {};
        };

    // data member
    protected:
        int             (*m_compare)(const void*,const void*);
        BinHeapEntry*   m_root;     // root of the BST
        Queue           m_reuse;    // queue of BinHeapEntry for mem reuse
        int             m_count;    // count of tree entries
    // methods
    public:
        // constructor/destructor
        BinHeap(int (*a_compare)(const void*,const void*));
        virtual ~BinHeap();
        //
        // update
        virtual void insert(void* a_p);
        virtual void* removeTop();
        virtual void clean();
        //
        // search
        virtual void* top() const;
        virtual int size() const;
        virtual bool isEmpty() const;
        virtual bool exist(void* a_p) const;
    };
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    //
    // class BinHeapReader
    // an utility to read binheap iteratively
    //
    class BinHeapReader
    {
    // data members
    protected:
        const BinHeap&  m_heap;
        Stack           m_stack;
    // methods
    public:
        // constructor/destructor
        BinHeapReader(const BinHeap& a_heap);
        virtual ~BinHeapReader();
        //
        // update
        void next();
        void first();
        //
        // search
        bool isEnd() const;
        void* get() const;    
    };
    //-------------------------------------------------------------------------
}

using namespace Collection;

#endif  // COLLECTTION_DEFINED
