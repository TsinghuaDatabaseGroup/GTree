/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
---------------------------------------------------------------------------- */

#include "spatialsearch.h"
#include "spatialmap.h"
#include "graph.h"
#include "node.h"
#include "graphsearch.h"

//-----------------------------------------------------------------------------
// single-point range search
//-----------------------------------------------------------------------------
void SpatialObjectSearch::rangeSearch(Graph& a_graph, SpatialMapping& a_smap,
                                      const int a_src, const float a_range,
                                      Array& a_result, int& a_nodeaccess, int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    Node* srcnode = (Node*)a_graph.getNode(a_src);
    float x = srcnode->m_x;
    float y = srcnode->m_y;
    delete srcnode;

    //-------------------------------------------------------------------------
    // find candidates
    //-------------------------------------------------------------------------
    const Array* cand = a_smap.distOrder(x,y);
    
    //-------------------------------------------------------------------------
    // examine the true network distance of individual candidates
    // if the Eculidean distance exceeds the range, quit!
    //-------------------------------------------------------------------------
    for (int i=0; i<cand->size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)cand->get(i);
        float cost = c->distance(x,y);
        if (cost > a_range) break;  // terminate

        //---------------------------------------------------------------------
        // use A* algorithm to determine the true distance
        //---------------------------------------------------------------------
        int na=0, ea=0;
        GraphSearchResult* gsr = GraphSearch::aStar(a_graph, a_src, c->m_nid, na, ea);
        a_nodeaccess += na;
        a_edgeaccess += ea;
        if (gsr->m_cost < a_range)
        {
            //-----------------------------------------------------------------
            // keep the candidate as final result
            //-----------------------------------------------------------------
            ObjectSearchResult* r =
                new ObjectSearchResult(c->m_nid, gsr->m_path, gsr->m_cost);
            r->m_objects.append((void*)c->m_oid);
            a_result.append(r);
        }
        delete gsr;
    }

    return;
}

//-----------------------------------------------------------------------------
// single-point kNN search
//-----------------------------------------------------------------------------
void SpatialObjectSearch::kNNSearch(Graph& a_graph, SpatialMapping& a_smap,
                                    const int a_src, const int a_k,
                                    Array& a_result, int& a_nodeaccess, int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    float distance=10000000;
    Node* srcnode = (Node*)a_graph.getNode(a_src);
    float x = srcnode->m_x;
    float y = srcnode->m_y;
    delete srcnode;

    //-------------------------------------------------------------------------
    // find candidates
    //-------------------------------------------------------------------------
    const Array* cand = a_smap.distOrder(x,y);
    
    //-------------------------------------------------------------------------
    // examine the true network distance of individual candidates
    // if the Eculidean distance exceeds the range, quit!
    //-------------------------------------------------------------------------
    for (int i=0; i<cand->size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)cand->get(i);
        float dist = c->distance(x,y);
        if (dist > distance) break;  // terminate

        //---------------------------------------------------------------------
        // use A* algorithm to determine the true distance
        //---------------------------------------------------------------------
        int na=0, ea=0;
        GraphSearchResult* gsr =
            GraphSearch::aStar(a_graph, a_src, c->m_nid, na, ea);
        a_nodeaccess += na;
        a_edgeaccess += ea;

        //---------------------------------------------------------------------
        // keep the candidate as final result
        //---------------------------------------------------------------------
        ObjectSearchResult* r =
            new ObjectSearchResult(c->m_nid, gsr->m_path, gsr->m_cost);

        r->m_objects.append((void*)c->m_oid);
        a_result.append(r);
        a_result.sort(ObjectSearchResult::compare);
        if (a_result.size() > a_k)
            distance = ((ObjectSearchResult*)a_result.get(a_k-1))->m_cost;

        delete gsr;
    }

    return;
}



//-----------------------------------------------------------------------------
// single-point kNN search with trace
//-----------------------------------------------------------------------------
void SpatialObjectSearch::kNNSearch(Graph& a_graph, SpatialMapping& a_smap,
                                    const int a_src, const int a_k,
                                    Array& a_result, int& a_nodeaccess, int& a_edgeaccess,
                                    Array& a_visited)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    float distance=10000000;
    Node* srcnode = (Node*)a_graph.getNode(a_src);
    float x = srcnode->m_x;
    float y = srcnode->m_y;
    delete srcnode;

    //-------------------------------------------------------------------------
    // find candidates
    //-------------------------------------------------------------------------
    const Array* cand = a_smap.distOrder(x,y);
    
    //-------------------------------------------------------------------------
    // examine the true network distance of individual candidates
    // if the Eculidean distance exceeds the range, quit!
    //-------------------------------------------------------------------------
    for (int i=0; i<cand->size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)cand->get(i);
        float dist = c->distance(x,y);
        if (dist > distance) break;  // terminate

        //---------------------------------------------------------------------
        // use A* algorithm to determine the true distance
        //---------------------------------------------------------------------
        int na=0, ea=0;
        GraphSearchResult* gsr =
            GraphSearch::aStar(a_graph, a_src, c->m_nid, na, ea, a_visited);
        a_nodeaccess += na;
        a_edgeaccess += ea;

        //---------------------------------------------------------------------
        // keep the candidate as final result
        //---------------------------------------------------------------------
        ObjectSearchResult* r =
            new ObjectSearchResult(c->m_nid, gsr->m_path, gsr->m_cost);

        r->m_objects.append((void*)c->m_oid);
        a_result.append(r);
        a_result.sort(ObjectSearchResult::compare);
        if (a_result.size() > a_k)
            distance = ((ObjectSearchResult*)a_result.get(a_k-1))->m_cost;

        delete gsr;
    }

    return;
}

//-----------------------------------------------------------------------------
// multi-point range search
//-----------------------------------------------------------------------------
void SpatialObjectSearch::groupRangeSearch(Graph& a_graph, SpatialMapping& a_smap,
                                           const int* a_src, const float* a_range,
                                           const int a_cnt,
                                           Array& a_result,
                                           int& a_nodeaccess, int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    float x[MAXQUERY], y[MAXQUERY];

    for (int i=0; i<a_cnt; i++)
    {
        Node* srcnode = (Node*)a_graph.getNode(a_src[i]);
        x[i] = srcnode->m_x;
        y[i] = srcnode->m_y;
        delete srcnode;
    }

    //-------------------------------------------------------------------------
    // find candidates
    //-------------------------------------------------------------------------
    const Array* cand = a_smap.distOrder(x,y,a_cnt);
    
    //-------------------------------------------------------------------------
    // examine the true network distance of individual candidates
    // if the Eculidean distance exceeds the range, quit!
    //-------------------------------------------------------------------------
    for (int i=0; i<cand->size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)cand->get(i);
        float cost = c->distance(x,y,a_cnt);

        bool covered = true;
        for (int j=0; j<a_cnt && covered; j++)
            covered = c->distance(x[j],y[j]) <= a_range[j];
        
        if (!covered) continue;

        //---------------------------------------------------------------------
        // use A* algorithm to determine the true distances from each src
        //---------------------------------------------------------------------
        bool satisfied = true;
        int na=0, ea=0;
        GroupObjectSearchResult* r = new GroupObjectSearchResult(c->m_nid, c->m_oid, a_cnt);
        for (int j=0; j<a_cnt; j++)
        {
            GraphSearchResult* gsr = GraphSearch::aStar(a_graph, a_src[j], c->m_nid, na, ea);
            a_nodeaccess += na;
            a_edgeaccess += ea;
            if (gsr->m_cost > a_range[j])
            {
                satisfied = false;
                delete gsr;
                break;
            }
            r->m_cost[j] = gsr->m_cost;
            r->m_path[j].copy(gsr->m_path);
            delete gsr;
        }

        if (satisfied)
            a_result.append(r);
        else
            delete r;
    }
    return;
}

//-----------------------------------------------------------------------------
// multi-point kNN search
//-----------------------------------------------------------------------------
void SpatialObjectSearch::groupKNNSearch(Graph& a_graph, SpatialMapping& a_smap,
                                         const int* a_src, const int a_cnt,
                                         const int a_k,
                                         Array& a_result,
                                         int& a_nodeaccess, int& a_edgeaccess)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    float maxdist = 1000000;
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    float x[MAXQUERY], y[MAXQUERY];

    for (int i=0; i<a_cnt; i++)
    {
        Node* srcnode = (Node*)a_graph.getNode(a_src[i]);
        x[i] = srcnode->m_x;
        y[i] = srcnode->m_y;
        delete srcnode;
    }

    //-------------------------------------------------------------------------
    // find candidates
    //-------------------------------------------------------------------------
    const Array* cand = a_smap.distOrder(x,y,a_cnt);
    
    //-------------------------------------------------------------------------
    // examine the true network distance of individual candidates
    // if the Eculidean distance exceeds the range, quit!
    //-------------------------------------------------------------------------
    for (int i=0; i<cand->size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)cand->get(i);
        float cost = c->distance(x,y,a_cnt);

        if (cost > maxdist) break;

        //---------------------------------------------------------------------
        // use A* algorithm to determine the true distances from each src
        //---------------------------------------------------------------------
        int na=0, ea=0;
        GroupObjectSearchResult* r = new GroupObjectSearchResult(c->m_nid, c->m_oid, a_cnt);
        for (int j=0; j<a_cnt; j++)
        {
            GraphSearchResult* gsr = GraphSearch::aStar(a_graph, a_src[j], c->m_nid, na, ea);
            a_nodeaccess += na;
            a_edgeaccess += ea;
            r->m_cost[j] = gsr->m_cost;
            r->m_path[j].copy(gsr->m_path);
            delete gsr;
        }
        a_result.append(r);
        if (a_result.size() >= a_k)
        {
            a_result.sort(GroupObjectSearchResult::compare);
            maxdist = ((GroupObjectSearchResult*)a_result.get(a_k-1))->sumcost();
        }
    }
    return;
}

void SpatialObjectSearch::groupKNNSearch(Graph& a_graph, SpatialMapping& a_smap,
                                         const int* a_src, const int a_cnt,
                                         const int a_k,
                                         Array& a_result,
                                         int& a_nodeaccess, int& a_edgeaccess,
                                         Array& a_visited)
{
    //-------------------------------------------------------------------------
    // initialization
    //-------------------------------------------------------------------------
    float maxdist = 1000000;
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    float x[MAXQUERY], y[MAXQUERY];

    for (int i=0; i<a_cnt; i++)
    {
        Node* srcnode = (Node*)a_graph.getNode(a_src[i]);
        x[i] = srcnode->m_x;
        y[i] = srcnode->m_y;
        delete srcnode;
    }

    //-------------------------------------------------------------------------
    // find candidates
    //-------------------------------------------------------------------------
    const Array* cand = a_smap.distOrder(x,y,a_cnt);
    
    //-------------------------------------------------------------------------
    // examine the true network distance of individual candidates
    // if the Eculidean distance exceeds the range, quit!
    //-------------------------------------------------------------------------
    for (int i=0; i<cand->size(); i++)
    {
        ObjectCoor* c = (ObjectCoor*)cand->get(i);
        float cost = c->distance(x,y,a_cnt);

        if (cost > maxdist) break;

        //---------------------------------------------------------------------
        // use A* algorithm to determine the true distances from each src
        //---------------------------------------------------------------------
        int na=0, ea=0;
        GroupObjectSearchResult* r = new GroupObjectSearchResult(c->m_nid, c->m_oid, a_cnt);
        for (int j=0; j<a_cnt; j++)
        {
            GraphSearchResult* gsr = GraphSearch::aStar(a_graph, a_src[j], c->m_nid, na, ea, a_visited);
            a_nodeaccess += na;
            a_edgeaccess += ea;
            r->m_cost[j] = gsr->m_cost;
            r->m_path[j].copy(gsr->m_path);
            delete gsr;
        }
        a_result.append(r);
        if (a_result.size() >= a_k)
        {
            a_result.sort(GroupObjectSearchResult::compare);
            maxdist = ((GroupObjectSearchResult*)a_result.get(a_k-1))->sumcost();
        }
    }
    return;
}
