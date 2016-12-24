// macro for 64 bits file, larger than 2G

#define _FILE_OFFSET_BITS 64



#include<stdio.h>

#include<vector>

#include<stdlib.h>

#include<memory.h>

#include<math.h>

#include<map>

#include<list>

#include<set>

#include<deque>

#include<stack>

#include<algorithm>

#include<sys/time.h>

#include<iostream>

using namespace std;



// MACRO for time tick

struct timeval tv;

long ts, te;

#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;

#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;

#define TIME_TICK_DIFF te - ts;

#define TIME_TICK_PRINT(T) printf("%s RESULT: %ld (0.01MS)\r\n", (#T), te - ts );

// ----------



#define ROUND_FACTOR 0.5

#define MAX_INF  1e100

#define MIN_INF -1e100





char FILE_NODE[128];

char FILE_EDGE[128];

char FILE_MORTON[128];

int WEIGHT_INFLATE_FACTOR;

// bool IS_DIRECTED_GRAPH;

char FILE_OBJECT[128];



typedef struct{

	double x,y;

	vector<int> adjnodes;

	vector<int> adjweight;

}Node;



typedef struct{

	double llx, lly, urx, ury;

	vector<int> children;

	bool isleaf;

	int first_path;

	int level;

	vector<int> leafinvlist; // valid in build, not for use

}QuadTreeNode;



typedef vector<QuadTreeNode> QuadTree;



typedef struct{

	long long z_order;

	int first_path;

}MortonBlock;



typedef struct{

	int max_level;

	vector<MortonBlock> array;

}MortonList;



vector<Node> Nodes;

vector<MortonList> MortonLists;



double min_llx, min_lly;

double max_urx, max_ury;



void init_input(){

	FILE *fin;



	// load node

	printf("LOADING NODE...");

	fin = fopen(FILE_NODE,"r");

	int nid;

	double x,y;

	while( fscanf(fin, "%d %lf %lf", &nid, &x, &y ) == 3 ){

		Node node = { x * WEIGHT_INFLATE_FACTOR, y * WEIGHT_INFLATE_FACTOR };

		Nodes.push_back(node);

	}

	fclose(fin);

	printf("COMPLETE. NODE_COUNT=%d\n", Nodes.size());



	// load edge

	printf("LOADING EDGE...");

	fin = fopen(FILE_EDGE, "r");

	int eid;

	int snid, enid;

	double weight;

	int iweight;

	while( fscanf(fin,"%d %d %d %lf", &eid, &snid, &enid, &weight ) == 4 ){

		iweight = (int) (weight * WEIGHT_INFLATE_FACTOR + ROUND_FACTOR);

		Nodes[snid].adjnodes.push_back( enid );

		Nodes[snid].adjweight.push_back( iweight );

		Nodes[enid].adjnodes.push_back( snid );

		Nodes[enid].adjweight.push_back( iweight );

	}

	fclose(fin);

	printf("COMPLETE.\n");



	// init get whole region

	min_llx = MAX_INF; min_lly = MAX_INF;

	max_urx = MIN_INF; max_ury = MIN_INF;

	for ( int i = 0; i < Nodes.size(); i++ ){

		if ( Nodes[i].x < min_llx ) min_llx = Nodes[i].x;

		if ( Nodes[i].x > max_urx ) max_urx = Nodes[i].x;

		if ( Nodes[i].y < min_lly ) min_lly = Nodes[i].y;

		if ( Nodes[i].y > max_ury ) max_ury = Nodes[i].y;

	}

}





void init(){

	init_input();

}



bool in_rect( double x, double y, double llx, double lly, double urx, double ury ){

	if ( llx <= x && x <= urx && lly <= y && y <= ury ) return true;

	return false;

}



void quadtree_split(int pos, int &dest, int& dest_first_path, QuadTree &qt){

	QuadTreeNode ul, ll, ur, lr;

	// isleaf

	ul.isleaf = true;

	ll.isleaf = true;

	ur.isleaf = true;

	lr.isleaf = true;

	

	// coordinate

	double mid_x = ( qt[pos].llx + qt[pos].urx ) / 2;

	double mid_y = ( qt[pos].lly + qt[pos].ury ) / 2;

	ul.llx = qt[pos].llx; ul.lly = mid_y; ul.urx = mid_x; ul.ury = qt[pos].ury;

	ll.llx = qt[pos].llx; ll.lly = qt[pos].lly; ll.urx = mid_x; ll.ury = mid_y;

	ur.llx = mid_x; ur.lly = mid_y; ur.urx = qt[pos].urx; ur.ury = qt[pos].ury;

	lr.llx = mid_x; lr.lly = qt[pos].lly; lr.urx = qt[pos].urx; lr.ury = mid_y;



	// leafinvlist

	int dest_pos;

	for ( int i = 0; i < qt[pos].leafinvlist.size(); i++ ){

		int vid = qt[pos].leafinvlist[i];

		// ul

		if ( in_rect(Nodes[vid].x, Nodes[vid].y, ul.llx, ul.lly, ul.urx, ul.ury) ){

			ul.leafinvlist.push_back(vid);

			if ( vid == dest ) dest_pos = 0;

		}

		// ur

		else if ( in_rect(Nodes[vid].x, Nodes[vid].y, ur.llx, ur.lly, ur.urx, ur.ury) ){

			ur.leafinvlist.push_back(vid);

			if ( vid == dest ) dest_pos = 1;

		}

		// ll

		else if ( in_rect(Nodes[vid].x, Nodes[vid].y, ll.llx, ll.lly, ll.urx, ll.ury) ){

			ll.leafinvlist.push_back(vid);

			if ( vid == dest ) dest_pos = 2;

		}

		// lr

		else{

			lr.leafinvlist.push_back(vid);

			if ( vid == dest ) dest_pos = 3;

		}

	}

	// first_path

	ul.first_path = ll.first_path = ur.first_path = lr.first_path = -1;

	if ( ul.leafinvlist.size() > 0 ) ul.first_path = qt[pos].first_path;

	if ( ur.leafinvlist.size() > 0 ) ur.first_path = qt[pos].first_path;

	if ( ll.leafinvlist.size() > 0 ) ll.first_path = qt[pos].first_path;

	if ( lr.leafinvlist.size() > 0 ) lr.first_path = qt[pos].first_path;



	// level

	ul.level = ur.level = ll.level = lr.level = qt[pos].level + 1;



	// children

	qt.push_back(ul);

	int ul_pos = qt.size() - 1;

	qt[pos].children.push_back( ul_pos );

	

	qt.push_back(ur);

	int ur_pos = qt.size() - 1;

	qt[pos].children.push_back( ur_pos );



	qt.push_back(ll);

	int ll_pos = qt.size() - 1;

    qt[pos].children.push_back( ll_pos );



	qt.push_back(lr);

	int lr_pos = qt.size() - 1;

	qt[pos].children.push_back( lr_pos );



	// check pure, recursively

	switch(dest_pos){

		case 0:

			if (ul.leafinvlist.size() != 1 ){

				quadtree_split(	ul_pos, dest, dest_first_path, qt );

			}

			else{

				qt[ul_pos].first_path = dest_first_path;

			}	

			break;

		case 1:

			if ( ur.leafinvlist.size() != 1 ){

				quadtree_split( ur_pos, dest, dest_first_path, qt );

			}

			else{

				qt[ur_pos].first_path = dest_first_path;

			}

			break;

		case 2:

			if ( ll.leafinvlist.size() != 1 ){

				quadtree_split( ll_pos, dest, dest_first_path, qt );

			}

			else{

				qt[ll_pos].first_path = dest_first_path;

			}

			break;

		case 3:

			if ( lr.leafinvlist.size() != 1 ){

				quadtree_split( lr_pos, dest, dest_first_path, qt );

			}

			else{

				qt[lr_pos].first_path = dest_first_path;

			}

			break;

	}



	// after all, clear pos!

	qt[pos].isleaf = false;

	qt[pos].first_path = -1;

	qt[pos].leafinvlist.clear();

}



void quadtree_add( int dest, int dest_first_path, QuadTree &qt ){

	int pos = 0;

	double x = Nodes[dest].x;

	double y = Nodes[dest].y;



	// locate a node on quad tree

	while( !qt[pos].isleaf ){

		for ( int i = 0; i < qt[pos].children.size(); i++ ){

			int cid = qt[pos].children[i];

			if ( in_rect(x, y, qt[cid].llx, qt[cid].lly, qt[cid].urx, qt[cid].ury) ){

				pos = cid;

				break;

			}

		}	

	}



	// add to the leaf node

	qt[pos].leafinvlist.push_back(dest);

	// if this distort, then split

	if ( qt[pos].first_path != dest_first_path ){

		if ( qt[pos].first_path == -1 ){

			qt[pos].first_path = dest_first_path;

		}

		else{

			quadtree_split( pos, dest, dest_first_path, qt );			

		}

	}

}



int morton_list_binary_search( vector<MortonBlock> &morton_list, long long value ){

	int left = 0;

	int right = morton_list.size() - 1;

	int mid;

	while( left <= right ){

		mid = ( left + right ) / 2;

		if ( mid == left ){

			if ( value >= morton_list[right].z_order ){

				return right;

			}

			else{

				return left;

			}

		}

		else if ( value < morton_list[mid].z_order ){

			right = mid - 1;

		}

		else{

			left = mid;

		}

	}

}



int morton_find( int src, int dest ){

	double x = Nodes[dest].x;

	double y = Nodes[dest].y;

	double llx = min_llx;

	double lly = min_lly;

	double urx = max_urx;

	double ury = max_ury;

	long long z_order = 0;



	for ( int i = 0; i < MortonLists[src].max_level; i++ ){

		double mid_x = ( llx + urx ) / 2;

		double mid_y = ( lly + ury ) / 2;

		if ( in_rect( x, y, llx, mid_y, mid_x, ury ) ){

			z_order = z_order << 2;

			lly = mid_y;

			urx = mid_x;

		}

		else if ( in_rect( x, y, mid_x, mid_y, urx, ury ) ){

			z_order = ( z_order << 2 ) + 1;

			llx = mid_x;

			lly = mid_y;

		}

		else if ( in_rect( x, y, llx, lly, mid_x, mid_y ) ){

			z_order = ( z_order << 2 ) + 2;

			urx = mid_x;

			ury = mid_y;

		}

		else{

			z_order = ( z_order << 2 ) + 3;

			llx = mid_x;

			ury = mid_y;

		}

	}



	int rst_pos = morton_list_binary_search( MortonLists[src].array, z_order );

	return MortonLists[src].array[rst_pos].first_path;

}



void morton_list_save( MortonList& morton_list, FILE* &fout ){

	// max_level

	int max_level = morton_list.max_level;

	fwrite( &max_level, sizeof(int), 1, fout );



	// array

	int array_count = morton_list.array.size();

	fwrite( &array_count, sizeof(int), 1, fout );



	long long z_order;

	int first_path;

	for ( int i = 0; i < array_count; i++ ){

		z_order = morton_list.array[i].z_order;

		first_path = morton_list.array[i].first_path;

		fwrite( &z_order, sizeof(long long), 1, fout );

		fwrite( &first_path, sizeof(int), 1, fout );

	}

}





void morton_load(){

	FILE* fin = fopen( FILE_MORTON, "rb" );



	// init clear

	MortonLists.clear();

	

	int max_level;

	int array_count;

	long long z_order;

	int first_path;

	for ( int i = 0; i < Nodes.size(); i++ ){

		MortonList morton_list;

		// max_level

		fread( &max_level, sizeof(int), 1, fin );

		morton_list.max_level = max_level;

		// array

		fread( &array_count, sizeof(int), 1, fin );

		for ( int j = 0; j < array_count; j++ ){

			fread( &z_order, sizeof(long long), 1, fin);

			fread( &first_path, sizeof(int), 1, fin);

			MortonBlock mb = { z_order, first_path };

			morton_list.array.push_back(mb);

		}

		// push

		MortonLists.push_back(morton_list);

	}	

	fclose( fin );

}



typedef struct{

	int id;

	long long z_order_start;

}Stack_Status;



void get_morton_list_by_quadtree( QuadTree& qt, MortonList& morton_list ){

	// init stack

	stack<Stack_Status> stk;

	Stack_Status root = { 0, 0 };

	stk.push( root );



	// init result list

	vector<MortonBlock> rst;

	int max_level = 0;

	

	// init get max level of quad tree;

	for ( int i = 0; i < qt.size(); i++ ){

		if ( qt[i].level > max_level ) max_level = qt[i].level;

	}

	

	// hierarchy traversal

	while( !stk.empty() ){

		Stack_Status top = stk.top();

		stk.pop();

		// if leaf, then add to rst

		if ( qt[top.id].isleaf ){

			if ( qt[top.id].first_path != -1 ){

				MortonBlock mb;

			    mb.z_order = top.z_order_start << ( 2 * ( max_level - qt[top.id].level ) );

				mb.first_path = qt[top.id].first_path;

				rst.push_back( mb );

			}		

		}

		else{

			for ( int i = qt[top.id].children.size() - 1; i >= 0; i-- ){

				Stack_Status child = { qt[top.id].children[i], ( top.z_order_start << 2 ) + i };	

				stk.push( child );

			}		

		}

	}

	

	// merge siblings

	morton_list.max_level = max_level;

	morton_list.array.clear();

	for ( int i = 0; i < rst.size(); i++ ){

		if ( i == 0 ){

			morton_list.array.push_back( rst[i] );

		}

		else if ( rst[i].first_path == rst[i-1].first_path ){

			continue;

		}

		else{

			morton_list.array.push_back( rst[i] );

		}

	}

}



void print_quadtree(QuadTree& qt){

	for (int i = 0; i < qt.size(); i++ ){

		printf("----------\n");



		printf("NODE=%d\n", i );

		printf("X=[%lf, %lf], y=[%lf, %lf]\n", qt[i].llx, qt[i].urx, qt[i].lly, qt[i].ury ); 

		printf("ISLEAF=%d FIRST_PATH=%d\n", qt[i].isleaf, qt[i].first_path );

		printf("CHILDREN: ");

		for (int j = 0; j < qt[i].children.size(); j++ ){

			printf("%d ", qt[i].children[j] );

		}

		printf("\n");

		printf("LEFAINVLIST: ");

		for (int j = 0; j < qt[i].leafinvlist.size(); j++ ){

			printf("%d ", qt[i].leafinvlist[j] );

		}

		printf("\n");

		printf("----------\n");

	}

}



void print_morton_list(MortonList& morton_list){

	printf("---------- MORTON LIST ------------\n");

	printf("MAX_LEVEL=%d\n", morton_list.max_level );

	for (int i = 0; i < morton_list.array.size(); i++ ){

		printf("(%lld,%d) | ", morton_list.array[i].z_order, morton_list.array[i].first_path );

	}

	printf("\n----------\n");

}



typedef struct{

	int id;

	bool isgd; // is graph distance

	double dis;

}Search_Status;



typedef struct{

	bool operator()( const Search_Status& l, const Search_Status& r ){

		return l.dis > r.dis; 

	}

}Search_Status_Comp;



bool l_compare( const Search_Status& l, const Search_Status& r ){

	return l.dis < r.dis;

}



double get_euclidean_dis( int src, int dest ){

	return sqrt((Nodes[src].x - Nodes[dest].x) * (Nodes[src].x - Nodes[dest].x) + (Nodes[src].y - Nodes[dest].y) * (Nodes[src].y - Nodes[dest].y));

}



double get_graph_dis( int src, int dest ){

	int current = src;

	int next;

	double rst = 0;

	//cout<<"src="<<src<<" dest="<<dest<<endl;

	//cout<<"-----------------------------"<<endl;

	while( current != dest ){

	//	cout<<current<<" ";

		next = morton_find( current, dest );

		rst += get_euclidean_dis(current, next);

		current = next;

		

	}

	//cout<<endl;



	return rst;

}



void knn_query( int src, int K, vector<int>& cands, vector<int> &result ){

//	TIME_TICK_START

	vector<Search_Status> l;

	int pos = 0;

	for ( int i = 0; i < cands.size(); i++ ){

		Search_Status current = { cands[i], get_euclidean_dis(src, cands[i]) }; 

		l.push_back( current );

	}

	sort( l.begin(), l.end(), l_compare );

	result.clear();

//	TIME_TICK_END

//	TIME_TICK_PRINT("pre")

//	cout<<"Enclidean done"<<endl;

	// time start here!

//	TIME_TICK_START

	vector<Search_Status> pq;

	while( pos < cands.size() && result.size() < K ){

		// printf("pq=%d result=%d\n", pq.size(), result.size());



		if ( pq.empty() ){

			Search_Status top = { l[pos].id, get_graph_dis(src, l[pos].id) };

			pq.push_back(top);

			make_heap( pq.begin(), pq.end(), Search_Status_Comp() );

			pos ++;

		}

		else{

			if ( l[pos].dis < pq[0].dis ){

				Search_Status top = { l[pos].id, get_graph_dis(src, l[pos].id) };

				pq.push_back(top);

				make_heap( pq.begin(), pq.end(), Search_Status_Comp() );

				pos ++;

			}

			else{

				result.push_back( pq[0].id );

				pop_heap( pq.begin(), pq.end(), Search_Status_Comp() );

				pq.pop_back();

			}

		}

	}

//	TIME_TICK_END

//	TIME_TICK_PRINT("cal")

}



/*

void knn_query( int src, int K, const vector<int> &cands, vector<int> &result ){

	vector<Search_Status> pq;

	for ( int i = 0; i < cands.size(); i++ ){

		

	//	if (i % (cands.size() / 100) == 0) printf("Now %d% @ Obj %d\n", i / (cands.size() / 100), i);



		Search_Status current = { cands[i], false, get_euclidean_dis(src, cands[i]) }; 

		pq.push_back( current );

	}

	make_heap( pq.begin(), pq.end(), Search_Status_Comp() );



	result.clear();



	TIME_TICK_START



	while( !pq.empty() && result.size() < K ){

		Search_Status top = pq[0];

		pop_heap( pq.begin(), pq.end(), Search_Status_Comp() );

		pq.pop_back();



		if ( top.isgd ){

			result.push_back( top.id );

		}

		else{

			top.dis = get_graph_dis( src, top.id );

			top.isgd = true;		

			pq.push_back(top);

			make_heap( pq.begin(), pq.end(), Search_Status_Comp() );

		}

	}

}

*/



void test(){

	FILE* fin;

	fin = fopen(FILE_OBJECT, "r");

	int cases;

	fscanf(fin, "%d", &cases);



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

		vector<int> objset;

		for (int j = 0; j < num_obj; j ++){

			int obj;

			fscanf(fin, "%d", &obj);

			objset.push_back(obj);

		}



		// read locid and test

		int num_ql, locid, K;



		fscanf(fin, "%d", &K);

		fscanf(fin, "%d", &num_ql);



		for (int j = 0; j < num_ql; j ++){

			fscanf(fin, "%d", &locid);

			// init result

			vector<int> result;



			// test start!

			TIME_TICK_START

			knn_query(locid, K, objset, result); 

			TIME_TICK_END



			single_time += TIME_TICK_DIFF

			all_cases ++;



			printf("Current: %lld\n",  (all_time + single_time)/ all_cases);

		}

		all_time += single_time;

		printf("Cases_%d: %lld\n", i, single_time / num_ql);

	}

	printf("Overall: %lld\n", all_time / all_cases);

	fclose(fin);

}





int main(int argc, char* argv[]){

	// args

	if (argc != 6){

		printf("Usage: exec FILE_NODE FILE_EDGE WEIGHT_INFLATE_FACTOR FILE_MORTON FILE_OBJECT\n");

		exit(0);

	}

	else {

		strcpy(FILE_NODE, argv[1]);
		strcpy(FILE_EDGE, argv[2]);
		WEIGHT_INFLATE_FACTOR = atoi(argv[3]);
		strcpy(FILE_MORTON, argv[4]);
		strcpy(FILE_OBJECT, argv[5]);

	}



	TIME_TICK_START
	init();
	TIME_TICK_END
	TIME_TICK_PRINT("INIT");



	TIME_TICK_START	
	morton_load();
	TIME_TICK_END
	TIME_TICK_PRINT("MORTON LOAD");

	

	// test

	/*

	TIME_TICK_START

	test();

	TIME_TICK_END

	TIME_TICK_PRINT("TEST")

	*/

	vector<int> object;

	vector<int> result;

	int num_node=430000;

	FILE* fin;

	fin=fopen(FILE_OBJECT,"r");

	int ob,ind;

	while(fscanf(fin,"%d %d",&ob,&ind)==2)

		object.push_back(ob);

	//for(int i=0;i<num_node/100;i++)

	//	object.push_back(rand()%num_node);

	

	int kset[]={1,5,10,20,50};

	for(int m=2;m<5;m++){

	int k=kset[m];

	TIME_TICK_START

	for(int j=0;j<10;j++){

	int S=rand()%num_node;

	result.clear();

	knn_query(S,k,object,result);

	cout<<"S="<<S<<endl;

	for(int p=0;p<k;p++)

		cout<<result[p]<<" ";

	cout<<endl;

	//cout<<j<<endl;

	}

	TIME_TICK_END

	std::cout<<"k="<<k<<std::endl;

	TIME_TICK_PRINT("knn")

	}

	return 0;

}

