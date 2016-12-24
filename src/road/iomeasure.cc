/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "iomeasure.h"
#include "access.h"
#include "collection.h"

int IOMeasure::byte(const Access& a_access)
{
    int byte = 0;
    for (int i=0; i<a_access.length(); i++)
    {
        int start, end;
        a_access.get(i, start, end);
        byte += end - start;
    }
    return byte;
}

int IOMeasure::page(const Access& a_access, const int a_pagesize)
{
    Set pages;
    int page = 0;
    for (int i=0; i<a_access.length(); i++)
    {
        int start, end;
        a_access.get(i, start, end);
        page += (end / a_pagesize) - (start / a_pagesize) + 1;
    }
    return page;
}

int IOMeasure::workingset(const Access& a_access, const int a_pagesize)
{
    Set pages;
    for (int i=0; i<a_access.length(); i++)
    {
        int start, end;
        a_access.get(i, start, end);
        int startpage = start/a_pagesize;
        int endpage = end/a_pagesize;
        for (int j=startpage; j<=endpage; j++)
            if (!pages.in((void*)j))
                pages.insert((void*)j);
    }
    return pages.size();
}

int IOMeasure::pagelru(const Access& a_access,
                       const int a_pagesize,
                       const int a_cachesize)
{
    class carrier
    {
    public:
        const int m_pageid;
        const int m_accesstime;
    public:
        carrier(const int a_p, const int a_t):
          m_pageid(a_p), m_accesstime(a_t) {};
        ~carrier() {};
    };

    int page = 0;
    Hash cache;
    Queue accessq;
    int timer = 1;
    for (int i=0; i<a_access.length(); i++)
    {
        int start, end;
        a_access.get(i, start, end);
        int spage = start / a_pagesize;
        int epage = end / a_pagesize;
        for (int e=spage; e<=epage; e++)
        {
            accessq.enqueue(new carrier(e, timer));
            int latest = (long)cache.get(e);
            if (latest == 0)
            {
                cache.put(e, (void*)timer);
                while (cache.size() > a_cachesize)
                {
                    if (accessq.isEmpty())
                        break;
                    carrier* c = (carrier*)accessq.dequeue();
                    int latest = (long)cache.get(c->m_pageid);
                    if (c->m_accesstime == latest)
                        cache.remove(c->m_pageid);
                    delete c;
                }
                page++;
            }
            else
            {
                cache.replace(e, (void*)timer);
            }
        }
        timer++;
    }

    while (!accessq.isEmpty())
        delete (carrier*)accessq.dequeue();
    cache.clean();
    return page;
}
