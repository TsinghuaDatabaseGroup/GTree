/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "graphplot.h"
#include "graph.h"
#include "node.h"
#include "edge.h"
#include "psdraw.h"
#include "objectgen.h"

#define MAX 10000
#define MIN -MAX

void boundingbox(Graph& a_graph,
                 float& a_minx, float& a_miny,
                 float& a_maxx, float& a_maxy)
{
    a_minx = a_miny = MAX;
    a_maxx = a_maxy = MIN;
    for (HashReader rdr(a_graph.m_nodes); !rdr.isEnd(); rdr.next())
    {
        int nid = rdr.getKey();
        Node* node = a_graph.getNode(nid);

        a_minx = a_minx < node->m_x ? a_minx : node->m_x;
        a_miny = a_miny < node->m_y ? a_miny : node->m_y;
        a_maxx = a_maxx > node->m_x ? a_maxx : node->m_x;
        a_maxy = a_maxy > node->m_y ? a_maxy : node->m_y;

        delete node;
    }
}

void boundingbox(Hash& a_nodes,
                 float& a_minx, float& a_miny,
                 float& a_maxx, float& a_maxy)
{
    a_minx = a_miny = MAX;
    a_maxx = a_maxy = MIN;

    for (HashReader rdr(a_nodes); !rdr.isEnd(); rdr.next())
    {
        Node* node = (Node*)rdr.getVal();
        a_minx = a_minx < node->m_x ? a_minx : node->m_x;
        a_miny = a_miny < node->m_y ? a_miny : node->m_y;
        a_maxx = a_maxx > node->m_x ? a_maxx : node->m_x;
        a_maxy = a_maxy > node->m_y ? a_maxy : node->m_y;
    }
}

void drawgraph(PSDraw& a_draw, Graph& a_graph)
{
    for (HashReader rdr(a_graph.m_nodes); !rdr.isEnd(); rdr.next())
    {
        int nid = rdr.getKey();
        Node* node = a_graph.getNode(nid);
        for (int i=0; i<node->m_edges.size(); i++)
        {
            Edge* edge = (Edge*)node->m_edges.get(i);
            Node* dest = a_graph.getNode(edge->m_neighbor);

            a_draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, 0.5);
            delete dest;
        }
        delete node;
    }
}

//-----------------------------------------------------------------------------
// plotting a graph
//-----------------------------------------------------------------------------
void GraphPlot::plot(const char* a_filename, Graph& a_graph)
{
    float minx, miny, maxx, maxy;
    boundingbox(a_graph, minx, miny, maxx, maxy);

    PSDraw draw(a_filename, minx, miny, maxx, maxy);
    drawgraph(draw, a_graph);
}


//-----------------------------------------------------------------------------
// plotting multiple graphs
//-----------------------------------------------------------------------------
void GraphPlot::plot(const char* a_filename, Graph** a_graph, const int a_cnt)
{
    float minx=MAX, miny=MAX, maxx=MIN, maxy=MIN;
    for (int k=0; k<a_cnt; k++)
    {
        float mix, max, miy, may;
        boundingbox(*a_graph[k], mix, miy, max, may);
        minx = minx < mix ? minx : mix;
        miny = miny < miy ? miny : miy;
        maxx = maxx > max ? maxx : max;
        maxy = maxy > may ? maxy : may;
    }

    PSDraw draw(a_filename, minx, miny, maxx, maxy);

    for (int k=0; k<a_cnt; k++)
    {
        drawgraph(draw, *a_graph[k]);
        /*
        for (HashReader rdr(a_graph[k]->m_nodes); !rdr.isEnd(); rdr.next())
        {
            int nid = rdr.getKey();
            Node* node = a_graph[k]->getNode(nid);
            for (int i=0; i<node->m_edges.size(); i++)
            {
                Edge* edge = (Edge*)node->m_edges.get(i);
                Node* dest = a_graph[k]->getNode(edge->m_neighbor);

                draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, (k+1)/10.f);
                delete dest;
            }
            delete node;
        }
        */
    }
}

//-----------------------------------------------------------------------------
// plotting a graph with object and query locations
//-----------------------------------------------------------------------------
void GraphPlot::plot(const char* a_filename,
                     Hash& a_nodes,
                     Array& a_node2objects)
{
    float minx, miny, maxx, maxy;
    boundingbox(a_nodes, minx, miny, maxx, maxy);

    PSDraw draw(a_filename, minx, miny, maxx, maxy);

    for (HashReader rdr(a_nodes); !rdr.isEnd(); rdr.next())
    {
        Node* node = (Node*)rdr.getVal();
        for (int i=0; i<node->m_edges.size(); i++)
        {
            Edge* edge = (Edge*)node->m_edges.get(i);
            Node* dest = (Node*)a_nodes.get(edge->m_neighbor);
            draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, 0.7f);
        }
    }

    for (int i=0; i<a_node2objects.size(); i++)
    {
        NodeObject* nobj = (NodeObject*)a_node2objects.get(i);
        Node* node = (Node*)a_nodes.get(nobj->m_nodeid);
        draw.box(node->m_x-0.05f, node->m_y-0.05f, node->m_x+0.05f, node->m_y+0.05f, 0.3f);
    }
}

//-----------------------------------------------------------------------------
// plotting a graph with object and query locations
//-----------------------------------------------------------------------------
void GraphPlot::plot(const char* a_filename,
                     Hash& a_nodes,
                     Array& a_node2objects,
                     Array& a_node2queries)
{
    float minx, miny, maxx, maxy;
    boundingbox(a_nodes, minx, miny, maxx, maxy);

    PSDraw draw(a_filename, minx, miny, maxx, maxy);

    for (HashReader rdr(a_nodes); !rdr.isEnd(); rdr.next())
    {
        Node* node = (Node*)rdr.getVal();
        for (int i=0; i<node->m_edges.size(); i++)
        {
            Edge* edge = (Edge*)node->m_edges.get(i);
            Node* dest = (Node*)a_nodes.get(edge->m_neighbor);
            draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, 0.7f);
        }
    }

    for (int i=0; i<a_node2objects.size(); i++)
    {
        NodeObject* nobj = (NodeObject*)a_node2objects.get(i);
        Node* node = (Node*)a_nodes.get(nobj->m_nodeid);
        draw.box(node->m_x-0.05f, node->m_y-0.05f, node->m_x+0.05f, node->m_y+0.05f, 0.3f);
    }

    for (int i=0; i<a_node2queries.size(); i++)
    {
        NodeObject* nqry = (NodeObject*)a_node2queries.get(i);
        Node* node = (Node*)a_nodes.get(nqry->m_nodeid);
        draw.solidbox(node->m_x-0.05f, node->m_y-0.05f, node->m_x+0.05f, node->m_y+0.05f, 0.3f);
    }
}

void GraphPlot::plot(const char* a_filename, Graph& a_graph,
                     Array& src1, Array& dest1,
                     Array& src2, Array& dest2,
                     const int a_src, const int a_dest)
{
    float minx, miny, maxx, maxy;
    boundingbox(a_graph, minx, miny, maxx, maxy);

    PSDraw draw(a_filename, minx, miny, maxx, maxy);

    for (int i=0; i<src1.size(); i++)
    {
        int src = (long)src1.get(i);
        int dest= (long)dest1.get(i);

        Node* srcnode = a_graph.getNode(src);
        Node* destnode = a_graph.getNode(dest);

        draw.line(srcnode->m_x, srcnode->m_y, destnode->m_x, destnode->m_y, 0.3f);

        delete srcnode;
        delete destnode;
    }

    for (int i=0; i<src2.size(); i++)
    {
        int src = (long)src2.get(i);
        int dest= (long)dest2.get(i);

        Node* srcnode = a_graph.getNode(src);
        Node* destnode = a_graph.getNode(dest);

        draw.line(srcnode->m_x, srcnode->m_y, destnode->m_x, destnode->m_y, 0.7f);

        delete srcnode;
        delete destnode;
    }

    Node* srcnode = a_graph.getNode(a_src);
    Node* destnode= a_graph.getNode(a_dest);
    draw.box(srcnode->m_x-0.05f, srcnode->m_y-0.05f, srcnode->m_x+0.05f, srcnode->m_y+0.05f, 0.2f);
    draw.box(destnode->m_x-0.05f, destnode->m_y-0.05f, destnode->m_x+0.05f, destnode->m_y+0.05f, 0.6f);
    delete srcnode;
    delete destnode;
}

void GraphPlot::plot(const char* a_filename, Graph& a_graph,
                     const int a_querynode, Array& a_objnode,
                     Array& a_visitednode)
{
    plot(a_filename, a_graph, &a_querynode, 1, a_objnode, a_visitednode);
}

void GraphPlot::plot(const char* a_filename, Graph& a_graph,
                     const int* a_querynode, const int a_sz,
                     Array& a_objnode, Array& a_visitednode)
{
    float minx, miny, maxx, maxy;
    boundingbox(a_graph, minx, miny, maxx, maxy);

    PSDraw draw(a_filename, minx, miny, maxx, maxy);

    a_visitednode.removeDuplicate();
    for (HashReader rdr(a_graph.m_nodes); !rdr.isEnd(); rdr.next())
    {
        int nid = rdr.getKey();
        bool found = (a_visitednode.binSearch((void*)nid) != -1);
        Node* node = a_graph.getNode(nid);
        for (int i=0; i<node->m_edges.size(); i++)
        {
            Edge* edge = (Edge*)node->m_edges.get(i);
            Node* dest = a_graph.getNode(edge->m_neighbor);
            if (found && a_visitednode.binSearch((void*)edge->m_neighbor) != -1)
                draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, 0.8f, 0, 0, 3);
                //draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, 0.2, 2);
            else
                draw.line(node->m_x, node->m_y, dest->m_x, dest->m_y, 0.3f, 0);
            delete dest;
        }
        delete node;
    }

    for (int i=0; i<a_visitednode.size(); i++)
    {
        int path = (long)a_visitednode.get(i);
        Node* node = a_graph.getNode(path);
        draw.box(node->m_x-0.06f, node->m_y-0.06f, node->m_x+0.06f, node->m_y+0.06f, 0.8f, 0, 0);
        delete node;
    }

    for (int i=0; i<a_objnode.size(); i++)
    {
        int obj = (long)a_objnode.get(i);
        Node* node = a_graph.getNode(obj);
        draw.box(node->m_x-0.1f, node->m_y-0.1f, node->m_x+0.1f, node->m_y+0.1f, 0, 0, 1);
        delete node;
    }

    for (int i=0; i<a_sz; i++)
    {
        Node* querynode = a_graph.getNode(a_querynode[i]);
        draw.box(querynode->m_x-0.12f, querynode->m_y-0.12f, querynode->m_x+0.12f, querynode->m_y+0.12f, 0, 0, 0);
        delete querynode;
    }

    return;
}
