/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program
    * performs graph partitioning and 
    * outputs a hierarchical graph.

    Suggested arguments:
    > (prog name) -n nodefile.txt -e edgefile.txt -t div -l level -h graph.idx -v
    explanations:
    -n: node file
    -e: edge file
    -t: division factor
    -l: the number of hierarchical levels
    -h: hiergraph index file (output)
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */
#include "hiergraph.h"
#include "node.h"
#include "edge.h"
#include "segfmem.h"
#include "segmmem.h"
#include "hiergraph.h"
#include "graphpartition.h"
#include "param.h"
#include "collection.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

#define MAXNODES    100000
#define PAGESIZE    4096        // 4KB per page

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-n nodefile.txt -e edgefile.txt -t div -l level -h hiergraph.idx -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-n: node file" << endl;
    cerr << "    format: nodeID  x y " << endl;
    cerr << "-e: edge file" << endl;
    cerr << "    format: src_nodeID dest_nodeID cost" << endl;
    cerr << "-t: division factor" << endl;
    cerr << "-l: level (number of hierarchical levels)" << endl;
    cerr << "-h: hiergraph index file (output)" << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "hierarchical graph loader" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* nodeflname = Param::read(a_argc, a_argv, "-n", "");
    const char* edgeflname = Param::read(a_argc, a_argv, "-e", "");
    const char* div = Param::read(a_argc, a_argv, "-t", "");
    const char* level = Param::read(a_argc, a_argv, "-l", "");
    const char* idxflname = Param::read(a_argc, a_argv, "-h", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    bool verbose = strcmp(vrbs,"null") != 0;
    fstream fnode, fedge;

    Hash nodes(MAXNODES);

    //-------------------------------------------------------------------------
    // loading nodes and edges
    //-------------------------------------------------------------------------
    cerr << "loading nodes ... ";
    fnode.open(nodeflname, ios::in);
    while (true)
    {
        int id;
        float x,y;
        fnode >> id;
        if (fnode.eof())
            break;

        fnode >> x;
        fnode >> y;

        nodes.put(id, new Node(id, x,y));
    }
    cerr << "[DONE]" << endl;

    cerr << "loading edges ... ";
    fedge.open(edgeflname, ios::in);
    while (true)
    {
        int id;
        int src, dest;
        float cost;
        fedge >> id;
        if (fedge.eof())
            break;
        fedge >> src;
        fedge >> dest;
        fedge >> cost;
        Edge edge2dest(dest, cost);
        
        Node* srcnode = (Node*)nodes.get(src);
        srcnode->addEdge(edge2dest);

		// for directed graph
		// Edge edge2src(src, cost);
        // Node* destnode = (Node*)nodes.get(dest);
        // destnode->addEdge(edge2src);
    }
    cerr << "[DONE]" << endl;

    fnode.close();
    fedge.close();

    //-------------------------------------------------------------------------
    // access graph index file
    //-------------------------------------------------------------------------
    SegFMemory hiergraphmem(idxflname, PAGESIZE*10, PAGESIZE, 24, true);
    HierGraph hiergraph(hiergraphmem);

    //-------------------------------------------------------------------------
    // partition the graph and result in hierarchical graph
    //-------------------------------------------------------------------------
    cerr << "partition a graph ..." << endl;
    struct timeb starttime, endtime;
    Array path;
    float dist=0;
    float totalbuildtime, parttime, shorttime;
    ftime(&starttime);  // time the algorithm
    // graph partitioning
    GraphPartition::geoPartition(nodes, atol(div), atol(level), hiergraph, parttime, shorttime);

    ftime(&endtime);
    totalbuildtime = 
        ((endtime.time*1000 + endtime.millitm) -
        (starttime.time*1000 + starttime.millitm)) / 1000.0f;
    cerr << "[DONE]" << endl;

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    for (HashReader rdr(nodes); !rdr.isEnd(); rdr.next())
        delete (Node*)rdr.getVal();

    cout << "partitiontime:," << parttime;
    cout << ",shortcuttime:," << shorttime;
    cout << ",totalbuildtime:" << totalbuildtime << endl;

    return 0;
}
