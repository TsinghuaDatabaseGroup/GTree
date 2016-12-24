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
/*
Stopwatch for 0.01ms
#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_DIFF te - ts;
#define TIME_TICK_PRINT(T) printf("%s RESULT: %ld (0.01MS)\r\n", (#T), te - ts );
*/
//Stop Watch for us
#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 1000000 + tv.tv_usec ;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 1000000 + tv.tv_usec ;
#define TIME_TICK_DIFF te - ts;
#define TIME_TICK_PRINT(T) printf("%s RESULT: %ld (uS)\r\n", (#T), te - ts );
#define PAGESIZE 409600000
//#define FILE_QUERY "wa.query.dat"

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments for GPtree test road:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-h graph.idx -o object.dat -r -b boundNumber -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-h: hiergraph index file" << endl;
    cerr << "-o: object file" << endl;
    cerr << "-q: #queries" << endl;
    cerr << "-b: #boundNumber" << endl;
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
        const char* objnum_bound = Param::read(a_argc, a_argv, "-b", "");
    const char* cnumquery  = Param::read(a_argc, a_argv, "-q", "");
    const char* ck = Param::read(a_argc, a_argv, "-k", "10");
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
	//fscanf(fin, "%d", &cases);
	cases = 1;
	cout << "Cases: read:" << cases << endl;

	long long all_time, single_time, pre_time, ave_time,num_obj_bound;
	int all_cases;
	all_time = 0;
	all_cases = 0;
	single_time = 0;
	pre_time = 0;
	ave_time = 0;
	num_obj_bound = atoi(objnum_bound);//atoi(density_bound); //用来控制测试密度，直接传递值。
	printf ("Bound is :%d \n", num_obj_bound);
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
			//cout << "Obj:" << obj << endl;
			if( j < num_obj_bound){
				objset.push_back(obj);
			}	
		}

		
		// ------------------------- init obj ------
		NodeMapping nmap;
	    GraphMapping gmap;

		TIME_TICK_START

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

		TIME_TICK_END

		pre_time = TIME_TICK_DIFF
		
		// read locid and test
		int num_ql, locid, K;

		//fscanf(fin, "%d", &K);
		K = atoi(ck);
		fscanf(fin, "%d", &num_ql);
		cout << "K:" << K << "\tNUM_QL:" << num_ql << endl;

		for (int j = 0; j < num_ql; j ++){
			fscanf(fin, "%d", &locid);
			//cout <<"num_ql:"<< num_ql<< "j" << j<<  "locid:" << locid << endl;
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

			//printf("    Current: %lld(0.01MS) \n", single_time / (j + 1));
		}
		all_time += single_time;
		ave_time = single_time / num_ql;
		printf("Cases_%d:\tPre_time\t%lld\t SigTime:\t %lld\t SinTotal:\t%lld \t Total Time:\t %lld\t AveTotal:\t%lld   (us)\r\n", i,pre_time,ave_time, pre_time + ave_time, pre_time + all_time,(pre_time + all_time)/ num_ql);
//		printf("Cases_%d:\tPre_time%%lld\t SigTime:\t %lld  (0.01MS)\t SinTotal:%lld \t AveTotal:%lld\r\n", i, single_time / num_ql);

	}
	printf("Overall: %lld  (0.01MS)\r\n ", all_time / all_cases);
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


