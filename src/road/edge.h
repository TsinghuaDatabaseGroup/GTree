/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008

    This file contains a class Edge declaration.
    This presents an edge that 
    * points a neighboring node and
    * carries a cost to that node.
---------------------------------------------------------------------------- */
#ifndef edge_defined
#define edge_defined

class Edge
{
public:
    int     m_neighbor;     // neighbor node id
    float   m_cost;         // cost of the edge towards neighbor
public:
    // constructor/destructor
    Edge(const int m_id, const float a_cost):
      m_neighbor(m_id), m_cost(a_cost) {};
    virtual ~Edge() {};
    //
    // storage size
    int size() const
    {
        return sizeof(m_neighbor)+sizeof(m_cost);
    };
};

#endif

