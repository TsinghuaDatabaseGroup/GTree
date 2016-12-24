/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program builds an index for a graph.

    Suggested arguments:
    > (prog name) -n nodefile.txt -e edgefile.txt -i graph.idx -v
    explanations:
    -n: node file
    -e: edge file
    -i: graph index file
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */
#include "graph.h"
#include "node.h"
#include "edge.h"
#include "param.h"
#include "segfmem.h"
#include "collection.h"
#include <iostream>
#include <fstream>

using namespace std;

#define MAXNODES    100000
#define PAGESIZE    4096    // 4KB page

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-n nodefile.txt -e edgefile.txt -i graph.idx -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-n: node file" << endl;
    cerr << "    format: nodeID  x y " << endl;
    cerr << "-e: edge file" << endl;
    cerr << "    format: src_nodeID dest_nodeID cost" << endl;
    cerr << "-i: graph index file (output" << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "graph loader" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* nodeflname = Param::read(a_argc, a_argv, "-n", "");
    const char* edgeflname = Param::read(a_argc, a_argv, "-e", "");
    const char* idxflname = Param::read(a_argc, a_argv, "-i", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    bool verbose = strcmp(vrbs,"null") != 0;
    fstream fnode, fedge;

    Node** node = new Node*[MAXNODES];
    memset(node, 0, sizeof(Node*)*MAXNODES);
    int nodecnt=0;

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

        node[id] = new Node(id,x,y);
        nodecnt = nodecnt > id ? nodecnt : id;
    }
	cerr << "done" << endl;

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
        Edge edge2src(src, cost);
        node[src]->addEdge(edge2dest);
        node[dest]->addEdge(edge2src);
    }
	cerr << "done" << endl;

    fnode.close();
    fedge.close();

    //-------------------------------------------------------------------------
    // insert nodes to graph
    //-------------------------------------------------------------------------
    cerr << "create a graph ... ";
    SegFMemory segmem(idxflname,PAGESIZE*10,PAGESIZE,32,true);    // 10 pages, 1 page
    Graph graph(segmem);
    for (int i=0; i<=nodecnt; i++)
    {
        if (node[i] != 0 && node[i]->m_edges.size() > 0)
            graph.writeNode(node[i]->m_id,*node[i]);
    }
	cerr << "done" << endl;

    //-------------------------------------------------------------------------
    // testing
    //-------------------------------------------------------------------------
    cerr << "testing ... ";
    for (int i=0; i<=nodecnt; i++)
    {
        if (node[i] == 0) continue;

        Node* m = graph.getNode(node[i]->m_id);
        if (m->m_id != node[i]->m_id)
        {
            cerr << "fail in node " << m->m_id << endl;
            return -1;
        }
        if (m->m_edges.size() != node[i]->m_edges.size())
        {
            cerr << "fail in #edges " << endl;
            return -1;
        }
        for (int c1=0; c1<m->m_edges.size(); c1++)
        {
            int found = 0;
            const Edge* e1 = (const Edge*)m->m_edges.get(c1);
            for (int c2=0; c2<node[i]->m_edges.size(); c2++)
            {
                const Edge* e2 = (const Edge*)node[i]->m_edges.get(c2);
                if (e1->m_neighbor == e2->m_neighbor &&
                    e1->m_cost == e2->m_cost)
                    found = 1;
            }
            if (found != 1)
            {
                cerr << "fail in edge content" << endl;
                return -1;
            }
        }
        delete m;
    }
	cerr << "done" << endl;

	cerr << "complete!" << endl;

    return 0;
}

