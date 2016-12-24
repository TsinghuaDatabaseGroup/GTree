/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008

    Copyright(c) Ken C. K. Lee 2008
    This program is for non-commerical use only.

    This program 
    * builds index for a graph and
    * generates (random) objects for a graph.

    Suggested arguments:
    > (prog name)
      -n nodefile.txt -e edgefile.txt
      -m #objects -c #clusters -s #query
      -i graph.idx -o object.dat -q query.dat
      -v
    explanations:
    -n: node file
    -e: edge file
    -m: #objects
    -c: #clusters
    -s: #query points
    -i: graph index file
    -o: object file (a list of nodeid and object id)
    -q: query node file (a list of nodeid and query id)
    -p: file that prints the graph in Postscript
    -v: turn verbose mode on (default: off)
---------------------------------------------------------------------------- */
#include "graph.h"
#include "node.h"
#include "edge.h"
#include "segfmem.h"
#include "objectgen.h"
#include "param.h"
#include "collection.h"
#include "graphplot.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

#define MAXNODES    100000
#define PAGESIZE    4096    // 4KB page

void helpmsg(const char* pgm)
{
    cerr << "Suggested arguments:" << endl;
    cerr << "> " << pgm << " ";
    cerr << "-n nodefile.txt -e edgefile.txt -m #objects -c #clusters ";
    cerr << "-i graph.idx -o object.dat -p output.ps -v" << endl;
    cerr << "explanations:" << endl;
    cerr << "-n: node file" << endl;
    cerr << "    format: nodeID  x y " << endl;
    cerr << "-e: edge file" << endl;
    cerr << "    format: src_nodeID dest_nodeID cost" << endl;
    cerr << "-m: number of objects" << endl;
    cerr << "-c: number of clusters" << endl;
    cerr << "-s: number of query points" << endl;
    cerr << "-i: graph index file (output)" << endl;
    cerr << "-o: object file [nodeid, object id] (output)" << endl;
    cerr << "-q: query file [nodeid, query id] (output)" << endl;
    cerr << "-p: file that prints the graph in Postscript" << endl;
    cerr << "-v: turn verbose mode on (default: off)" << endl;
}

int main(const int a_argc, const char** a_argv)
{
    if (a_argc == 1)
    {
        helpmsg(a_argv[0]);
        return -1;
    }

    cerr << "graph, object and query loader" << endl;
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    const char* nodeflname = Param::read(a_argc, a_argv, "-n", "");
    const char* edgeflname = Param::read(a_argc, a_argv, "-e", "");
    const char* numobject  = Param::read(a_argc, a_argv, "-m", "");
    const char* numcluster = Param::read(a_argc, a_argv, "-c", "");
    const char* numquery = Param::read(a_argc, a_argv, "-s", "");
    const char* idxflname = Param::read(a_argc, a_argv, "-i", "");
    const char* objflname = Param::read(a_argc, a_argv, "-o", "");
    const char* qryflname = Param::read(a_argc, a_argv, "-q", "");
    const char* psflname = Param::read(a_argc, a_argv, "-p", "");
    const char* vrbs = Param::read(a_argc, a_argv, "-v", "null");
    bool verbose = strcmp(vrbs,"null") != 0;
    fstream fnode, fedge;

    Array node2object;          // a mapping between nodes and objects
    Array node2query;           // a mapping between nodes and query nodes
    Hash nodes(10000);          // this is for fast quick lookup
    Array allnodes(10000,100);  // this is for node ordering

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

        Node* node = new Node(id,x,y);
        nodes.put(id,node);
        allnodes.append(node);
    }
    allnodes.sort(Node::compareid);
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
        Edge edge2src(src, cost);
        // source node
        Node* srcnode = (Node*)nodes.get(src);
        srcnode->addEdge(edge2dest);
        // destination node
        Node* destnode= (Node*)nodes.get(dest);
        destnode->addEdge(edge2src);
    }
    cerr << "[DONE]" << endl;

    fnode.close();
    fedge.close();

    //-------------------------------------------------------------------------
    // insert nodes to graph
    //-------------------------------------------------------------------------
    cerr << "create a graph ... ";
    SegFMemory segmem(idxflname,PAGESIZE*10,PAGESIZE,32,true);    // 10 pages, 1 page
    Graph graph(segmem);
    for (int i=0; i<allnodes.size(); i++)
    {
        Node* node = (Node*)allnodes.get(i);
        if (node->m_edges.size() > 0)
            graph.writeNode(node->m_id,*node);
    }
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // create objects
    //-------------------------------------------------------------------------
    cerr << "create objects ... ";
    int numobj = atol(numobject);
    int numclu = atol(numcluster);
    if (numobj > 0)
    {
        if (numclu == 0)
            ObjectGen::uniform(numobj, allnodes, node2object);
        else
            ObjectGen::cluster(numclu, numobj, allnodes, node2object);

        fstream fobj;
        fobj.open(objflname, ios::out);
        for (int i=0; i<node2object.size(); i++)
        {
            NodeObject* nobj = (NodeObject*)node2object.get(i);
            fobj << nobj->m_nodeid << " " << nobj->m_objid << endl;
        }
        fobj.close();
    }
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // create querying nodes
    //-------------------------------------------------------------------------
    cerr << "create querying nodes ... ";
    int numqry = atol(numquery);
    if (numqry > 0)
    {
        ObjectGen::uniform(numqry, allnodes, node2query);

        fstream fqry;
        fqry.open(qryflname, ios::out);
        for (int i=0; i<node2query.size(); i++)
        {
            NodeObject* nobj = (NodeObject*)node2query.get(i);
            fqry << nobj->m_nodeid << " " << nobj->m_objid << endl;
        }
        fqry.close();
    }
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // testing
    //-------------------------------------------------------------------------
    cerr << "testing ... ";
    for (int i=0; i<allnodes.size(); i++)
    {
        Node* node = (Node*)allnodes.get(i);

        Node* m = graph.getNode(node->m_id);
        if (m->m_id != node->m_id)
        {
            cerr << "fail in node " << m->m_id << endl;
            return -1;
        }
        if (m->m_edges.size() != node->m_edges.size())
        {
            cerr << "fail in #edges " << endl;
            return -1;
        }
        for (int c1=0; c1<m->m_edges.size(); c1++)
        {
            int found = 0;
            const Edge* e1 = (const Edge*)m->m_edges.get(c1);
            for (int c2=0; c2<node->m_edges.size(); c2++)
            {
                const Edge* e2 = (const Edge*)node->m_edges.get(c2);
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
    cerr << "[DONE]" << endl;

    //-------------------------------------------------------------------------
    // plot the graph and objects
    //-------------------------------------------------------------------------
    if (psflname[0] != 0)
        GraphPlot::plot(psflname, nodes, node2object, node2query);

    //-------------------------------------------------------------------------
    // clean up
    //-------------------------------------------------------------------------
    for (int i=0; i<allnodes.size(); i++)
        delete (Node*)allnodes.get(i);
    for (int i=0; i<node2object.size(); i++)
        delete (Node*)node2object.get(i);

    return 0;
}

