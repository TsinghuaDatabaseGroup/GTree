/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program performs range search on objects based on hierarchical graph.

    Suggested arguments:
    > (prog name) -h hiergraph.idx -o object.dat -q #queries -s #sources -r range -v
    explanations:
    -h: hiergraph index file (input)
    -o: object file
    -q: number of queries
    -s: number of sources
    -r: range
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */

#include "hiergraph.h"
#include "bordernode.h"
#include "shortcuttreenode.h"
#include "segfmem.h"
#include "param.h"
#include "collection.h"
#include "nodemap.h"
#include "graphmap.h"
#include "graphsearch.h"
#include "hierobjsearch.h"
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
    cerr << "-h graph.idx -o object.dat -r distance -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-h: hiergraph index file" << endl;
    cerr << "-o: object file" << endl;
    cerr << "-q: #queries" << endl;
    cerr << "-s: #sources" << endl;
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

    cerr << "group range object search on hierarchical graph" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* hidxflname = Param::read(a_argc, a_argv, "-h", "");
    const char* objflname = Param::read(a_argc, a_argv, "-o", "");
    const char* cnumquery  = Param::read(a_argc, a_argv, "-q", "");
    const char* cnumsource = Param::read(a_argc, a_argv, "-s", "");
    const char* crange = Param::read(a_argc, a_argv, "-r", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    bool verbose = strcmp(vrbs,"null") != 0;

    //-------------------------------------------------------------------------
    // access graph index file
    //-------------------------------------------------------------------------
    cerr << "loading a graph index ... ";
    SegFMemory segmem(hidxflname, PAGESIZE*10, PAGESIZE, 32, false);
    HierGraph hiergraph(segmem);
    int hiergraphsize = segmem.size();
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // access object file
    //-------------------------------------------------------------------------
    cerr << "loading objects ... ";
    struct timeb starttime, endtime;
    float idxtime=0;

    int objsize = 0;
    NodeMapping nmap;
    GraphMapping gmap;
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
        nmap.addObject(nodeid, objid);
        objsize += sizeof(int)*2;

        BorderNode* bnode = hiergraph.getBorderNode(nodeid);

        Array* a = &bnode->m_shortcuttree;
        while (a->size() > 0)
        {
            ShortcutTreeNode* s = (ShortcutTreeNode*)a->get(0);
            if (s->m_subnetid == 0) break;
            gmap.addObject(s->m_subnetid, objid);
            a = &s->m_child;
            objsize += sizeof(int)*2;
        }
    }
    ftime(&endtime);  // time the the object index creation
    idxtime =
        ((endtime.time*1000 + endtime.millitm) -
        (starttime.time*1000 + starttime.millitm)) / 1000.0f;
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // performance statistics
    //-------------------------------------------------------------------------
    float totaltime = 0;
    int resultsize = 0;
    int totallru1=0;
    int totallru10=0;
    int totallru20=0;
    int totallru30=0;
    int totallru40=0;
    int totallru50=0;
    int totalnodeaccess=0;
    int totaledgeaccess=0;

    //-------------------------------------------------------------------------
    // search
    //-------------------------------------------------------------------------
    cerr << "search ... ";
    int nodeaccess=0, edgeaccess=0;
    float diameter = GraphSearch::diameter(hiergraph,0,nodeaccess,edgeaccess);
    int nodecnt = hiergraph.m_nodes.size();
    int numquery = atol(cnumquery);
    int numsrc = atol(cnumsource);
    float range = (float)atof(crange);

    for (int i=0; i<numquery; i++)
    {
        Array result;
        int nodeaccess=0;
        int edgeaccess=0;
        int rescnt=0;
        struct timeb starttime, endtime;
        segmem.m_history.clean();

        //----------------------------------------------------------------------
        // find query point
        //----------------------------------------------------------------------
        int src[MAXQUERY];
        float rng[MAXQUERY];
        for (int j=0; j<numsrc; j++)
        {
            while ((src[j] = (int)((rand()%1000 / 1000.0f)*nodecnt)) > nodecnt);
            rng[j] = range * diameter;
        }

        //----------------------------------------------------------------------
        // object search here
        //----------------------------------------------------------------------
        ftime(&starttime);  // time the algorithm
        HierObjectSearch::groupRangeSearch(
            hiergraph, nmap, gmap,
            src, rng,
            numsrc,
            result,nodeaccess,edgeaccess);
        ftime(&endtime);
        float qtime = 
            ((endtime.time*1000 + endtime.millitm) -
            (starttime.time*1000 + starttime.millitm)) / 1000.0f;

        //----------------------------------------------------------------------
        // debug use
        //----------------------------------------------------------------------
        if (verbose)
        {
            for (int i=0; i<numsrc; i++)
                cerr << src[i] << ",";

            for (int i=0; i<result.size(); i++)
            {
                GroupObjectSearchResult* r = (GroupObjectSearchResult*)result.get(i);
                cerr << " " << r->m_nid << "," << r->m_oid << ":" << r->sumcost() << endl;
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
            GroupObjectSearchResult* r = (GroupObjectSearchResult*)result.get(i);
            rescnt++;
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
    }
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // report
    //-------------------------------------------------------------------------
    cout << "idxtime:," << idxtime;
    cout << ",higraphsz:," << hiergraphsize;
    cout << ",objsz:," << objsize;
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
    cout << endl;

    //-------------------------------------------------------------------------
    // all done
    //-------------------------------------------------------------------------
    return 0;
}


