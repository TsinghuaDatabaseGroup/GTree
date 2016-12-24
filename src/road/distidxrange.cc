/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program performs range object search based on distance index.

    Suggested arguments:
    > (prog name) -i graph.idx -d distance.idx -q #queries -r range -v
    explanations:
    -i: graph index file (input)
    -d: distance index file (input)
    -q: number of query
    -r: range
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */

#include "graph.h"
#include "segfmem.h"
#include "param.h"
#include "collection.h"
#include "distidx.h"
#include "distidxsearch.h"
#include "graphsearch.h"
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

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-i graph.idx -d dist.idx -q #queries -r distance -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-i: graph index file" << endl;
    cerr << "-d: distance index file" << endl;
    cerr << "-q: #queries" << endl;
    cerr << "-r: range " << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "range object search on distance index" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* idxflname = Param::read(a_argc, a_argv, "-i", "");
    const char* didxflname= Param::read(a_argc, a_argv, "-d", "");
    const char* cnumquery  = Param::read(a_argc, a_argv, "-q", "");
    const char* crange = Param::read(a_argc, a_argv, "-r", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    bool verbose = strcmp(vrbs,"null") != 0;

    //-------------------------------------------------------------------------
    // access graph index file
    //-------------------------------------------------------------------------
    cerr << "loading a graph index ... ";
    SegFMemory segmem(idxflname, PAGESIZE*10, PAGESIZE, 32, false);
    Graph graph(segmem);
    int graphsize = segmem.size();
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // access distance index
    //-------------------------------------------------------------------------
    cerr << "loading distance index ... ";
    SegFMemory segdmem(didxflname, PAGESIZE*10, PAGESIZE, 32, false);
    DistIndex didx(segdmem);
    int didxsize = segdmem.size();
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // performance statistics
    //-------------------------------------------------------------------------
    float totaltime = 0;
    int resultsize = 0;
    int totallru1=0, totaldlru1=0;
    int totallru10=0, totaldlru10=0;
    int totallru20=0, totaldlru20=0;
    int totallru30=0, totaldlru30=0;
    int totallru40=0, totaldlru40=0;
    int totallru50=0, totaldlru50=0;
    int totalnodeaccess=0;
    int totaledgeaccess=0;

    //-------------------------------------------------------------------------
    // search
    //-------------------------------------------------------------------------
    cerr << "search ... ";
    int nodeaccess=0, edgeaccess=0;
    float diameter = GraphSearch::diameter(graph,0,nodeaccess,edgeaccess);
    int nodecnt = graph.m_nodes.size();
    int numquery = atol(cnumquery);
    float range = (float)atof(crange);

    for (int i=0; i<numquery; i++)
    {
        int src = 0;
        Array result;
        int nodeaccess=0;
        int edgeaccess=0;
        int rescnt=0;
        struct timeb starttime, endtime;
        segmem.m_history.clean();
        segdmem.m_history.clean();

        //----------------------------------------------------------------------
        // find query point
        //----------------------------------------------------------------------
        while ((src = (int)((rand()%1000 / 1000.0f)*nodecnt)) > nodecnt);

        //----------------------------------------------------------------------
        // object search here
        //----------------------------------------------------------------------
        ftime(&starttime);  // time the algorithm
        DistIndexSearch::rangeSearch(
            graph, didx, src, range*diameter,
            result, nodeaccess, edgeaccess);
        ftime(&endtime);
        float qtime = 
            ((endtime.time*1000 + endtime.millitm) -
            (starttime.time*1000 + starttime.millitm)) / 1000.0f;

        //----------------------------------------------------------------------
        // debug use
        //----------------------------------------------------------------------
        if (verbose)
        {
            for (int i=0; i<result.size(); i++)
            {
                ObjectSearchResult* r = (ObjectSearchResult*)result.get(i);
                cerr << r->m_nid << "," << r->m_cost << ":" << r->m_objects.size() << endl;
            }
            cerr << "---- time: " << qtime;
            cerr << " -- result size: " << result.size();
            cerr << " ----" << endl;
        }

        //----------------------------------------------------------------------
        // result clean up
        //----------------------------------------------------------------------
        for (int i=0; i<result.size(); i++)
        {
            ObjectSearchResult* r = (ObjectSearchResult*)result.get(i);
            rescnt += r->m_objects.size();
            delete r;
        }


        //----------------------------------------------------------------------
        // performance evaluation
        //----------------------------------------------------------------------
        int lru1 = IOMeasure::pagelru(segmem.m_history, PAGESIZE, 1);
        int lru10= IOMeasure::pagelru(segmem.m_history, PAGESIZE, 10);
        int lru20= IOMeasure::pagelru(segmem.m_history, PAGESIZE, 20);
        int lru30= IOMeasure::pagelru(segmem.m_history, PAGESIZE, 30);
        int lru40= IOMeasure::pagelru(segmem.m_history, PAGESIZE, 40);
        int lru50= IOMeasure::pagelru(segmem.m_history, PAGESIZE, 50);

        int dlru1 = IOMeasure::pagelru(segdmem.m_history, PAGESIZE, 1);
        int dlru10= IOMeasure::pagelru(segdmem.m_history, PAGESIZE, 10);
        int dlru20= IOMeasure::pagelru(segdmem.m_history, PAGESIZE, 20);
        int dlru30= IOMeasure::pagelru(segdmem.m_history, PAGESIZE, 30);
        int dlru40= IOMeasure::pagelru(segdmem.m_history, PAGESIZE, 40);
        int dlru50= IOMeasure::pagelru(segdmem.m_history, PAGESIZE, 50);

        totaltime += qtime;
        resultsize += rescnt;
        totalnodeaccess += nodeaccess;
        totaledgeaccess += edgeaccess;
        totallru1 += lru1;
        totallru10 += lru10;
        totallru20 += lru20;
        totallru30 += lru30;
        totallru40 += lru40;
        totallru50 += lru50;

        totaldlru1 += dlru1;
        totaldlru10 += dlru10;
        totaldlru20 += dlru20;
        totaldlru30 += dlru30;
        totaldlru40 += dlru40;
        totaldlru50 += dlru50;
    }
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // report
    //-------------------------------------------------------------------------
    cout << "graphsz:," << graphsize;
    cout << ",didxsz:," << didxsize;
    cout << ",#query:," << numquery;
    cout << ",qtime:," << totaltime/numquery;
    cout << ",#res:," << resultsize*1.0/numquery;
    cout << ",node:," << totalnodeaccess*1.0/numquery;
    cout << ",edge:," << totaledgeaccess*1.0/numquery;
    cout << ",lru1:," << totallru1/numquery;
    cout << ",lru10:," << totallru10/numquery;
    cout << ",lru20:," << totallru20/numquery;
    cout << ",lru30:," << totallru30/numquery;
    cout << ",lru40:," << totallru40/numquery;
    cout << ",lru50:," << totallru50/numquery;
    cout << ",dlru1:," << totaldlru1/numquery;
    cout << ",dlru10:," << totaldlru10/numquery;
    cout << ",dlru20:," << totaldlru20/numquery;
    cout << ",dlru30:," << totaldlru30/numquery;
    cout << ",dlru40:," << totaldlru40/numquery;
    cout << ",dlru50:," << totaldlru50/numquery;
    cout << endl;

    //-------------------------------------------------------------------------
    // all done
    //-------------------------------------------------------------------------
    return 0;
}


