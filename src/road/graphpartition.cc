/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "graphpartition.h"
#include "graph.h"
#include "node.h"
#include "edge.h"
#include "bordernode.h"
#include "graphsearch.h"
#include "hiergraph.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <iostream>
using namespace std;


#define MAXCOST 100000000

// ----------------------------------------------------------------------------
// local classes
// ----------------------------------------------------------------------------
class NodeEdge
{
public:
    const int   m_src;              // src info
    const float m_srcx, m_srcy;
    const int   m_dest;             // dest info
    const float m_destx, m_desty;
    const int   m_minid;            // their min for sorting
    const float m_minx, m_miny;
    const float m_src2dest, m_dest2src;
public:
    NodeEdge(
        const int a_src, const float a_srcx, const float a_srcy,
        const int a_dest, const float a_destx, const float a_desty,
        const float a_src2dest, const float a_dest2src):
        m_src(a_src), m_srcx(a_srcx), m_srcy(a_srcy),
        m_dest(a_dest), m_destx(a_destx), m_desty(a_desty),
        m_minid(a_src < a_dest ? a_src : a_dest),
        m_minx(a_srcx < a_destx ? a_srcx : a_destx),
        m_miny(a_srcy < a_desty ? a_srcy : a_desty),
        m_src2dest(a_src2dest), m_dest2src(a_dest2src)
        {};
    ~NodeEdge() {};
    static int compareX(const void* a0, const void* a1)
    {
        NodeEdge* e0 = *(NodeEdge**)a0;
        NodeEdge* e1 = *(NodeEdge**)a1;
        if (e0->m_minx < e1->m_minx) return -1;
        if (e0->m_minx > e1->m_minx) return +1;
        if (e0->m_miny < e1->m_miny) return -1;
        if (e0->m_miny > e1->m_miny) return +1;
        if (e0->m_minid < e1->m_minid) return -1;
        if (e0->m_minid > e1->m_minid) return +1;
        return 0;
    };
    static int compareY(const void* a0, const void* a1)
    {
        NodeEdge* e0 = *(NodeEdge**)a0;
        NodeEdge* e1 = *(NodeEdge**)a1;
        if (e0->m_miny < e1->m_miny) return -1;
        if (e0->m_miny > e1->m_miny) return +1;
        if (e0->m_minx < e1->m_minx) return -1;
        if (e0->m_minx > e1->m_minx) return +1;
        if (e0->m_minid < e1->m_minid) return -1;
        if (e0->m_minid > e1->m_minid) return +1;
        return 0;
    };

};

void extractEdge(Hash& a_nodes, Array& a_edges)
{
    for (HashReader rdr(a_nodes); !rdr.isEnd(); rdr.next())
    {
        Node* node = (Node*)rdr.getVal();
        for (int i=0; i<node->m_edges.size(); i++)
        {
            Edge* e = (Edge*)node->m_edges.get(i);
            if (e->m_neighbor < node->m_id)
                continue;
            Node* dst = (Node*)a_nodes.get(e->m_neighbor);
            a_edges.append(
                new NodeEdge(
                    node->m_id,node->m_x,node->m_y,
                    dst->m_id,dst->m_x,dst->m_y,
                    e->m_cost, e->m_cost));
        }
    }
}

void formNode(Array& a_nodeedges, Hash& nodes)
{
    for (int i=0; i<a_nodeedges.size(); i++)
    {
        NodeEdge* e = (NodeEdge*)a_nodeedges.get(i);
        // --------------------------------------------------------------------
        // load source node (create one if it does not exist)
        // --------------------------------------------------------------------
        Node* srcnode = (Node*)nodes.get(e->m_src);
        if (srcnode == 0)
            nodes.put(e->m_src, srcnode=new Node(e->m_src, e->m_srcx, e->m_srcy));
        Edge sedge(e->m_dest, e->m_src2dest);
        srcnode->addEdge(sedge);

        // --------------------------------------------------------------------
        // create destination node (if it does not exist)
        // --------------------------------------------------------------------
        Node* destnode = (Node*)nodes.get(e->m_dest);
        if (destnode == 0)
            nodes.put(e->m_dest, destnode=new Node(e->m_dest, e->m_destx, e->m_desty));
        Edge dedge(e->m_src, e->m_dest2src);
        destnode->addEdge(dedge);
    }
}

void clusterEdge(Array& a_nodeedges, Array& a_border,
                 Array& a_edges1, Array& a_edges2,
                 Array& a_border1, Array& a_border2,
                 int (*compare)(const void*,const void*))
{
    const int numedges = a_nodeedges.size();
    Array node1(numedges/2);

    a_border.sort();
    a_nodeedges.sort(compare);
    a_edges1.clean(); a_edges2.clean();

    for (int i=0; i<numedges/2; i++)
    {
        NodeEdge* e = (NodeEdge*)a_nodeedges.get(i);
        a_edges1.append(e);
        if (a_border.binSearch((void*)e->m_src) != -1)
            a_border1.append((void*)e->m_src);
        if (a_border.binSearch((void*)e->m_dest) != -1)
            a_border1.append((void*)e->m_dest);
        node1.append((void*)e->m_src);
        node1.append((void*)e->m_dest);
    }
    node1.removeDuplicate();

    for (int i=numedges/2; i<numedges; i++)
    {
        NodeEdge* e = (NodeEdge*)a_nodeedges.get(i);
        a_edges2.append(e);
        if (node1.binSearch((void*)e->m_src) != -1)
        {
            a_border1.append((void*)e->m_src);
            a_border2.append((void*)e->m_src);
        }
        else
        {
            if (a_border.binSearch((void*)e->m_src) != -1)
                a_border2.append((void*)e->m_src);
        }
        if (node1.binSearch((void*)e->m_dest) != -1)
        {
            a_border1.append((void*)e->m_dest);
            a_border2.append((void*)e->m_dest);
        }
        else
        {
            if (a_border.binSearch((void*)e->m_dest) != -1)
                a_border2.append((void*)e->m_dest);
        }
    }
    a_border1.removeDuplicate();
    a_border2.removeDuplicate();
}

void binPartition(Array& a_edges, Array& a_border,
                  Array& a_edges1, Array& a_edges2,
                  Array& a_border1, Array& a_border2)
{
    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    Array edges1a(a_edges.size()), edges1b(a_edges.size());
    Array border1a, border1b;

    // ------------------------------------------------------------------------
    // try cluster edges along x and y dimensions
    // ------------------------------------------------------------------------
    clusterEdge(
        a_edges, a_border,
        edges1a, edges1b, border1a, border1b,
        NodeEdge::compareX);
    clusterEdge(
        a_edges, a_border,
        a_edges1, a_edges2, a_border1, a_border2,
        NodeEdge::compareY);

    // ------------------------------------------------------------------------
    // partition
    // ------------------------------------------------------------------------
    if (border1a.size() + border1b.size() < a_border1.size() + a_border2.size())
    {
        a_edges1.copy(edges1a);
        a_edges2.copy(edges1b);
        a_border1.copy(border1a);
        a_border2.copy(border1b);
    }

    // ------------------------------------------------------------------------
    // exchange some edges to yeild fewer border nodes
    // ------------------------------------------------------------------------
    // currently this is disabled.
}

void kpartition(Array& a_edges, Array& a_borders,
                Array& a_edgess, Array& a_borderss,
                const int a_k)
{
    class carrier
    {
    public:
        Array*  m_edges;
        Array*  m_borders;
    public:
        carrier(Array* a_edges, Array* a_borders):
          m_edges(a_edges), m_borders(a_borders) {};
        ~carrier() {};
    };

    Queue q;
    q.enqueue(new carrier(new Array(a_edges), new Array(a_borders)));
    while (q.length() < a_k)
    {
        carrier* c = (carrier*)q.dequeue();

        carrier* c0 = new carrier(new Array(c->m_edges->size()), new Array(c->m_borders->size()));
        carrier* c1 = new carrier(new Array(c->m_edges->size()), new Array(c->m_borders->size()));

        binPartition(
            *c->m_edges, *c->m_borders, 
            *c0->m_edges, *c1->m_edges, 
            *c0->m_borders, *c1->m_borders);
        q.enqueue(c0);
        q.enqueue(c1);

        delete c->m_edges;
        delete c->m_borders;
        delete c;
    }

    while (!q.isEmpty())
    {
        carrier* c = (carrier*)q.dequeue();
        a_edgess.append(c->m_edges);
        a_borderss.append(c->m_borders);
        delete c;
    }
}


class ShortcutPath
{
public:
    const int   m_src;
    const int   m_dest;
    Array       m_path;
    const float m_cost;
public:
    ShortcutPath(const int a_src, const int a_dest, Array& a_path, const float a_cost):
      m_src(a_src), m_dest(a_dest), m_path(a_path), m_cost(a_cost)
      {};
    ~ShortcutPath() {};
};

class GraphTree
{
public:
    const int   m_graphid;
    const int   m_level;
    Array       m_nodeedges;
    Array       m_borders;  // after opertaion, it should be sorted
    Array       m_shortcuts;
    Array       m_children;
public:
    GraphTree(const int a_id, const int a_level):
        m_graphid(a_id), m_level(a_level) {};
    GraphTree(const int a_id, const int a_level, const int sz):
        m_graphid(a_id), m_level(a_level), m_nodeedges(sz,sz) {};
    ~GraphTree() {};
};

GraphTree* partition(Hash& a_nodes, const int a_k, const int a_level)
{
    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    int graphid = 0;
    GraphTree* root = new GraphTree(graphid,0,100000);
    extractEdge(a_nodes, root->m_nodeedges);

    // ------------------------------------------------------------------------
    // Extend the graph tree based on breadth-first strategy
    // ------------------------------------------------------------------------
    Queue q;
    q.enqueue(root);
    while (!q.isEmpty())
    {
        GraphTree* t = (GraphTree*)q.dequeue();
        if (t->m_level >= a_level) continue;

        Array edgess;
        Array borderss;
        kpartition(t->m_nodeedges, t->m_borders, edgess, borderss, a_k);
        for (int i=0; i<a_k; i++)
        {
            Array* edges = (Array*)edgess.get(i);
            Array* borders = (Array*)borderss.get(i);
            GraphTree* child = new GraphTree(++graphid, t->m_level+1, edges->size());
            child->m_nodeedges.copy(*edges);
            child->m_borders.copy(*borders);
            t->m_children.append(child);
            delete edges;
            delete borders;
            q.enqueue(child);
        }
    }
    return root;
}

void deleteGraphTree(GraphTree* a_root)
{
    Queue q;
    q.enqueue(a_root);
    while (!q.isEmpty())
    {
        GraphTree* t = (GraphTree*)q.dequeue();

        for (int i=0; i<t->m_shortcuts.size(); i++)
            delete (ShortcutPath*)t->m_shortcuts.get(i);
        for (int i=0; i<t->m_children.size(); i++)
            q.enqueue(t->m_children.get(i));
        delete t;
    }
}

void computeShortcut(GraphTree* a_root, const int a_level)
{
    Array* gtreelevel = new Array[a_level+1];
    Queue q;
    q.enqueue(a_root);
    while (!q.isEmpty())
    {
        GraphTree* t = (GraphTree*)q.dequeue();
        gtreelevel[t->m_level].append(t);
        for (int i=0; i<t->m_children.size(); i++)
            q.enqueue(t->m_children.get(i));
    }

    // ------------------------------------------------------------------------
    // prepare nodes of (base) graph
    // ------------------------------------------------------------------------
    Hash nodes(10000);
    for (int i=0; i<gtreelevel[a_level].size(); i++)
    {
        GraphTree* t = (GraphTree*)gtreelevel[a_level].get(i);
        formNode(t->m_nodeedges,nodes);
    }

    for (int i=a_level; i>=0; i--)
    {
cerr << "create shortcuts at level: " << i << " (#subnets:" << gtreelevel[i].size() << ")" << endl;
        for (int j=0; j<gtreelevel[i].size(); j++)
        {
            GraphTree* t = (GraphTree*)gtreelevel[i].get(j);
            
            // ----------------------------------------------------------------
            // prepare a list of border nodes
            // ----------------------------------------------------------------
            Set borders(t->m_borders.size());
            for (int k=0; k<t->m_borders.size(); k++)
                borders.insert(t->m_borders.get(k));

            for (int k=0; k<t->m_borders.size(); k++)
            {
                int src = (long)t->m_borders.get(k);
                Hash result(t->m_borders.size());
                int na=0, ea=0;
                GraphSearch::shortestPathSearch(nodes, src, borders, result, na, ea);

                // ------------------------------------------------------------
                // add shortcuts
                // ------------------------------------------------------------
                for (HashReader r(result); !r.isEnd(); r.next())
                {
                    int dest = r.getKey();
                    GraphSearchResult* res = (GraphSearchResult*)r.getVal();
                    if (src != dest || result.size() == 1)
                        t->m_shortcuts.append(new ShortcutPath(src, dest, res->m_path, res->m_cost));
                    delete res; // clean the result
                }
            }
        }
        // --------------------------------------------------------------------
        // clean up the nodes;
        // --------------------------------------------------------------------
        for (HashReader rdr(nodes); !rdr.isEnd(); rdr.next())
            delete (Node*)rdr.getVal();
        nodes.clean();

        // --------------------------------------------------------------------
        // construct nodes based on shortcuts
        // --------------------------------------------------------------------
        if (i > 0)
        {
            for (int j=0; j<gtreelevel[i].size(); j++)
            {
                GraphTree* t = (GraphTree*)gtreelevel[i].get(j);
                for (int k=0; k<t->m_shortcuts.size(); k++)
                {
                    ShortcutPath* sc = (ShortcutPath*)t->m_shortcuts.get(k);
                    Node* srcnode = (Node*)nodes.get(sc->m_src);
                    if (srcnode == 0)
                        nodes.put(sc->m_src, srcnode = new Node(sc->m_src));
                    Edge sedge(sc->m_dest, sc->m_cost);
                    srcnode->addEdge(sedge);

                    Node* destnode = (Node*)nodes.get(sc->m_dest);
                    if (destnode == 0)
                        nodes.put(sc->m_dest, destnode = new Node(sc->m_dest));
                    Edge dedge(sc->m_src, sc->m_cost);
                    destnode->addEdge(dedge);
                }
            }
        }
    }
    delete[] gtreelevel;
}

void createBorderNodes(Hash& a_nodes, Hash& a_bnodes)
{
    for (HashReader rdr(a_nodes); !rdr.isEnd(); rdr.next())
    {
        Node* node = (Node*)rdr.getVal();
        if (a_bnodes.get(node->m_id) == 0)
            a_bnodes.put(node->m_id, new BorderNode(node->m_id, node->m_x, node->m_y));
    }
}

void createShortcuts(GraphTree* a_root, Hash& a_nodes)
{
    class carrier
    {
    public:
        GraphTree*  m_root;
        Array       m_treepath;
    public:
        carrier(GraphTree* a_root): m_root(a_root)
        {};
        carrier(GraphTree* a_root, Array& a_path): m_root(a_root), m_treepath(a_path)
        {
            m_treepath.append((void*)a_root->m_graphid);
        };
        ~carrier()
        {};
    };

    Stack s;
    s.push(new carrier(a_root));
    while (!s.isEmpty())
    {
        carrier* c = (carrier*)s.pop();

        // --------------------------------------------------------------------
        // create shortcuts on border node
        // --------------------------------------------------------------------
        for (int i=0; i<c->m_root->m_shortcuts.size(); i++)
        {
            ShortcutPath* sc = (ShortcutPath*)c->m_root->m_shortcuts.get(i);
            // source node
            BorderNode* srcbnode = (BorderNode*)a_nodes.get(sc->m_src);
            Edge sedge(sc->m_dest, sc->m_cost);
            srcbnode->addEdge(c->m_treepath, sedge);
            // destination node
            BorderNode* destbnode = (BorderNode*)a_nodes.get(sc->m_dest);
            Edge dedge(sc->m_src, sc->m_cost);
            destbnode->addEdge(c->m_treepath, dedge);
            delete sc;
        }
        c->m_root->m_shortcuts.clean();

        if (c->m_root->m_children.size() > 0)
        {
            // ----------------------------------------------------------------
            // explore children
            // ----------------------------------------------------------------
            for (int i=0; i<c->m_root->m_children.size(); i++)
                s.push(new carrier(
                    (GraphTree*)c->m_root->m_children.get(i),
                    c->m_treepath));
        }
        else
        {
            // ----------------------------------------------------------------
            // explore edges
            // ----------------------------------------------------------------
            Array treepath(c->m_treepath);
            treepath.append(0);
            for (int i=0; i<c->m_root->m_nodeedges.size(); i++)
            {
                NodeEdge* e = (NodeEdge*)c->m_root->m_nodeedges.get(i);
                // source node
                BorderNode* srcbnode = (BorderNode*)a_nodes.get(e->m_src);
                Edge sedge(e->m_dest, e->m_src2dest);
                srcbnode->addEdge(treepath, sedge);
                // destination node
                BorderNode* destbnode = (BorderNode*)a_nodes.get(e->m_dest);
                Edge dedge(e->m_src, e->m_dest2src);
                destbnode->addEdge(treepath, dedge);
                delete e;
            }
            c->m_root->m_nodeedges.clean();
        }
        delete c;
    }
}

void GraphPartition::geoPartition(Hash& a_nodes, const int a_k, const int a_level,
                                  HierGraph& a_hiergraph,
                                  float& a_parttime,
                                  float& a_shorttime)
{
    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    struct timeb starttime, endtime;
    GraphTree* t;
    Hash bnodes(a_nodes.size());
    
    // ------------------------------------------------------------------------
    // create border node
    // ------------------------------------------------------------------------
    createBorderNodes(a_nodes, bnodes);

    // ------------------------------------------------------------------------
    // 1) partition a graph into a graph tree and
    // ------------------------------------------------------------------------
cerr << "start creating a hierarchical graph" << endl;
    ftime(&starttime);                  // time the algorithm
    t = partition(a_nodes, a_k, a_level);    // partitioning
    ftime(&endtime);
    a_parttime = 
        ((endtime.time*1000 + endtime.millitm) -
        (starttime.time*1000 + starttime.millitm)) / 1000.0f;
cerr << "end creating hierarchical graph" << endl;

    // ------------------------------------------------------------------------
    // compute shortcuts at each level
    // materialize all shortcuts at individual nodes
    // ------------------------------------------------------------------------
cerr << "start creating shortcuts" << endl;
    ftime(&starttime);                  // time the algorithm
    computeShortcut(t,a_level);         // determine the shortcuts
    createShortcuts(t,bnodes);
cerr << "end creating shortcuts" << endl;

    // ------------------------------------------------------------------------
    // store the border nodes to hiergraph
    // ------------------------------------------------------------------------
    for (HashReader rdr(bnodes); !rdr.isEnd(); rdr.next())
    {
        BorderNode* b = (BorderNode*)rdr.getVal();
        a_hiergraph.writeNode(rdr.getKey(), *b);
    }
    ftime(&endtime);
    a_shorttime = 
        ((endtime.time*1000 + endtime.millitm) -
        (starttime.time*1000 + starttime.millitm)) / 1000.0f;

    // ------------------------------------------------------------------------
    // clean up
    // ------------------------------------------------------------------------
    deleteGraphTree(t);
    return;
}

