/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2008
-----------------------------------------------------------------------------*/
#include "distbrwssearch.h"
#include "graph.h"
#include "node.h"
#include "distbrws.h"
#include "spqdtree.h"
#include "bound.h"
#include "point.h"
#include <stdio.h>

    class nodeobj
    {
    public:
        const int   m_objid;    // object id
        const int   m_nodeid;   // node id (that contains the object)
        const Point m_pt;       // coordinate of the node
        float       m_accdist;  // accumulated distance
        float       m_preddist; // predicted distance (remainder part)
        int         m_nextnode; // next node to be visited
        Array       m_path;     // path towards the objects
    public:
        nodeobj(
            const int a_objid, const int a_nodeid,
            const Point& a_pt, 
            const float a_accdist, const float a_preddist,
            const int a_nextnode):
          m_objid(a_objid),
          m_nodeid(a_nodeid),
          m_pt(a_pt),
          m_accdist(a_accdist),
          m_preddist(a_preddist),
          m_nextnode(a_nextnode)
        {};
        ~nodeobj(){};
        static int compare(const void* a0, const void* a1)
        {
            nodeobj* no0 = *(nodeobj**)a0;
            nodeobj* no1 = *(nodeobj**)a1;
            const float dist0 = no0->m_accdist + no0->m_preddist;
            const float dist1 = no1->m_accdist + no1->m_preddist;
            if (dist0 < dist1) return -1;
            if (dist0 > dist1) return +1;
            if (no0->m_nextnode < no1->m_nextnode) return -1;
            if (no0->m_nextnode > no1->m_nextnode) return +1;
            if (no0->m_objid < no1->m_objid) return -1;
            if (no0->m_objid > no1->m_objid) return +1;
            return 0;
        };
    };

void DistBrwsSearch::rangeSearch(Graph &a_graph,
								 DistBrws &a_distbrws, const Bound& a_bound,
								 const Array& a_nodes,
								 const int a_src, const float a_range,
								 Array &a_result,
								 int &a_nodeaccess, int &a_edgeaccess)
{
    Queue q;

    SPQuadtree* tree = a_distbrws.getNode(a_src, a_bound);
    for (int i=0; i<a_nodes.size(); i++)
    {
        int nodeid = (long)a_nodes.get(i);
        Node* node = a_graph.getNode(nodeid);
        float p[2];
        p[0] = node->m_x;
        p[1] = node->m_y;
        Point pt(2,p);
        const int nextnode = tree->next(pt);
        const float preddist = tree->mindist(pt);
        q.enqueue(new nodeobj(i, nodeid, pt, 0, preddist, nextnode));
        delete node;
    }
    delete tree;

    while (!q.isEmpty())
    {
        nodeobj* no = (nodeobj*)q.dequeue();

        //---------------------------------------------------------------------
        // clean up
        //---------------------------------------------------------------------
		if (no->m_accdist + no->m_preddist >= a_range)
        {
            delete no;
            continue;
        }


        //---------------------------------------------------------------------
        // exploring the path towards the object
        //---------------------------------------------------------------------
        if (no->m_nextnode == no->m_nodeid)
        {
            // found the result objects
            ObjectSearchResult* res =
                new ObjectSearchResult(no->m_nodeid, no->m_path, no->m_accdist);
            res->m_objects.append((void*)no->m_objid);
            a_result.append((void*)res);
            delete no;
        }
        else
        {
            Node* node = a_graph.getNode(no->m_nextnode);
            SPQuadtree* tree = a_distbrws.getNode(no->m_nextnode, a_bound);
            no->m_path.append((void*)no->m_nextnode);
            no->m_preddist = tree->mindist(no->m_pt);
            no->m_nextnode = tree->next(no->m_pt);
            no->m_accdist += node->cost(no->m_nextnode);

            q.enqueue(no);

            delete node;
            delete tree;

            a_nodeaccess++;
            a_edgeaccess++;
        }
    }
}

void DistBrwsSearch::nnSearch(Graph &a_graph,
                              DistBrws &a_distbrws, const Bound& a_bound,
                              const Array& nodes, 
                              const int a_src, const int a_k,
                              Array &a_result,
                              int &a_nodeaccess, int &a_edgeaccess)
{

    BinHeap h(nodeobj::compare);
	

    SPQuadtree* tree = a_distbrws.getNode(a_src, a_bound);
    for (int i=0; i<nodes.size(); i++)
    {
        int nodeid = (long)nodes.get(i);
        Node* node = a_graph.getNode(nodeid);
        float p[2];
        p[0] = node->m_x;
        p[1] = node->m_y;
        Point pt(2,p);
        const int nextnode = tree->next(pt);
        const float preddist = tree->mindist(pt);
//		printf("no=%d next=%d\n", i, nextnode );
        h.insert(new nodeobj(i, nodeid, pt, 0, preddist, nextnode));
        delete node;
    }
    delete tree;


    while (!h.isEmpty())
    {
        nodeobj* no = (nodeobj*)h.removeTop();

		
        //---------------------------------------------------------------------
        // clean up
        //---------------------------------------------------------------------
        if (a_result.size() >= a_k)
        {
            delete no;
            continue;
        }
		// printf("%d %d %f %f %d\n", no->m_objid, no->m_nodeid, no->m_accdist, no->m_preddist, no->m_nextnode );
        //---------------------------------------------------------------------
        // exploring the path towards the object
        //---------------------------------------------------------------------
        if (no->m_nextnode == no->m_nodeid)
        {
            // found the result objects
            ObjectSearchResult* res =
                new ObjectSearchResult(no->m_nodeid, no->m_path, no->m_accdist);
            res->m_objects.append((void*)no->m_objid);
            a_result.append((void*)res);
            delete no;
        }
        else
        {
            Node* node = a_graph.getNode(no->m_nextnode);
            SPQuadtree* tree = a_distbrws.getNode(no->m_nextnode, a_bound);
            no->m_path.append((void*)no->m_nextnode);
            no->m_preddist = tree->mindist(no->m_pt);
            no->m_nextnode = tree->next(no->m_pt);
            no->m_accdist += node->cost(no->m_nextnode);

            h.insert(no);

            delete node;
            delete tree;

            a_nodeaccess++;
            a_edgeaccess++;
        }
    }
}
