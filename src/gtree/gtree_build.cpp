// macro for 64 bits file, larger than 2G
#define _FILE_OFFSET_BITS 64

#include<stdio.h>
#include<metis.h>
#include<vector>
#include<stdlib.h>
#include<memory.h>
#include<unordered_map>
#include<map>
#include<set>
#include<deque>
#include<stack>
#include<algorithm>
#include<sys/time.h>
using namespace std;

// MACRO for timing
struct timeval tv;
long long ts, te;
#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 100000 + tv.tv_usec / 10;
#define TIME_TICK_PRINT(T) printf("%s RESULT: %lld (0.01MS)\r\n", (#T), te - ts );
// ----------

#define FILE_NODE "cal.cnode"
#define FILE_EDGE "cal.cedge"
// set all edge weight to 1(unweighted graph)
#define ADJWEIGHT_SET_TO_ALL_ONE true
// we assume edge weight is integer, thus (input edge) * WEIGHT_INFLATE_FACTOR = (our edge weight)
#define WEIGHT_INFLATE_FACTOR 100000
// gtree fanout
#define PARTITION_PART 4
// gtree leaf node capacity = tau(in paper)
#define LEAF_CAP 32
// gtree index disk storage
#define FILE_NODES_GTREE_PATH "cal.paths"
#define FILE_GTREE 			  "cal.gtree"
#define FILE_ONTREE_MIND	  "cal.minds"

typedef struct{
	double x,y;
	vector<int> adjnodes;
	vector<int> adjweight;
	bool isborder;
	vector<int> gtreepath; // this is used to do sub-graph locating
}Node;

typedef struct{
	vector<int> borders;
	vector<int> children;
	bool isleaf;
	vector<int> leafnodes;
	int father;
// ----- min dis -----
	vector<int> union_borders; // for non leaf node	
	vector<int> mind; // min dis, row by row of union_borders
// ----- for pre query init, OCCURENCE LIST in paper -----
	vector<int> nonleafinvlist;
	vector<int> leafinvlist;
	vector<int> up_pos;
	vector<int> current_pos;
}TreeNode;

int noe; // number of edges
vector<Node> Nodes;
vector<TreeNode> GTree;

// use for metis
// idx_t = int64_t / real_t = double
idx_t nvtxs; // |vertices|
idx_t ncon; // number of weight per vertex
idx_t* xadj; // array of adjacency of indices
idx_t* adjncy; // array of adjacency nodes
idx_t* vwgt; // array of weight of nodes
idx_t* adjwgt; // array of weight of edges in adjncy
idx_t nparts; // number of parts to partition
idx_t objval; // edge cut for partitioning solution
idx_t* part; // array of partition vector
idx_t options[METIS_NOPTIONS]; // option array

// METIS setting options
void options_setting(){
	METIS_SetDefaultOptions(options);
	options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY; // _RB
	options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT; // _VOL
	options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM; // _RM
	options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_RANDOM; // _GROW _EDGE _NODE
	options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM; // _GREEDY _SEP2SIDED _SEP1SIDED
	// options[METIS_OPTION_NCUTS] = 1;
	// options[METIS_OPTION_NITER] = 10;
	/* balance factor, used to be 500 */
	options[METIS_OPTION_UFACTOR] = 500;
	// options[METIS_OPTION_MINCONN];
	options[METIS_OPTION_CONTIG] = 1;
	// options[METIS_OPTION_SEED];
	options[METIS_OPTION_NUMBERING] = 0;
	// options[METIS_OPTION_DBGLVL] = 0;
}

// input init
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
	printf("COMPLETE. NODE_COUNT=%d\n", (int)Nodes.size());

	// load edge
	printf("LOADING EDGE...");
	fin = fopen(FILE_EDGE, "r");
	int eid;
	int snid, enid;
	double weight;
	int iweight;
	noe = 0;
	while( fscanf(fin,"%d %d %d %lf", &eid, &snid, &enid, &weight ) == 4 ){
		noe ++;
		iweight = (int) (weight * WEIGHT_INFLATE_FACTOR );
		Nodes[snid].adjnodes.push_back( enid );
		Nodes[snid].adjweight.push_back( iweight );
		Nodes[enid].adjnodes.push_back( snid );
		Nodes[enid].adjweight.push_back( iweight );
	}
	fclose(fin);
	printf("COMPLETE.\n");
}

// transform original data format to that suitable for METIS
void data_transform_init( set<int> &nset ){
	// nvtxs, ncon
	nvtxs = nset.size();
	ncon = 1;
	
	xadj = new idx_t[nset.size() + 1];
	adjncy = new idx_t[noe * 2];
	adjwgt = new idx_t[noe * 2];


	int xadj_pos = 1;
	int xadj_accum = 0;
	int adjncy_pos = 0;

	// xadj, adjncy, adjwgt
	unordered_map<int,int> nodemap;
	nodemap.clear();

	xadj[0] = 0;
	int i = 0;
	for ( set<int>::iterator it = nset.begin(); it != nset.end(); it++, i++ ){
		// init node map
		nodemap[*it] = i;

		int nid = *it;
		int fanout = Nodes[nid].adjnodes.size();
		for ( int j = 0; j < fanout; j++ ){
			int enid = Nodes[nid].adjnodes[j];
			// ensure edges within
			if ( nset.find( enid ) != nset.end() ){
				xadj_accum ++;

				adjncy[adjncy_pos] = enid;
				adjwgt[adjncy_pos] = Nodes[nid].adjweight[j];
				adjncy_pos ++;
			}
		}
		xadj[xadj_pos++] = xadj_accum;
	}

	// adjust nodes number started by 0
	for ( int i = 0; i < adjncy_pos; i++ ){
		adjncy[i] = nodemap[adjncy[i]];
	}

	// adjwgt -> 1
	if (ADJWEIGHT_SET_TO_ALL_ONE){
		for ( int i = 0; i < adjncy_pos; i++ ){
			adjwgt[i] = 1;
		}
	}

	// nparts
	nparts = PARTITION_PART;

	// part
	part = new idx_t[nset.size()];
}

void init(){
	init_input();
	options_setting();
}

void finalize(){
    delete xadj;
    delete adjncy;
    delete adjwgt;
    delete part;
}

// graph partition
// input: nset = a set of node id
// output: <node, node belong to partition id>
unordered_map<int,int> graph_partition( set<int> &nset ){
	unordered_map<int,int> result;

	// transform data to metis
	data_transform_init( nset );		

	// partition, result -> part
	// k way partition
	METIS_PartGraphKway(
        &nvtxs,
        &ncon,
        xadj,
        adjncy,
        NULL,
        NULL,
        adjwgt,
        &nparts,
        NULL,
        NULL,
        options,
        &objval,
        part
    );	

	// push to result
	result.clear();
	int i = 0;
	for ( set<int>::iterator it = nset.begin(); it != nset.end(); it++, i++ ){
		result[*it] = part[i];
	}

	// finalize
	finalize();

	return result;
}

// init status struct
typedef struct{
	int tnid; // tree node id
	set<int> nset; // node set
}Status;

// gtree construction
void build(){
	// init root
	TreeNode root;
	root.isleaf = false;
	root.father = -1;
	GTree.push_back(root);

	// init stack
	stack<Status> buildstack;
	Status rootstatus;
	rootstatus.tnid = 0;
	rootstatus.nset.clear();
	for ( int i = 0; i < Nodes.size(); i++ ){
		rootstatus.nset.insert(i);
	}
	buildstack.push( rootstatus );

	// start to build
	unordered_map<int,int> presult;
	set<int> childset[PARTITION_PART];


	while( buildstack.size() > 0 ){
		// pop top
		Status current = buildstack.top();
		buildstack.pop();

		// update gtreepath
		for ( set<int>::iterator it = current.nset.begin(); it != current.nset.end(); it++ ){
			Nodes[*it].gtreepath.push_back( current.tnid );
		}

		// check cardinality
		if ( current.nset.size() <= LEAF_CAP ){
			// build leaf node
			GTree[current.tnid].isleaf = true;
			GTree[current.tnid].leafnodes.clear();
			for ( set<int>::iterator it = current.nset.begin(); it != current.nset.end(); it++ ){
				GTree[current.tnid].leafnodes.push_back( *it );
			}
			continue;
		}

		// partition
//		printf("PARTITIONING...NID=%d...SIZE=%d...", current.tnid, (int)current.nset.size() );
		presult = graph_partition( current.nset );
//		printf("COMPLETE.\n");

		// construct child node set
		for ( int i = 0; i < PARTITION_PART; i++ ){
			childset[i].clear();
		}
		int slot;
		for ( set<int>::iterator it = current.nset.begin(); it != current.nset.end(); it++ ){
			slot = presult[*it];
			childset[slot].insert(*it);
		}

		// generate child tree nodes
		int childpos;
		for ( int i = 0; i < PARTITION_PART; i++ ){
			TreeNode tnode;
			tnode.isleaf = false;
			tnode.father = current.tnid;
			
			// insert to GTree first
            GTree.push_back(tnode);
			childpos = GTree.size() - 1;
			GTree[current.tnid].children.push_back( childpos );

			// calculate border nodes
			GTree[childpos].borders.clear();
			for ( set<int>::iterator it = childset[i].begin(); it != childset[i].end(); it++ ){

				bool isborder = false;
				for ( int j = 0; j < Nodes[*it].adjnodes.size(); j++ ){
					if ( childset[i].find( Nodes[*it].adjnodes[j] ) == childset[i].end() ){
						isborder = true;
						break;
					}
				}
				if ( isborder ){
					GTree[childpos].borders.push_back(*it);
					// update globally
					Nodes[*it].isborder = true;
				}
			}

			// add to stack
			Status ongoingstatus;
			ongoingstatus.tnid = childpos;
			ongoingstatus.nset = childset[i];
			buildstack.push(ongoingstatus);

		}

	}
	
	
}

// dump gtree index to file
void gtree_save(){
	// FILE_GTREE
	FILE *fout = fopen( FILE_GTREE, "wb" );
	int *buf = new int[ Nodes.size() ];
	for ( int i = 0; i < GTree.size(); i++ ){
		// borders
		int count_borders = GTree[i].borders.size();
		fwrite( &count_borders, sizeof(int), 1, fout );
		copy( GTree[i].borders.begin(), GTree[i].borders.end(), buf );
		fwrite( buf, sizeof(int), count_borders, fout );
		// children
		int count_children = GTree[i].children.size();
		fwrite( &count_children, sizeof(int), 1, fout );
		copy( GTree[i].children.begin(), GTree[i].children.end(), buf );
		fwrite( buf, sizeof(int), count_children, fout );
		// isleaf
		fwrite( &GTree[i].isleaf, sizeof(bool), 1, fout );
		// leafnodes
		int count_leafnodes = GTree[i].leafnodes.size();
		fwrite( &count_leafnodes, sizeof(int), 1, fout );
		copy( GTree[i].leafnodes.begin(), GTree[i].leafnodes.end(), buf );
		fwrite( buf, sizeof(int), count_leafnodes, fout );
		// father
		fwrite( &GTree[i].father, sizeof(int), 1, fout );
	}
	fclose(fout);

	// FILE_NODES_GTREE_PATH
	fout = fopen( FILE_NODES_GTREE_PATH, "wb" );
	for ( int i = 0; i < Nodes.size(); i++ ){
		int count = Nodes[i].gtreepath.size();
		fwrite( &count, sizeof(int), 1, fout );
		copy( Nodes[i].gtreepath.begin(), Nodes[i].gtreepath.end(), buf );
		fwrite( buf, sizeof(int), count, fout );
	}
	fclose(fout);
	delete[] buf;
}

// load gtree index from file
void gtree_load(){
	// FILE_GTREE
	FILE *fin = fopen( FILE_GTREE, "rb" );
	int *buf = new int[ Nodes.size() ];
	int count_borders, count_children, count_leafnodes;
	bool isleaf;
	int father;

	// clear gtree
	GTree.clear();

	while( fread( &count_borders, sizeof(int), 1, fin ) ){
		TreeNode tn;
		// borders
		tn.borders.clear();
		fread( buf, sizeof(int), count_borders, fin );
		for ( int i = 0; i < count_borders; i++ ){
			tn.borders.push_back(buf[i]);
		}
		// children
		fread( &count_children, sizeof(int), 1, fin );
		fread( buf, sizeof(int), count_children, fin );
		for ( int i = 0; i < count_children; i++ ){
			tn.children.push_back(buf[i]);
		}
		// isleaf
		fread( &isleaf, sizeof(bool), 1, fin );
		tn.isleaf = isleaf;
		// leafnodes
		fread( &count_leafnodes, sizeof(int), 1, fin );
		fread( buf, sizeof(int), count_leafnodes, fin );
		for ( int i = 0; i < count_leafnodes; i++ ){
			tn.leafnodes.push_back(buf[i]);
		}
		// father
		fread( &father, sizeof(int), 1, fin );
		tn.father = father;

		GTree.push_back(tn);
	}
	fclose(fin);
	
	// FILE_NODES_GTREE_PATH
	int count;
	fin = fopen( FILE_NODES_GTREE_PATH, "rb" );
	int pos = 0;
	while( fread( &count, sizeof(int), 1, fin ) ){
		fread( buf, sizeof(int), count, fin );
		// clear gtreepath
		Nodes[pos].gtreepath.clear();
		for ( int i = 0; i < count; i++ ){
			Nodes[pos].gtreepath.push_back( buf[i] );
		}
		// pos increase
		pos ++;
	}
	fclose(fin);
	delete[] buf;
}

// dijkstra search, used for single-source shortest path search WITHIN one gtree leaf node!
// input: s = source node
//        cands = candidate node list
//        graph = search graph(this can be set to subgraph)
vector<int> dijkstra_candidate( int s, vector<int> &cands, vector<Node> &graph ){
	// init
	set<int> todo;
	todo.clear();
	todo.insert(cands.begin(), cands.end());
	
	unordered_map<int,int> result;
	result.clear();
	set<int> visited;
	visited.clear();
	unordered_map<int,int> q;
	q.clear();
	q[s] = 0;

	// start
	int min, minpos, adjnode, weight;
	while( ! todo.empty() && ! q.empty() ){
		min = -1;
		for ( unordered_map<int,int>::iterator it = q.begin(); it != q.end(); it ++ ){
			if ( min == -1 ){
				minpos = it -> first;
				min = it -> second;
			}
			else{
				if ( it -> second < min ){
					min = it -> second;
					minpos = it -> first;
				}
			}
		}

		// put min to result, add to visited
		result[minpos] = min;
		visited.insert( minpos );
		q.erase(minpos);

		if ( todo.find( minpos ) != todo.end() ){
			todo.erase( minpos );
		}

		// expand
		for ( int i = 0; i < graph[minpos].adjnodes.size(); i++ ){
			adjnode = graph[minpos].adjnodes[i];
			if ( visited.find( adjnode ) != visited.end() ){
				continue;
			}
			weight = graph[minpos].adjweight[i];

			if ( q.find(adjnode) != q.end() ){
				if ( min + weight < q[adjnode] ){
					q[adjnode] = min + weight;
				}
			}
			else{
				q[adjnode] = min + weight;
			}
		
		}
	}

	// output
	vector<int> output;
	for ( int i = 0; i < cands.size(); i++ ){
		output.push_back( result[cands[i]] );
	}

	// return
	return output;
}

// calculate the distance matrix, algorithm shown in section 5.2 of paper
void hierarchy_shortest_path_calculation(){
	// level traversal
	vector< vector<int> > treenodelevel;
	
	vector<int> current;
	current.clear();
	current.push_back(0);
	treenodelevel.push_back(current);

	vector<int> mid;
	while( current.size() != 0 ){
		mid = current;
		current.clear();
		for ( int i = 0; i < mid.size(); i++ ){
			for ( int j = 0; j < GTree[mid[i]].children.size(); j++ ){
				current.push_back( GTree[mid[i]].children[j] );
			}
		}
		if ( current.size() == 0 ) break;
		treenodelevel.push_back( current );
	}
	
	// bottom up calculation
	// temp graph
	vector<Node> graph;
	graph = Nodes;
	vector<int> cands;
	vector<int> result;
	unordered_map<int, unordered_map<int,int> > vertex_pairs;

	// do dijkstra
	int s, t, tn, nid, cid, weight;
	vector<int> tnodes, tweight;
	set<int> nset;

	for ( int i = treenodelevel.size() - 1; i >= 0; i-- ){
		for ( int j = 0; j < treenodelevel[i].size(); j++ ){
			tn = treenodelevel[i][j];

			cands.clear();
			if ( GTree[tn].isleaf ){
				// cands = leafnodes
				cands = GTree[tn].leafnodes;
				// union borders = borders;
				GTree[tn].union_borders = GTree[tn].borders;
			}
			else{
				nset.clear();
				for ( int k = 0; k < GTree[tn].children.size(); k++ ){
					cid = GTree[tn].children[k];
					nset.insert( GTree[cid].borders.begin(), GTree[cid].borders.end() );
				}
				// union borders = cands;
				
				cands.clear();
				for ( set<int>::iterator it = nset.begin(); it != nset.end(); it ++ ){
					cands.push_back( *it );
				}
				GTree[tn].union_borders = cands;
			}
				
			// start to do min dis
			vertex_pairs.clear();
				
			// for each border, do min dis
			int cc = 0;

			for ( int k = 0; k < GTree[tn].union_borders.size(); k++ ){
				//printf("DIJKSTRA...LEAF=%d BORDER=%d\n", tn, GTree[tn].union_borders[k] );
				result = dijkstra_candidate( GTree[tn].union_borders[k], cands, graph );
				//printf("DIJKSTRA...END\n");

				// save to map
				for ( int p = 0; p < result.size(); p ++ ){
					GTree[tn].mind.push_back( result[p] );
					vertex_pairs[GTree[tn].union_borders[k]][cands[p]] = result[p];
				}
			}

			// IMPORTANT! after all border finished, degenerate graph
			// first, remove inward edges
			for ( int k = 0; k < GTree[tn].borders.size(); k++ ){
				s = GTree[tn].borders[k];
				tnodes.clear();
				tweight.clear();
				for ( int p = 0; p < graph[s].adjnodes.size(); p++ ){
					nid = graph[s].adjnodes[p];
					weight = graph[s].adjweight[p];
					// if adj node in same tree node

					if ( graph[nid].gtreepath.size() <= i || graph[nid].gtreepath[i] != tn ){
						// only leave those useful
						tnodes.push_back(nid);
						tweight.push_back(weight);

					}
				}
				// cut it
				graph[s].adjnodes = tnodes;
				graph[s].adjweight = tweight;
			}
			// second, add inter connected edges
			for ( int k = 0; k < GTree[tn].borders.size(); k++ ){
				for ( int p = 0; p < GTree[tn].borders.size(); p++ ){
					if ( k == p ) continue;
					s = GTree[tn].borders[k];
					t = GTree[tn].borders[p];
					graph[s].adjnodes.push_back( t );
					graph[s].adjweight.push_back( vertex_pairs[s][t] );
				}
			}
		}
	}
}

// dump distance matrix into file
void hierarchy_shortest_path_save(){
	FILE* fout = fopen( FILE_ONTREE_MIND, "wb" );
	int* buf;
	int count;
	for ( int i = 0; i < GTree.size(); i++ ){
		// union borders
		count = GTree[i].union_borders.size();
		fwrite( &count, sizeof(int), 1, fout );
		buf = new int[count];
		copy( GTree[i].union_borders.begin(), GTree[i].union_borders.end(), buf );
		fwrite( buf, sizeof(int), count, fout );
		delete[] buf;
		// mind
		count = GTree[i].mind.size();
		fwrite( &count, sizeof(int), 1, fout );
		buf = new int[count];
		copy( GTree[i].mind.begin(), GTree[i].mind.end(), buf );
		fwrite( buf, sizeof(int), count, fout );
		delete[] buf;
	}
	fclose(fout);
}

// load distance matrix from file
void hierarchy_shortest_path_load(){
	FILE* fin = fopen( FILE_ONTREE_MIND, "rb" );
	int* buf;
	int count, pos = 0;
	while( fread( &count, sizeof(int), 1, fin ) ){
		// union borders
		buf = new int[count];
		fread( buf, sizeof(int), count, fin );
		GTree[pos].union_borders.clear();
		for ( int i = 0; i < count; i++ ){
			GTree[pos].union_borders.push_back(buf[i]);
		}
		delete[] buf;
		// mind
		fread( &count, sizeof(int), 1, fin );
		buf = new int[count];
		fread( buf, sizeof(int), count, fin );
		GTree[pos].mind.clear();
		for ( int i = 0; i < count; i++ ){
			GTree[pos].mind.push_back(buf[i]);
		}
		pos++;
		delete[] buf;
	}
	fclose(fin);
}

int main(){
	// init
	TIME_TICK_START
	init();
	TIME_TICK_END
	TIME_TICK_PRINT("INIT")

	// gtree_build
	TIME_TICK_START
	build();
	TIME_TICK_END
	TIME_TICK_PRINT("BUILD")

	// dump gtree
	gtree_save();
	
	// calculate distance matrix
	TIME_TICK_START
	hierarchy_shortest_path_calculation();
	TIME_TICK_END
	TIME_TICK_PRINT("MIND")

	// dump distance matrix
	hierarchy_shortest_path_save();

	return 0;
}
