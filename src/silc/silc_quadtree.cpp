// macro for 64 bits file, larger than 2G
#define _FILE_OFFSET_BITS 64

#include<stdio.h>
#include<vector>
#include<stdlib.h>
#include<memory.h>
#include<map>
#include<set>
#include<deque>
#include<stack>
#include<algorithm>
#include<sys/time.h>
using namespace std;

#define FILE_NODE "../../src/data/SF.cnode"
#define FILE_EDGE "../../src/data/SF.cedge"

#define FILE_MORTON "./data/SF.morton"
// cal=10000 SF=100 ny=100000 e=100000
#define WEIGHT_INFLATE_FACTOR 100
#define ROUND_FACTOR 0.5
#define MAX_INF  1e100
#define MIN_INF -1e100

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
	vector<int> leafinvlist; // valid in build, not for use
}QuadTreeNode;

typedef vector<QuadTreeNode> QuadTree;

vector<Node> Nodes;
vector<QuadTree> Morton;

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
}

void init(){
	init_input();
}

// SPFA
void SPFA_first_path( int src, int* &first_path ){
	// min_dis array
	int *min_dis = new int[ Nodes.size() ];
	bool *used = new bool[ Nodes.size() ];
	deque<int> q;
	
	// init for each starting node
	memset( used, 0, sizeof(bool) * Nodes.size() );
	memset( min_dis, -1, sizeof(int) * Nodes.size() );
	min_dis[src] = 0;
	q.push_back(src);
	used[src] = true;
	// init for first_path
	first_path[src] = -1;
	for ( int i = 0; i < Nodes[src].adjnodes.size(); i++ ){
		first_path[Nodes[src].adjnodes[i]] = Nodes[src].adjnodes[i];
	}

	// start spfa
	long long mindis_sum = 0;
	while( !q.empty() ){
		int top;
		// optimization : large label last
		while( top = q.front(), (long long)min_dis[top] * (long long)q.size() > mindis_sum ){
			q.pop_front();
			q.push_back(top);
		}

		q.pop_front();
		mindis_sum -= min_dis[top];
		used[top] = false;
			
		for ( int j = 0; j < Nodes[top].adjnodes.size(); j++ ){
			int cid = Nodes[top].adjnodes[j];
			int weight = Nodes[top].adjweight[j];
			if ( min_dis[cid] == -1 ){
				min_dis[cid] = min_dis[top] + weight;
				// first_path
				if ( top != src ){
					first_path[cid] = first_path[top];
				}

				if ( !used[cid] ){
					if ( q.size() == 0 ){
						q.push_back(cid);
					}
					else{
						// optimization : small label first
						if ( min_dis[cid] <= min_dis[q.front()] ){
							q.push_front(cid);
						}
						else{
							q.push_back(cid);
						}
					}
					mindis_sum += min_dis[cid];
					used[cid] = true;
				}			
			}
			else{
				if ( min_dis[cid] > min_dis[top] + weight ){
					min_dis[cid] = min_dis[top] + weight;
					// first_path
					if ( top != src ){
						first_path[cid] = first_path[top];
					}

					if ( !used[cid] ){
						if ( q.size() == 0 ){
							q.push_back(cid);
						}
						else{
							// optimization : small label first
							if ( min_dis[cid] <= min_dis[q.front()] ){
								q.push_front(cid);
							}
							else{
								q.push_back(cid);
							}
						}
						mindis_sum += min_dis[cid];
						used[cid] = true;
					}
				}
			}	
		}
	}
	
	delete[] used;
	delete[] min_dis;


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
		// ll
		else if ( in_rect(Nodes[vid].x, Nodes[vid].y, ll.llx, ll.lly, ll.urx, ll.ury) ){
			ll.leafinvlist.push_back(vid);
			if ( vid == dest ) dest_pos = 1;
		}
		// ur
		else if ( in_rect(Nodes[vid].x, Nodes[vid].y, ur.llx, ur.lly, ur.urx, ur.ury) ){
			ur.leafinvlist.push_back(vid);
			if ( vid == dest ) dest_pos = 2;
		}
		else{
			lr.leafinvlist.push_back(vid);
			if ( vid == dest ) dest_pos = 3;
		}
	}
	// first_path
	ul.first_path = qt[pos].first_path;
	ll.first_path = qt[pos].first_path;
	ur.first_path = qt[pos].first_path;
	lr.first_path = qt[pos].first_path;

	// children
	qt.push_back(ul);
	int ul_pos = qt.size() - 1;
	qt[pos].children.push_back( ul_pos );
	
	qt.push_back(ll);
	int ll_pos = qt.size() - 1;
    qt[pos].children.push_back( ll_pos );
	
	qt.push_back(ur);
	int ur_pos = qt.size() - 1;
	qt[pos].children.push_back( ur_pos );
	
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
			if ( ll.leafinvlist.size() != 1 ){
				quadtree_split( ll_pos, dest, dest_first_path, qt );
			}
			else{
				qt[ll_pos].first_path = dest_first_path;
			}
			break;
		case 2:
			if ( ur.leafinvlist.size() != 1 ){
				quadtree_split( ur_pos, dest, dest_first_path, qt );
			}
			else{
				qt[ur_pos].first_path = dest_first_path;
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
		quadtree_split( pos, dest, dest_first_path, qt );			
	}
}

int morton_find( int src, int dest ){
	int pos = 0;
    double x = Nodes[dest].x;
    double y = Nodes[dest].y;

    // locate a node on quad tree
    while( !Morton[src][pos].isleaf ){
        for ( int i = 0; i < Morton[src][pos].children.size(); i++ ){
            int cid = Morton[src][pos].children[i];
            if ( in_rect(x, y, Morton[src][cid].llx, Morton[src][cid].lly, Morton[src][cid].urx, Morton[src][cid].ury) ){
                pos = cid;
                break;
            }
        }
    }

	return Morton[src][pos].first_path;
}

void quadtree_slim_recursive(int pos, QuadTree& qt ){
	vector<int> to_del;

	if ( qt[pos].isleaf && qt[pos].leafinvlist.size() == 0 ){
		to_del.push_back(pos);	
	}
}

void quadtree_slim( QuadTree& qt ){
	// first 
	int count = 0;	
	for ( int i = 0; i < qt.size(); i++ ){
		if ( qt[i].isleaf && qt[i].leafinvlist.size() == 0 ){
			count ++;
		}
	}
	printf("WASTE=%d\n", count );
}

void quadtree_save( QuadTree& qt, FILE* &fout ){
	int* buf;
	int quadtree_count = qt.size();
	fwrite( &quadtree_count, sizeof(int), 1, fout );

	for ( int i = 0; i < quadtree_count; i++ ){
		// coordinate
		fwrite( &qt[i].llx, sizeof(double), 1, fout );
		fwrite( &qt[i].lly, sizeof(double), 1, fout );
		fwrite( &qt[i].urx, sizeof(double), 1, fout );
		fwrite( &qt[i].ury, sizeof(double), 1, fout );
		// isleaf
		fwrite( &qt[i].isleaf, sizeof(bool), 1, fout );
		// first_path
		fwrite( &qt[i].first_path, sizeof(int), 1, fout );
/*
	// leafinvlist
	int leafinvlist_count = Morton[current].leafinvlist.size();
	fwrite( &leafinvlist_count, sizeof(int), 1, fout );
	buf = new int[leafinvlist_count];
	copy( Morton[current].leafinvlist.begin(), Morton[current].leafinvlist.end(), buf );
	fwrite( buf, sizeof(int), leafinvlist_count, fout );
	delete[] buf;
*/
		// children
		int children_count = qt[i].children.size();
		fwrite( &children_count, sizeof(int), 1, fout );
		buf = new int[children_count];
		copy( qt[i].children.begin(), qt[i].children.end(), buf );
		fwrite( buf, sizeof(int), children_count, fout );
		delete[] buf;
	}

}

void morton_load(){
	FILE* fin = fopen( FILE_MORTON, "rb" );

	// init clear
	Morton.clear();
	
	int quadtree_count;
	double coordinate[4];
	bool isleaf;
	int first_path;
	int children_count;
	int* buf;

	while( fread(&quadtree_count, sizeof(int), 1, fin) ){
		QuadTree qt;
		for ( int i = 0; i < quadtree_count; i++ ){
			QuadTreeNode qtn;
		
			// coordinate
			fread( coordinate, sizeof(double), 4, fin );
			qtn.llx = coordinate[0];
			qtn.lly = coordinate[1];
			qtn.urx = coordinate[2];
			qtn.ury = coordinate[3];

			// isleaf
			fread( &isleaf, sizeof(bool), 1, fin );
			qtn.isleaf = isleaf;

			// first_path
			fread( &first_path, sizeof(int), 1, fin );
			qtn.first_path = first_path;
	
			// children
			fread( &children_count, sizeof(int), 1, fin );
			buf = new int[children_count];
			fread( buf, sizeof(int), children_count, fin );
			for ( int i = 0; i < children_count; i++ ){
				qtn.children.push_back(buf[i]);
			}
			delete[] buf;

			qt.push_back(qtn);
		}

		Morton.push_back(qt);
	}
	fclose( fin );
}

void build(){

	FILE *fout = fopen( FILE_MORTON, "wb" );

	// init get whole region
	double min_llx = MAX_INF, min_lly = MAX_INF;
	double max_urx = MIN_INF, max_ury = MIN_INF;
	for ( int i = 0; i < Nodes.size(); i++ ){
		if ( Nodes[i].x < min_llx ) min_llx = Nodes[i].x;
		if ( Nodes[i].x > max_urx ) max_urx = Nodes[i].x;
		if ( Nodes[i].y < min_lly ) min_lly = Nodes[i].y;
		if ( Nodes[i].y > max_ury ) max_ury = Nodes[i].y;
	}

	int * fp = new int[ Nodes.size() ];

	for ( int i = 0; i < Nodes.size(); i++ ){
		// get first path first
		SPFA_first_path( i, fp );
		
		// build quad tree
		QuadTree qt;
		// init root
		QuadTreeNode root;
		root.llx = min_llx; root.lly = min_lly;
		root.urx = max_urx; root.ury = max_ury;
		root.isleaf = true;
		root.first_path = -1;
		qt.push_back(root);


		// start
		for ( int j = 0; j < Nodes.size(); j++ ){
			if ( i == j ) continue;
			quadtree_add( j, fp[j], qt );
		}

		// quadtree slim
		quadtree_slim(qt);

		// push to morton
		// Morton.push_back( qt );


		// save to disk
		quadtree_save( qt, fout );	

		if ( i > 100 ) break;
/*		// check here
		for ( int j = 0; j < Nodes.size(); j++ ){
			if ( i == j ) continue;
			if ( fp[j] != morton_find(i,j) ) printf("j=%d fp=%d morton=%d\n", j, fp[j], morton_find(i,j) );
		}
*/
		printf("ADD QUAD=%d SIZE=%d\n", i, (int)qt.size() );

	}	

	delete[] fp;
	fclose(fout);
}

int main(){
	init();
	
	build();

/*
	int locid, K;
	printf("ENGINE START!\n");
	while( scanf("%d %d", &locid, &K ) == 2 ){
		knn_query(locid,K);	
	}
*/

	return 0;
}
