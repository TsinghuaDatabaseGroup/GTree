/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program builds distance browsing index.

    Suggested arguments:
    > (prog name) -i graph.idx -o object.dat -s query.dat -x distbrws.idx -v
    explanations:
    -i: graph index file (input)
    -o: object.dat
    -s: query.dat
    -x: distbrws.idx
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
#include "access.h"
#include "iomeasure.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

#define PAGESIZE    4096
#define INFTY       1e10

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-i graph.idx -x quadtrees.idx -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-i: graph index file" << endl;
    cerr << "-x: quadtrees" << endl;
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
    const char* dbrwsflname= Param::read(a_argc, a_argv, "-x", "");
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

    float idxtime=0;	// overall index construction time
	float pathtime=0;	// time for searching paths to all other nodes
	float quadtime=0;	// time to construct shortest path quad tree
	float iotime=0;		// i/o time (writing to disk)

	struct timeb starttime, endtime;	// overall time

    SegFMemory segfmem(dbrwsflname, PAGESIZE*10, PAGESIZE, 32, true);
    DistBrws distbrws(segfmem);

	int cnt = 0;
    cerr << "create quadtrees ... ";
    ftime(&starttime);  // time the distance browsing index creation
    for (HashReader r(graph.m_nodes); !r.isEnd() ; r.next() ) // && cnt < 10; r.next())
    {
		cnt++;
cerr << "1";
        int src = (int)r.getKey();
        SPQuadtree* tree = new SPQuadtree(src, area);

        // --------------------------------------------------------------------
        // forming the spanning tree to all other nodes
        // --------------------------------------------------------------------
        Array toAllNodes(graph.m_nodes.size());

	    struct timeb startpathtime, endpathtime;
		ftime(&startpathtime);
		//
        GraphSearch::diffuseSearch(graph, src, toAllNodes);
		//
		ftime(&endpathtime);
		pathtime +=
			((endpathtime.time*1000 + endpathtime.millitm) -
			(startpathtime.time*1000 + startpathtime.millitm)) / 1000.0f;

cerr << "2";
		struct timeb startquadtime, endquadtime;
		ftime(&startquadtime);
		//
        for (int i=0; i<toAllNodes.size(); i++)
        {
            // ----------------------------------------------------------------
            // get the destination nodes from the spanning tree
            // ----------------------------------------------------------------
            GraphSearchResult* res = (GraphSearchResult*)toAllNodes.get(i);
            int destid = res->m_nid;
            if (destid != src)
            {
                int firstnode = (long)res->m_path.get(0);
                float cost = res->m_cost;

                // ----------------------------------------------------------------
                // add the path info to the shortest path quadtree
                // ----------------------------------------------------------------
                Node* node = graph.getNode(destid);
                float f[2];
                f[0] = node->m_x; f[1] = node->m_y;
                Point pt(2, f);
                SPQuadtreeRec* rec = new SPQuadtreeRec(destid, pt, firstnode, cost);
                tree->addObject(rec);
            }

            // ----------------------------------------------------------------
            // clean up
            // ----------------------------------------------------------------
            delete res;
        }
        toAllNodes.clean();
		//
		ftime(&endquadtime);
		quadtime +=
			((endquadtime.time*1000 + endquadtime.millitm) -
			(startquadtime.time*1000 + startquadtime.millitm)) / 1000.0f;

cerr << "3";
		//----------------------------------------------------------------------
		// write distance index to memory
		//----------------------------------------------------------------------
		struct timeb startiotime, endiotime;
		ftime(&startiotime);
		//
        distbrws.writeNode(src, tree);
		//
		ftime(&endiotime);
		iotime +=
			((endiotime.time*1000 + endiotime.millitm) -
			(startiotime.time*1000 + startiotime.millitm)) / 1000.0f;
cerr << ".";
        delete tree;
    }
    ftime(&endtime);  // time the the quadtree index creation
    idxtime =
        ((endtime.time*1000 + endtime.millitm) -
        (starttime.time*1000 + starttime.millitm)) / 1000.0f;
    cerr << "[DONE]" << endl;

    /*
	float loadtime=0;	// overall index construction time
	float checktime=0;
	cnt = 0;
	//-------------------------------------------------------------------------
	// testing
	//-------------------------------------------------------------------------
    cerr << "testing ... ";
	int nodesize = 0;
    for (HashReader r(graph.m_nodes); !r.isEnd(); r.next() )//  && cnt < 10; r.next())
    {
cerr << "1";
		cnt++;
        int src = (int)r.getKey();

        // --------------------------------------------------------------------
        // loading the spanning tree
        // --------------------------------------------------------------------
		struct timeb startloadtime, endloadtime;
		ftime(&startloadtime);
		//
		SPQuadtree* tree = distbrws.getNode(src, area);
		//
		ftime(&endloadtime);
		loadtime +=
			((endloadtime.time*1000 + endloadtime.millitm) -
			(startloadtime.time*1000 + startloadtime.millitm)) / 1000.0f;
		nodesize += tree->size();

        // --------------------------------------------------------------------
        // forming the spanning tree to all other nodes
        // --------------------------------------------------------------------
        Array toAllNodes(graph.m_nodes.size());
        GraphSearch::diffuseSearch(graph, src, toAllNodes);

		float tmpchecktime = 0;
        for (int i=0; i<toAllNodes.size(); i++)
        {
            // ----------------------------------------------------------------
            // get the destination nodes from the spanning tree
            // ----------------------------------------------------------------
            GraphSearchResult* res = (GraphSearchResult*)toAllNodes.get(i);
            int destid = res->m_nid;
            if (destid != src)
            {
                int firstnode = (int)res->m_path.get(0);
                float cost = res->m_cost;

                // ----------------------------------------------------------------
                // add the path info to the shortest path quadtree
                // ----------------------------------------------------------------
                Node* node = graph.getNode(destid);
                float f[2];
                f[0] = node->m_x; f[1] = node->m_y;
                Point pt(2, f);

				struct timeb startchecktime, endchecktime;
				ftime(&startchecktime);
				//
				int nextnodeid = tree->next(pt);
				if (nextnodeid != firstnode)
				{
					cerr << "src: " << src << " dest: " << destid;
					cerr << " exp neighbor: " << nextnodeid;
					cerr << " res neighbour: " << firstnode << endl;
				}
				//
				ftime(&endchecktime);
				tmpchecktime +=
					((endchecktime.time*1000 + endchecktime.millitm) -
					(startchecktime.time*1000 + startchecktime.millitm)) / 1000.0f;
            }

            // ----------------------------------------------------------------
            // clean up
            // ----------------------------------------------------------------
            delete res;
        }
		checktime += tmpchecktime / toAllNodes.size();
        toAllNodes.clean();
        delete tree;
	}

    cerr << "[DONE]" << endl;
	*/

	cout << "#totalnode:," << graph.m_nodes.size() << ",";
	cout << "#testnode:,10,"; 
	cout << "pathtime:," << pathtime << ",";
	cout << "quadtime:," << quadtime << ",";
	cout << "iotime:," << iotime << ",";
	cout << "idxtime:," << idxtime << ",";
//	cout << "loadtime/node:," << loadtime/10 << ",";
//	cout << "checktime/node:," << checktime/10 << ",";
//	cout << "totalnodesz/node:," << nodesize/10 << endl;

    return 0;
}
