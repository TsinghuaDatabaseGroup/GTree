/* ----------------------------------------------------------------------------
   
   
   
   	To evaluate different distance influence to road
	usage 
	-h: hiergraph index file
	-x: FILE_OBJECT.Varient 
	-k 10
   
   ./gdist -h col.t8l4.idx  -x col.vary -k 10
   对应文件格式
   line1: query_vertex_id  number_of_object_n
   line2: near_id  far_id farther_id farthest_id 
   ...
   linen+1 : query_vertex_id  number_of_object_n
   linen+2: near_id  far_id farther_id farthest_id 
   ...
   
   
   
   
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
#include <sstream>
using namespace std;

const int N = 50; //测试的组数（当前文件是100组，100组跑了24个小时没有结束，为了加快速度，测一半）
const int PERCENT_OBJ = 4357; //针对不同的数据集合的0.01C，对COL是4356.6 = 4357
//测试集有100个案例

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
void load_distance(string file_name, int N,
	vector <int> &que_pos,
	vector <vector <int> > &near,
	vector <vector <int> > &far,
	vector <vector <int> > &farther,
	vector <vector <int> > &farthest);

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments for GPtree test road:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-h graph.idx -o object.dat -r distance -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-h: hiergraph index file" << endl;
    cerr << "-o: object file" << endl;
    cerr << "-q: #queries" << endl;
    cerr << "-k: #NNs " << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}


void load_distance(string file_name, int N,
	vector <int> &que_pos,
	vector <vector <int> > &near,
	vector <vector <int> > &far,
	vector <vector <int> > &farther,
	vector <vector <int> > &farthest) {

	int obj_id;  //当前位置vertex id
	int query_num; //多少个query num
	int query_id;  //query的id
	int near_dis = 0, far_dis = 0, farther_dis = 0, farthest_dis = 0;//Save the distance temp varient

	ifstream in(file_name);
	vector<int> empty;

	near.clear();
	far.clear();
	farther.clear();
	farther.clear(); //init 


	for (int i = 0; i < N; i++) {  //得到N组不同的集合，用来返回测试用户

		near.push_back(empty);
		far.push_back(empty);
		farther.push_back(empty);
		farthest.push_back(empty);

		string lt;
		getline(in, lt);
		istringstream lbuf(lt);
		lbuf >> obj_id >> query_num;
		que_pos.push_back(obj_id);
		cout << "OBJid :" << obj_id << "query_NUM :" << query_num << endl;
		for (int j = 0; j < query_num; j++) {
			getline(in, lt);			istringstream lbuf(lt);
			lbuf >> near_dis >> far_dis >> farther_dis >> farthest_dis;

			near[i].push_back(near_dis);
			far[i].push_back(far_dis);
			farther[i].push_back(farther_dis);
			farthest[i].push_back(farthest_dis);
		}
	}
	obj_id = obj_id;
	in.close();
}

void load_distance_diff_size(string file_name, int N,
	vector <int> &que_pos,
	vector <vector <int> > &near,
	vector <vector <int> > &far,
	vector <vector <int> > &farther,
	vector <vector <int> > &farthest) {

	int obj_id;  //当前位置vertex id
	int near_size;
	int far_size;
	int farther_size;
	int farthest_size;
	int query_num; //多少个query num
	int query_id;  //query的id
	int near_dis = 0, far_dis = 0, farther_dis = 0, farthest_dis = 0;//Save the distance temp varient

	ifstream in(file_name);
	vector<int> empty;

	near.clear();
	far.clear();
	farther.clear();
	farther.clear(); //init 


	for (int i = 0; i < N; i++) {  //得到N组不同的集合，用来返回测试用户

		near.push_back(empty);
		far.push_back(empty);
		farther.push_back(empty);
		farthest.push_back(empty);

		in >> obj_id >> near_size >> far_size >> farther_size >> farthest_size;
		cout << "obj_id:" << obj_id << "near_size:" << near_size << "far_size:" << far_size << "farther_size:" << farther_size << "farthest_size:" << farthest_size << endl;
		//getchar();
		que_pos.push_back(obj_id);
		for (int p = 0; p < near_size; p++) {
			in >> near_dis;
			near[i].push_back(near_dis);
		}

		for (int p = 0; p < far_size; p++) {
			in >> far_dis;
			far[i].push_back(far_dis);
		}

		for (int p = 0; p < farther_size; p++) {
			in >> farther_dis;
			farther[i].push_back(farther_dis);
		}

		for (int p = 0; p < farthest_size; p++) {
			in >> farthest_dis; 
			farthest[i].push_back(farthest_dis);
		}
	}
	in.close();
}

void 	run_dist_test(HierGraph &hiergraph,
						SegFMemory &segmem, 
						int &hiergraphsize,
						vector <vector <int> > &obj_dist,
						vector <int> que_pos,
	                    int K,
						string testname)
{


	int cases = N;
	//cout << "Cases: read:" << cases << endl;

	long long all_time, single_time, pre_time, ave_time;
	int all_cases;
	all_time = 0;
	all_cases = 0;
	single_time = 0;
	pre_time = 0;
	long long all_que_time = 0;
	long long all_pre_time = 0;
	int run_times = 0;
	int que_times = 0;
	vector<int> objset;

	for (int i = 0; i < cases; i++) {
		// init time
		single_time = 0;
		// read obj set
		objset.clear();
		int num_obj = obj_dist[i].size(); //获取本组obj数量
	//	cout << "Numobj:" << num_obj << endl;
		//if (num_obj < PERCENT_OBJ) continue; //防止测试的obj数量不能达到总的百分比

	//	for (int m = 0; m < num_obj && m < PERCENT_OBJ; m++) {

		for (int m = 0; m < num_obj; m++) { //去掉了obj数目限制，为了放开距离的影响，实际上引入了数量的影响
			objset.push_back(obj_dist[i][m]);      //生成obj测试集合
		}
		  int loc_id = que_pos[i];
		  cout << "Number of num_obj used to test" << objset.size() << endl;

		// ------------------------- init obj ------
		NodeMapping nmap;
		GraphMapping gmap;

		TIME_TICK_START

			for (int j = 0; j < objset.size(); j++) {

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
		all_pre_time += pre_time;

			// read locid and test
			int num_ql, locid, K;
	//	cout << "Pre Time" << pre_time << endl;
		num_ql = 1;
//		cout << "K:" << K << "\tNUM_QL:" << num_ql << endl;

		for (int j = 0; j < num_ql; j++) {
		//	cout <<"I"<<i<<"\tnum_ql:"<< num_ql<< "j" << j<<  "locid:" << loc_id << endl;
			Array result;
			int nodeaccess = 0;
			int edgeaccess = 0;
			segmem.m_history.clean();

			// test start!
			TIME_TICK_START
				HierObjectSearch::kNNSearch(hiergraph, nmap, gmap, loc_id, K, result, nodeaccess, edgeaccess);
			TIME_TICK_END

				//----------------------------------------------------------------------
				// result clean up
				//----------------------------------------------------------------------
				for (int t = 0; t<result.size(); t++)
				{
					ObjectSearchResult* r = (ObjectSearchResult*)result.get(t);
					delete r;
				}

				single_time = TIME_TICK_DIFF
				cout << "single_time Time" << single_time << endl;
				all_que_time += single_time;
				que_times++;
				all_cases++;

			//printf("    Current: %lld(0.01MS) \n", single_time / (j + 1));
		}

		run_times++;
		//ave_time = single_time / num_ql;
		//printf("Cases_%d:\tPre_time\t%lld\t SigTime:\t %lld\t SinTotal:\t%lld \t Total Time:\t %lld\t AveTotal:\t%lld   (us)\r\n", i, pre_time, ave_time, pre_time + ave_time, pre_time + all_time, (pre_time + all_time) / num_ql);

	}
	//printf("Overall: %lld  (0.01MS)\r\n ", all_time / all_cases);
	cout << testname << "\tK:\t" << K;
	long long ave_pre_time = all_pre_time / run_times;
	long long ave_que_time = all_que_time / que_times;

	printf("Pre_time\t%lld\t QueryTime:\t %lld\tTotal Time:\t%lld (us)\r\n", ave_pre_time , ave_que_time, ave_que_time+ ave_pre_time);
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

	//-------------------------------------------------------------------------
	// Load test data
	//-------------------------------------------------------------------------
	vector <int> que_pos;
	vector <vector <int> >near;
	vector <vector <int> >far;
	vector <vector <int> >farther;
	vector <vector <int> >farthest;

	int K = atoi(ck);
	string file_name = string(FILE_OBJECT);


	load_distance_diff_size(file_name, N, que_pos, near, far, farther, farthest);
	
	cout << "Near size is " << near.size();

	// --------------------------- begin test -------------------------------
	run_dist_test(hiergraph, segmem, hiergraphsize, near, que_pos,K,"near");
	run_dist_test(hiergraph, segmem, hiergraphsize, far, que_pos, K, "far");
	run_dist_test(hiergraph, segmem, hiergraphsize, farther, que_pos, K, "farther");
	run_dist_test(hiergraph, segmem, hiergraphsize, farthest, que_pos, K, "farthest");


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


