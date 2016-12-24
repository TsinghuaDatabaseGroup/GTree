/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program performs (k)NN search based on distance browsing index.

    Suggested arguments:
    > (prog name) -i graph.idx -o object.dat -q query.dat -x distbrws.idx -k #NNs -v
    explanations:
    -i: graph index file (input)
    -o: object.dat
    -q: query.dat
    -x: distbrws.idx
    -k: #NNs
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */

#include "graph.h"
#include "node.h"
#include "segfmem.h"
#include "param.h"
#include "collection.h"
#include "spqdtree.h"
#include "spqdtreerec.h"
#include "graphsearch.h"
#include "distbrws.h"
#include "distbrwssearch.h"
#include "access.h"
#include "iomeasure.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

#define PAGESIZE    4096
#define INFTY       1e10

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-i graph.idx -o object.dat -q query.dat -x quadtrees.idx -k #NNs -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-i: graph index file" << endl;
    cerr << "-o: object file" << endl;
    cerr << "-q: query file" << endl;
    cerr << "-x: quadtrees" << endl;
    cerr << "-k: #NNs" << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "build distance browsing index" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* idxflname = Param::read(a_argc, a_argv, "-i", "");
    const char* objflname = Param::read(a_argc, a_argv, "-o", "");
    const char* qryflname = Param::read(a_argc, a_argv, "-q", "");
    const char* dbrwsflname= Param::read(a_argc, a_argv, "-x", "");
    const int k = atol(Param::read(a_argc, a_argv, "-k", ""));
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
    // determine the area covered by the graph
    //-------------------------------------------------------------------------
    Array nodeidlist;   // used to collect a sorted node id list
    float min[2], max[2];
    min[0] = min[1] = INFTY;
    max[0] = max[1] = -INFTY;
    for (HashReader r(graph.m_nodes); !r.isEnd(); r.next())
    {
        int nodeid = r.getKey();
        Node* node = (Node*)graph.getNode(nodeid);
        min[0] = min[0] < node->m_x ? min[0] : node->m_x;
        min[1] = min[1] < node->m_y ? min[1] : node->m_y;
        max[0] = max[0] > node->m_x ? max[0] : node->m_x;
        max[1] = max[1] > node->m_y ? max[1] : node->m_y;
        nodeidlist.append((void*)nodeid);
    }
    Point ptl(2,min), ptu(2,max);
    Bound area(ptl, ptu);

    //-------------------------------------------------------------------------
    // load all objects
    //-------------------------------------------------------------------------
    Array nodeobj;
    fstream fobject;
    fobject.open(objflname, ios::in);
    while (true)
    {
        int nodeid;
        int objid;
        fobject >> nodeid;
        if (fobject.eof())
            break;
        fobject >> objid;
        nodeobj.append((void*)nodeid);
    }
    fobject.close();

    //-------------------------------------------------------------------------
    // load the distance browsing index
    //-------------------------------------------------------------------------
    SegFMemory segfmem(dbrwsflname, PAGESIZE*10, PAGESIZE, 32, false);
    DistBrws distbrws(segfmem);

    //-------------------------------------------------------------------------
    // find all query locations as sources
    //-------------------------------------------------------------------------
    struct timeb starttime, endtime;
    float querytime=0;

    cerr << "processing the query ... ";
    fstream fquery;
    fquery.open(qryflname, ios::in);

	while (true)
    {
        int nodeid;
        int qryid;
        fquery >> nodeid;
        if (fquery.eof())
            break;
        fquery >> qryid;

		printf("%d %d\n", nodeid, qryid);

        int nodeaccess=0;
        int edgeaccess=0;
        Array result;

        ftime(&starttime);  // time the the distance browsing query
        DistBrwsSearch::nnSearch(
            graph,
            distbrws, area,
            nodeobj, nodeid, k, 
            result,
            nodeaccess, edgeaccess);
        ftime(&endtime);  // time the the distance browsing query
        querytime +=
            ((endtime.time*1000 + endtime.millitm) -
            (starttime.time*1000 + starttime.millitm)) / 1000.0f;

        for (int i=0; i<result.size(); i++)
        {
            ObjectSearchResult* r = (ObjectSearchResult*)result.get(i);
            delete r;
        }
    }
    fquery.close();
    cerr << "[DONE]" << endl;

	cout << "k:," << k << ",";
	cout << "querytime:," << querytime << endl;

    return 0;
}


