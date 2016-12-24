/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class Object declaration.
    It consists of object information.
---------------------------------------------------------------------------- */
#ifndef object_defined
#define object_defined

class Object
{
public:
    const int   m_objID;        // object id
    const int   m_objType;      // object type
public:
    Object(const int a_objID, const int a_objType):
      m_objID(a_objID), m_objType(a_objType) {};
    virtual ~Object() {};
};

#endif
