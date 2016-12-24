/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program builds a distance index.

    Suggested arguments:
    > (prog name) -i graph.idx -o object.dat -d dist.idx -v
    explanations:
    -i: graph index file (input)
    -o: object file
    -d: distance index
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */

#include "graph.h"
#include "segfmem.h"
#include "param.h"
#include "collection.h"
#include "distsign.h"
#include "distidx.h"
#include "distidxsearch.h"
#include "graphsearch.h"
#include "access.h"
#include "iomeasure.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

#define PAGESIZE    4096

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-h graph.idx -o object.dat -d dist.idx -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-i: graph index file" << endl;
    cerr << "-o: object file" << endl;
    cerr << "-d: distance index" << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "build distance index" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* idxflname = Param::read(a_argc, a_argv, "-i", "");
    const char* objflname = Param::read(a_argc, a_argv, "-o", "");
    const char* didxflname= Param::read(a_argc, a_argv, "-d", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    bool verbose = strcmp(vrbs,"null") != 0;

    //-------------------------------------------------------------------------
    // access graph index file
    //-------------------------------------------------------------------------
    cerr << "loading a graph index ... ";
    SegFMemory segmem(idxflname, PAGESIZE*10, PAGESIZE, 32, false);
    Graph graph(segmem);
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // access object file and create distance index
    //-------------------------------------------------------------------------
    cerr << "loading objects ... ";
    struct timeb starttime, endtime;
    float idxtime=0;
    int numobj=0;
    int numnodes=0;

    Hash nodedistsign;  // keep distance signatures as an array for each node
    Array nodes;        // keep node ids
    fstream fobj;
    fobj.open(objflname, ios::in);
    ftime(&starttime);  // time the the object index creation

    while (true)
    {
        int nodeid, objid;
        fobj >> nodeid;
        if (fobj.eof())
            break;
        fobj >> objid;

        // find object distance to every node
        segmem.m_history.clean();
        Array toAllNodes(graph.m_nodes.size());
        GraphSearch::spanSearch(graph, nodeid, toAllNodes);
        for (int i=0; i<toAllNodes.size(); i++)
        {
            GraphSearchResult* res = (GraphSearchResult*)toAllNodes.get(i);
            int nid = res->m_nid;
            int prev= (long)res->m_path.get(0);
            float cost = res->m_cost;

            Array* a = (Array*)nodedistsign.get(nid);
            if (a == 0)
            {
                nodedistsign.put(nid, a = new Array);
                nodes.append((void*)nid);
            }
            a->append(new DistSignature(objid, cost, prev));
            delete res; // clean up
        }
        toAllNodes.clean();

        numobj++;
    }
    ftime(&endtime);  // time the the object index creation
    idxtime =
        ((endtime.time*1000 + endtime.millitm) -
        (starttime.time*1000 + starttime.millitm)) / 1000.0f;
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // write distance index to memory
    //-------------------------------------------------------------------------
    SegFMemory segfmem(didxflname, PAGESIZE*10, PAGESIZE, 32, true);
    DistIndex didx(segfmem);
    for (int i=0; i<nodes.size(); i++)
    {
        int nid = (long)nodes.get(i);
        Array* a = (Array*)nodedistsign.get(nid);
        didx.writeNode(nid, *a);

        delete a;
    }

    //-------------------------------------------------------------------------
    // clean up
    //-------------------------------------------------------------------------
    nodedistsign.clean();
    nodes.clean();

    cout << "#object:," << numobj;
    cout << ",idxtime:," << idxtime << endl;

    return 0;
}


