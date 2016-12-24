/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu

    Copyright(c) Ken C. K. Lee 2008

    This library contains a class IOMeasure declaration.
    It is an utility to convert segment access into units of bytes/pages.
---------------------------------------------------------------------------- */
#ifndef iomeasure_defined
#define iomeasure_defined

class Access;

class IOMeasure
{
public:
    static int byte(
        const Access& a_access);
    static int page(
        const Access& a_access, const int a_pagesize);
    static int workingset(
        const Access& a_access, const int a_pagesize);
    static int pagelru(
        const Access& a_access,
        const int a_pagesize, const int a_cachesize);
};

#endif

