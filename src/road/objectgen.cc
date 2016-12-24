/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "objectgen.h"
#include "node.h"
#include <math.h>
#include <stdlib.h>

void ObjectGen::uniform(const int a_numobj, Array& a_allnodes, Array& a_map)
{
    int numnode = a_allnodes.size();
    int division = numnode / (a_numobj+3);
    for (int i=0; i<a_numobj; i++)
    {
        int j = ((i+1) * division) % numnode;
        Node* node = (Node*)a_allnodes.get(j);
        a_map.append(new NodeObject(node->m_id, i));
    }
}

void ObjectGen::cluster(const int a_numcluster, const int a_numobj,
                        Array& a_allnodes, Array& a_map)
{
    int numnode = a_allnodes.size();
    int* centroid = new int[a_numcluster];
    for (int c=0; c<a_numcluster; c++)
    {
        int j = (int)ceil((rand()%1000)/1000.0f * numnode);
        Node* node = (Node*)a_allnodes.get(j);
        centroid[c] = node->m_id;
    }

    for (int i=0; i<a_numobj; i++)
    {
        int j = rand() % a_numcluster;
        a_map.append(new NodeObject(centroid[j],i));
    }

    delete[] centroid;
}
