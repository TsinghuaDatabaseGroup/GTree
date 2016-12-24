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
using namespace std;

// MACRO for time tick
struct timeval tv;
long ts, te;
#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_PRINT(T) printf("%s RESULT: %ld (0.01MS)\r\n", (#T), te - ts );
// ----------

#define FILE_NODE "../../src/data/col.cnode"
// cal=10000 SF=NA=100 ny=100000 e=100000 col=100000

#define FILE_PATH "../../common_data/col.path.Q4.dat"
#define FACTOR 0.75
#define PATH_SIZE 1000

#define MAX_INF 1e100
#define MIN_INF -1e100

typedef struct{
	double x,y;
	vector<int> adjnodes;
	vector<int> adjweight;
}Node;

vector<Node> Nodes;

double min_llx, min_lly, max_urx, max_ury;

void init_input(){
	FILE *fin;

	// load node
	printf("LOADING NODE...");
	fin = fopen(FILE_NODE,"r");
	int nid;
	double x,y;
	while( fscanf(fin, "%d %lf %lf", &nid, &x, &y ) == 3 ){
		Node node = { x, y };
		Nodes.push_back(node);
	}
	fclose(fin);
	printf("COMPLETE. NODE_COUNT=%d\n", Nodes.size());

	// load edge
/*	printf("LOADING EDGE...");
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
*/

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

double get_euclidean_dis( int src, int dest ){
	return sqrt((Nodes[src].x - Nodes[dest].x) * (Nodes[src].x - Nodes[dest].x) + (Nodes[src].y - Nodes[dest].y) * (Nodes[src].y - Nodes[dest].y));
}

int main(){
	init();

	FILE *fout = fopen( FILE_PATH, "w" );

	vector<int> s,t;
    s.clear();
	t.clear();
    for ( int i = 0; i < Nodes.size(); i++ ){
        s.push_back(i);
		t.push_back(i);
    }

    random_shuffle( s.begin(), s.end() );
    random_shuffle( t.begin(), t.end() );

	double diag = sqrt((max_urx-min_llx)*(max_urx-min_llx)+(max_ury-min_lly)*(max_ury-min_lly));
	double mindis = diag * FACTOR;

	int count = 0;
	for (int i = 0; i < Nodes.size(); i++ ){
		if ( s[i] == t[i] ) continue;
		if ( get_euclidean_dis( s[i], t[i] ) < mindis ) continue;

		fprintf( fout, "%d %d\n", s[i], t[i] );
		count ++;
		if ( count >= PATH_SIZE ) break;
	}

	fclose( fout );
	return 0;
}
