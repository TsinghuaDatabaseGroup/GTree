/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program performs kNN search on objects.

    Suggested arguments:
    > (prog name) -h hiergraph.idx -o object.dat -q #queries -k k -v
    explanations:
    -h: hiergraph index file (input)
    -o: object file
    -q: number of query
    -k: k (# of NNs)
    -v: turn verbose mode on (default: off)

	MY TEST:(ONLY THESE TWO PARAMS)
	-h: hiergraph index file
	-x: FILE_OBJECT
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
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

struct timeval tv;
long ts, te;
#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_DIFF te - ts;
#define TIME_TICK_PRINT(T) printf("%s RESULT: %ld (0.01MS)\r\n", (#T), te - ts );

#define PAGESIZE 409600000
//#define FILE_QUERY "wa.query.dat"

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-h graph.idx -o object.dat -r distance -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-h: hiergraph index file" << endl;
    cerr << "-o: object file" << endl;
    cerr << "-q: #queries" << endl;
    cerr << "-k: #NNs " << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

void test(){
	}


int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "kNN object search on hierarchical graph" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* hidxflname = Param::read(a_argc, a_argv, "-h", "");
    const char* objflname = Param::read(a_argc, a_argv, "-o", "");
    const char* cnumquery  = Param::read(a_argc, a_argv, "-q", "");
    const char* ck = Param::read(a_argc, a_argv, "-k", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    const char* FILE_OBJECT = Param::read(a_argc, a_argv, "-x", "");
	bool verbose = strcmp(vrbs,"null") != 0;

	
    //-------------------------------------------------------------------------
    // access graph index file
    //-------------------------------------------------------------------------
    cerr << "loading a graph index ... ";
    // SegFMemory segmem(hidxflname, PAGESIZE*10, PAGESIZE, 32, false);
	SegFMemory segmem(hidxflname, PAGESIZE, PAGESIZE, 32, false);
    HierGraph hiergraph(segmem);
    int hiergraphsize = segmem.size();
    cerr << "[DONE]" << endl;


	// --------------------------- begin test -------------------------------

	FILE* fin;
	fin = fopen(FILE_OBJECT, "r");
	int cases;
	fscanf(fin, "%d", &cases);
	cout << "Cases: read:" << cases << endl;

	long long all_time, single_time;
	int all_cases;
	all_time = 0;
	all_cases = 0;
	for (int i = 0; i < cases; i ++){
		// init time
		single_time = 0;
		// read obj set
		int num_obj;
		fscanf(fin, "%d", &num_obj);
		cout << "Numobj:" << num_obj << endl;
		vector<int> objset;
		for (int j = 0; j < num_obj; j ++){
			int obj;
			fscanf(fin, "%d", &obj);
			cout << "Obj:" << obj << endl;
			objset.push_back(obj);
		}

		
		// ------------------------- init obj ------
		NodeMapping nmap;
	    GraphMapping gmap;

		for (int j = 0; j < objset.size(); j++){
			int nodeid, objid;
			nodeid = objset[j];
			objid = j;

			nmap.addObject(nodeid, objid);
			BorderNode* bnode = hiergraph.getBorderNode(nodeid);

	        Array* a = &bnode->m_shortcuttree;
		    while (a->size() > 0)
			{
				ShortcutTreeNode* s = (ShortcutTreeNode*)a->get(0);
			    if (s->m_subnetid == 0) break;
			    gmap.addObject(s->m_subnetid, objid);
			    a = &s->m_child;
			}

		}
		
		
		// read locid and test
		int num_ql, locid, K;

		fscanf(fin, "%d", &K);
		fscanf(fin, "%d", &num_ql);
		cout << "K" << K << "NUM_QL:" << num_ql << endl;

		for (int j = 0; j < num_ql; j ++){
			fscanf(fin, "%d", &locid);
			cout << "locid" << locid << endl;
			Array result;
	        int nodeaccess=0;
		    int edgeaccess=0;
			segmem.m_history.clean();

			// test start!
			TIME_TICK_START
			HierObjectSearch::kNNSearch(hiergraph,nmap,gmap, locid, K ,result,nodeaccess,edgeaccess);
			TIME_TICK_END

			//----------------------------------------------------------------------
			// result clean up
			//----------------------------------------------------------------------
			for (int i=0; i<result.size(); i++)
			{
				ObjectSearchResult* r = (ObjectSearchResult*)result.get(i);
				delete r;
			}

			single_time += TIME_TICK_DIFF
			all_cases ++;

			printf("    Current: %lld\n", single_time / (j + 1));
		}
		all_time += single_time;
		printf("Cases_%d: %lld\n", i, single_time / num_ql);
	}
	printf("Overall: %lld\n", all_time / all_cases);
	fclose(fin);



    //-------------------------------------------------------------------------
    // access object file
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // performance statistics
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // search
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // report
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // all done
    //-------------------------------------------------------------------------
    return 0;
}


