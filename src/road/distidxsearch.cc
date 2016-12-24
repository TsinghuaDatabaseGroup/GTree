/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Jan, 2008
-----------------------------------------------------------------------------*/
#include "distidxsearch.h"
#include "distidx.h"
#include "distsign.h"
#include "graph.h"
#include "node.h"
#include "edge.h"


// ----------------------------------------------------------------------------
// single point range search
// ----------------------------------------------------------------------------
void DistIndexSearch::rangeSearch(Graph& a_graph, DistIndex& a_distidx,
                                   const int a_src, const float a_range,
                                   Array& a_result,
                                   int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // local carrier for ordered node exploring
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_nid;
        const int   m_oid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_nid, const float a_cost, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        carrier(const int a_nid, const float a_cost, Array& a_path, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_path(a_path), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_oid < c1->m_oid) return -1;
            if (c0->m_oid > c1->m_oid) return +1;
            return 0;
        };
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    Set visited(1000);

    BinHeap heap(carrier::compare);

    // ------------------------------------------------------------------------
    // result candidates
    // ------------------------------------------------------------------------
    Array* a = a_distidx.getNode(a_src);  // ordered in IDs
    Array sorta(*a);
    sorta.sort(DistSignature::compare);
    for (int i=0; i<sorta.size(); i++)
    {
        DistSignature* distsign = (DistSignature*)sorta.get(i);
        if (distsign->m_cost > a_range) break;
        heap.insert(new carrier(a_src, 0, distsign->m_oid));
    }
    for (int i=0; i<a->size(); i++)
        delete (DistSignature*)a->get(i);
    delete a;

    // ------------------------------------------------------------------------
    // network expansion
    // ------------------------------------------------------------------------
    while (!heap.isEmpty())
    {
        carrier* c = (carrier*)heap.removeTop();

        // --------------------------------------------------------------------
        // examine distance signature for next hop
        // --------------------------------------------------------------------
        const Array* a = a_distidx.getNode(c->m_nid);
        DistSignature d(c->m_oid,0,0);
        const int i = a->binSearch(&d, DistSignature::compareID);
        if (i >= 0)
        {
            DistSignature* distsign = (DistSignature*)a->get(i);
            if (distsign->m_cost > (a_range - c->m_cost)) break;
            if (distsign->m_oid == c->m_oid)
            {
                // ------------------------------------------------------------
                // terminate the exploring for an object if the object is found
                // ------------------------------------------------------------
                if (distsign->m_cost == 0)
                {
                    ObjectSearchResult* res =
                        new ObjectSearchResult(c->m_nid, c->m_path, c->m_cost);
                    res->m_objects.append((void*)c->m_oid);
                    a_result.append((void*)res);
                }
                // ------------------------------------------------------------
                // continue the exploring
                // ------------------------------------------------------------
                else
                {
                    if (!visited.in((void*)c->m_nid))
                    {
                        a_nodeaccess++;
                        visited.insert((void*)c->m_nid);
                    }

                    Node* node = (Node*)a_graph.getNode(c->m_nid);
                    float cost = 0;
                    for (int i=0; i<node->m_edges.size(); i++)
                    {
                        Edge* e = (Edge*)node->m_edges.get(i);
                        if (e->m_neighbor == distsign->m_prev)
                        {
                            cost = e->m_cost;
                            a_edgeaccess++;
                            break;
                        }
                    }
                    heap.insert(
                        new carrier(distsign->m_prev, c->m_cost + cost,
                        c->m_path, distsign->m_oid));
                    delete node;
                }
            }
        }

        // --------------------------------------------------------------------
        // clean up
        // --------------------------------------------------------------------
        for (int i=0; i<a->size(); i++)
            delete (DistSignature*)a->get(i);
        delete a;
        delete c;
    }
}

// ----------------------------------------------------------------------------
// single point kNN search
// ----------------------------------------------------------------------------
void DistIndexSearch::kNNSearch(Graph& a_graph, DistIndex& a_distidx,
                                const int a_src, const int a_k,
                                Array& a_result,
                                int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // local carrier for ordered node exploring
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_nid;
        const int   m_oid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_nid, const float a_cost, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        carrier(const int a_nid, const float a_cost, Array& a_path, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_path(a_path), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_oid < c1->m_oid) return -1;
            if (c0->m_oid > c1->m_oid) return +1;
            return 0;
        };
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    Set visited(1000);

    BinHeap heap(carrier::compare);

    // ------------------------------------------------------------------------
    // result candidates
    // ------------------------------------------------------------------------
    Array* a = a_distidx.getNode(a_src);
    Array sorta(*a);
    sorta.sort(DistSignature::compare);
    for (int i=0; i<sorta.size(); i++)
    {
        if (i == a_k) break;
        DistSignature* distsign = (DistSignature*)sorta.get(i);
        heap.insert(new carrier(a_src, 0, distsign->m_oid));
    }
    for (int i=0; i<a->size(); i++)
        delete (DistSignature*)a->get(i);
    delete a;

    // ------------------------------------------------------------------------
    // network expansion
    // ------------------------------------------------------------------------
    while (!heap.isEmpty())
    {
        carrier* c = (carrier*)heap.removeTop();

        // --------------------------------------------------------------------
        // examine distance signature for next hop
        // --------------------------------------------------------------------
        Array* a = a_distidx.getNode(c->m_nid);
        DistSignature d(c->m_oid,0,0);
        const int i = a->binSearch(&d, DistSignature::compareID);
        if (i >= 0)
        {
            DistSignature* distsign = (DistSignature*)a->get(i);
            if (distsign->m_oid == c->m_oid)
            {
                // ------------------------------------------------------------
                // terminate the exploring for an object if the object is found
                // ------------------------------------------------------------
                if (distsign->m_cost == 0)
                {
                    ObjectSearchResult* res =
                        new ObjectSearchResult(c->m_nid, c->m_path, c->m_cost);
                    res->m_objects.append((void*)c->m_oid);
                    a_result.append((void*)res);
                }
                // ------------------------------------------------------------
                // continue the exploring
                // ------------------------------------------------------------
                else
                {
                    if (!visited.in((void*)c->m_nid))
                    {
                        a_nodeaccess++;
                        visited.insert((void*)c->m_nid);
                    }

                    Node* node = (Node*)a_graph.getNode(c->m_nid);
                    float cost = 0;
                    for (int i=0; i<node->m_edges.size(); i++)
                    {
                        Edge* e = (Edge*)node->m_edges.get(i);
                        if (e->m_neighbor == distsign->m_prev)
                        {
                            cost = e->m_cost;
                            a_edgeaccess++;
                            break;
                        }
                    }
                    heap.insert(
                        new carrier(distsign->m_prev, c->m_cost + cost,
                        c->m_path, distsign->m_oid));
                    delete node;
                }
            }
        }

        // --------------------------------------------------------------------
        // clean up
        // --------------------------------------------------------------------
        for (int i=0; i<a->size(); i++)
            delete (DistSignature*)a->get(i);
        delete a;
        delete c;
    }
}


// ----------------------------------------------------------------------------
// single point kNN search
// ----------------------------------------------------------------------------
void DistIndexSearch::kNNSearch(Graph& a_graph, DistIndex& a_distidx,
                                const int a_src, const int a_k,
                                Array& a_result,
                                int& a_nodeaccess, int& a_edgeaccess,
                                Array& a_visited)
{
    // ------------------------------------------------------------------------
    // local carrier for ordered node exploring
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_nid;
        const int   m_oid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_nid, const float a_cost, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        carrier(const int a_nid, const float a_cost, Array& a_path, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_path(a_path), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_oid < c1->m_oid) return -1;
            if (c0->m_oid > c1->m_oid) return +1;
            return 0;
        };
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;
    Set visited(1000);

    BinHeap heap(carrier::compare);

    // ------------------------------------------------------------------------
    // result candidates
    // ------------------------------------------------------------------------
    Array* a = a_distidx.getNode(a_src);
    Array sorta(*a);
    sorta.sort(DistSignature::compare);
    for (int i=0; i<sorta.size(); i++)
    {
        if (i == a_k) break;
        DistSignature* distsign = (DistSignature*)sorta.get(i);
        heap.insert(new carrier(a_src, 0, distsign->m_oid));
    }
    for (int i=0; i<a->size(); i++)
        delete (DistSignature*)a->get(i);
    delete a;

    // ------------------------------------------------------------------------
    // network expansion
    // ------------------------------------------------------------------------
    while (!heap.isEmpty())
    {
        carrier* c = (carrier*)heap.removeTop();

        // --------------------------------------------------------------------
        // examine distance signature for next hop
        // --------------------------------------------------------------------
        Array* a = a_distidx.getNode(c->m_nid);
        DistSignature d(c->m_oid,0,0);
        const int i = a->binSearch(&d, DistSignature::compareID);
        if (i >= 0)
        {
            DistSignature* distsign = (DistSignature*)a->get(i);
            if (distsign->m_oid == c->m_oid)
            {
                // ------------------------------------------------------------
                // terminate the exploring for an object if the object is found
                // ------------------------------------------------------------
                if (distsign->m_cost == 0)
                {
                    ObjectSearchResult* res =
                        new ObjectSearchResult(c->m_nid, c->m_path, c->m_cost);
                    res->m_objects.append((void*)c->m_oid);
                    a_result.append((void*)res);
                }
                // ------------------------------------------------------------
                // continue the exploring
                // ------------------------------------------------------------
                else
                {
                    if (!visited.in((void*)c->m_nid))
                    {
                        a_nodeaccess++;
                        visited.insert((void*)c->m_nid);
                        a_visited.append((void*)c->m_nid);
                    }

                    Node* node = (Node*)a_graph.getNode(c->m_nid);
                    float cost = 0;
                    for (int i=0; i<node->m_edges.size(); i++)
                    {
                        Edge* e = (Edge*)node->m_edges.get(i);
                        if (e->m_neighbor == distsign->m_prev)
                        {
                            cost = e->m_cost;
                            a_edgeaccess++;
                            break;
                        }
                    }
                    heap.insert(
                        new carrier(distsign->m_prev, c->m_cost + cost,
                        c->m_path, distsign->m_oid));
                    delete node;
                }
            }
        }

        // --------------------------------------------------------------------
        // clean up
        // --------------------------------------------------------------------
        for (int i=0; i<a->size(); i++)
            delete (DistSignature*)a->get(i);
        delete a;
        delete c;
    }
}

// ----------------------------------------------------------------------------
// multi-point range search
// ----------------------------------------------------------------------------
void DistIndexSearch::groupRangeSearch(Graph& a_graph, DistIndex& a_distidx,
                                       const int* a_src, const float* a_range,
                                       const int a_cnt,
                                       Array& a_result, int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // local carrier for ordered node exploring
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_nid;
        const int   m_oid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_nid, const float a_cost, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        carrier(const int a_nid, const float a_cost, Array& a_path, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_path(a_path), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_oid < c1->m_oid) return -1;
            if (c0->m_oid > c1->m_oid) return +1;
            return 0;
        };
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // collect result candidates
    // ------------------------------------------------------------------------
    Hash cand;
    for (int i=0; i<a_cnt; i++)
    {
        Array* a = a_distidx.getNode(a_src[i]);
        Array sorta(*a);
        sorta.sort(DistSignature::compare);
        for (int j=0; j<sorta.size(); j++)
        {
            DistSignature* distsign = (DistSignature*)sorta.get(j);
            if (distsign->m_cost > a_range[i]) break;
            GroupObjectSearchResult* r =
                (GroupObjectSearchResult*)cand.get(distsign->m_oid);
            if (r == 0)
                cand.put(distsign->m_oid, r =
                new GroupObjectSearchResult(-1,distsign->m_oid,a_cnt));
            r->m_cost[i] = distsign->m_cost;
        }
        for (int j=0; j<a->size(); j++)
            delete (DistSignature*)a->get(j);
        delete a;
    }

    // ------------------------------------------------------------------------
    // filter candidates
    // ------------------------------------------------------------------------
    Array falsehit;
    for (HashReader rdr(cand); !rdr.isEnd(); rdr.next())
    {
        GroupObjectSearchResult* r = (GroupObjectSearchResult*)rdr.getVal();
        if (r->allreached(a_cnt))
            a_result.append(r);
        else
            falsehit.append(r);
    }

    // ------------------------------------------------------------------------
    // network expansion
    // ------------------------------------------------------------------------
    for (int i=0; i<a_cnt; i++)
    {
        Set visited(1000);
        BinHeap heap(carrier::compare);
        for (int j=0; j<a_result.size(); j++)
        {
            GroupObjectSearchResult* r =
                (GroupObjectSearchResult*)a_result.get(j);
            heap.insert(new carrier(a_src[i], r->m_cost[i], r->m_oid));
        }

        // --------------------------------------------------------------------
        // network expansion
        // --------------------------------------------------------------------
        while (!heap.isEmpty())
        {
            carrier* c = (carrier*)heap.removeTop();

            // ----------------------------------------------------------------
            // examine distance signature for next hop
            // ----------------------------------------------------------------
            Array* a = a_distidx.getNode(c->m_nid);
            DistSignature d(c->m_oid, 0, 0);
            const int k = a->binSearch(&d, DistSignature::compareID);
            // for (int k=0; k<a->size(); k++)
            if (k >= 0)
            {
                DistSignature* distsign = (DistSignature*)a->get(k);
                if (distsign->m_oid == c->m_oid)
                {
                    // --------------------------------------------------------
                    // terminate the exploring for an object if it is found
                    // --------------------------------------------------------
                    if (distsign->m_cost == 0)
                    {
                        GroupObjectSearchResult* r =
                            (GroupObjectSearchResult*)cand.get(c->m_oid);
                        r->m_nid = c->m_nid;
                        r->m_path[i].copy(c->m_path);   // record the path
                    }
                    // --------------------------------------------------------
                    // continue the exploring
                    // --------------------------------------------------------
                    else
                    {
                        if (!visited.in((void*)c->m_nid))
                        {
                            a_nodeaccess++;
                            visited.insert((void*)c->m_nid);
                        }

                        Node* node = (Node*)a_graph.getNode(c->m_nid);
                        float cost = 0;
                        for (int l=0; l<node->m_edges.size(); l++)
                        {
                            Edge* e = (Edge*)node->m_edges.get(l);
                            if (e->m_neighbor == distsign->m_prev)
                            {
                                cost = e->m_cost;
                                a_edgeaccess++;
                                break;
                            }
                        }
                        heap.insert(
                            new carrier(distsign->m_prev, c->m_cost + cost,
                            c->m_path, distsign->m_oid));
                        delete node;
                    }
                }
            }
            // ----------------------------------------------------------------
            // clean up
            // ----------------------------------------------------------------
            for (int d=0; d<a->size(); d++)
                delete (DistSignature*)a->get(d);
            delete a;
            delete c;
        }
    }

    for (int i=0; i<falsehit.size(); i++)
        delete (GroupObjectSearchResult*)falsehit.get(i);
}

////
// ----------------------------------------------------------------------------
// multi-point kNN search
// ----------------------------------------------------------------------------
void DistIndexSearch::groupKNNSearch(Graph& a_graph, DistIndex& a_distidx,
                                     const int* a_src, const int a_cnt,
                                     const int a_k,
                                     Array& a_result, int& a_nodeaccess, int& a_edgeaccess)
{
    // ------------------------------------------------------------------------
    // local carrier for ordered node exploring
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_nid;
        const int   m_oid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_nid, const float a_cost, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        carrier(const int a_nid, const float a_cost, Array& a_path, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_path(a_path), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_oid < c1->m_oid) return -1;
            if (c0->m_oid > c1->m_oid) return +1;
            return 0;
        };
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // collect result candidates
    // ------------------------------------------------------------------------
    Hash cand;
    Array candlist;
    for (int i=0; i<a_cnt; i++)
    {
        const Array* a = a_distidx.getNode(a_src[i]);
        for (int j=0; j<a->size(); j++)
        {
            DistSignature* distsign = (DistSignature*)a->get(j);
            GroupObjectSearchResult* r =
                (GroupObjectSearchResult*)cand.get(distsign->m_oid);
            if (r == 0)
            {
                r = new GroupObjectSearchResult(-1,distsign->m_oid,a_cnt);
                cand.put(distsign->m_oid, r);
                candlist.append(r);
            }
            r->m_cost[i] = distsign->m_cost;
            delete distsign;
        }
        delete a;
    }

    // ------------------------------------------------------------------------
    // filter candidates
    // ------------------------------------------------------------------------
    candlist.sort(GroupObjectSearchResult::compare);
    for (int i=0; i<candlist.size(); i++)
    {
        GroupObjectSearchResult* r = (GroupObjectSearchResult*)candlist.get(i);
        if (a_result.size() > a_k)
        {
            delete r;
        }
        else
        {
            if (r->allreached(a_cnt))
                a_result.append(r);
            else
                delete r;
        }
    }

    // ------------------------------------------------------------------------
    // network expansion
    // ------------------------------------------------------------------------
    for (int i=0; i<a_cnt; i++)
    {
        Set visited(1000);
        BinHeap heap(carrier::compare);
        for (int j=0; j<a_result.size(); j++)
        {
            GroupObjectSearchResult* r =
                (GroupObjectSearchResult*)a_result.get(j);
            heap.insert(new carrier(a_src[i], r->m_cost[i], r->m_oid));
        }

        // --------------------------------------------------------------------
        // network expansion
        // --------------------------------------------------------------------
        while (!heap.isEmpty())
        {
            carrier* c = (carrier*)heap.removeTop();

            // ----------------------------------------------------------------
            // examine distance signature for next hop
            // ----------------------------------------------------------------
            const Array* a = a_distidx.getNode(c->m_nid);
            DistSignature d(c->m_oid,0,0);
            const int k = a->binSearch(&d, DistSignature::compareID);
            //for (int k=0; k<a->size(); k++)
            if (k >= 0)
            {
                DistSignature* distsign = (DistSignature*)a->get(k);
                if (distsign->m_oid == c->m_oid)
                {
                    // --------------------------------------------------------
                    // terminate the exploring for an object if it is found
                    // --------------------------------------------------------
                    if (distsign->m_cost == 0)
                    {
                        GroupObjectSearchResult* r =
                            (GroupObjectSearchResult*)cand.get(c->m_oid);
                        r->m_nid = c->m_nid;
                        r->m_path[i].copy(c->m_path);   // record the path
                    }
                    // --------------------------------------------------------
                    // continue the exploring
                    // --------------------------------------------------------
                    else
                    {
                        if (!visited.in((void*)c->m_nid))
                        {
                            a_nodeaccess++;
                            visited.insert((void*)c->m_nid);
                        }

                        Node* node = (Node*)a_graph.getNode(c->m_nid);
                        float cost = 0;
                        for (int l=0; l<node->m_edges.size(); l++)
                        {
                            Edge* e = (Edge*)node->m_edges.get(l);
                            if (e->m_neighbor == distsign->m_prev)
                            {
                                cost = e->m_cost;
                                a_edgeaccess++;
                                break;
                            }
                        }
                        heap.insert(
                            new carrier(distsign->m_prev, c->m_cost + cost,
                            c->m_path, distsign->m_oid));
                        delete node;
                    }
                    //break;
                }
            }
            // ----------------------------------------------------------------
            // clean up
            // ----------------------------------------------------------------
            for (int d=0; d<a->size(); d++)
                delete (DistSignature*)a->get(d);
            delete a;
            delete c;
        }
    }
}


///
void DistIndexSearch::groupKNNSearch(Graph& a_graph, DistIndex& a_distidx,
                                     const int* a_src, const int a_cnt,
                                     const int a_k,
                                     Array& a_result,
                                     int& a_nodeaccess, int& a_edgeaccess,
                                     Array& a_visited)
{
    // ------------------------------------------------------------------------
    // local carrier for ordered node exploring
    // ------------------------------------------------------------------------
    class carrier
    {
    public:
        const int   m_nid;
        const int   m_oid;
        const float m_cost;
        Array       m_path;
    public:
        carrier(const int a_nid, const float a_cost, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        carrier(const int a_nid, const float a_cost, Array& a_path, const int a_oid):
            m_nid(a_nid), m_cost(a_cost), m_path(a_path), m_oid(a_oid)
            {
                m_path.append((void*)a_nid);
            }
        virtual ~carrier() {};
        static int compare(const void* a0, const void* a1)
        {
            carrier* c0 = *(carrier**)a0;
            carrier* c1 = *(carrier**)a1;
            if (c0->m_cost < c1->m_cost) return -1;
            if (c0->m_cost > c1->m_cost) return +1;
            if (c0->m_oid < c1->m_oid) return -1;
            if (c0->m_oid > c1->m_oid) return +1;
            return 0;
        };
    };

    // ------------------------------------------------------------------------
    // initialization
    // ------------------------------------------------------------------------
    a_result.clean();
    a_nodeaccess = 0;
    a_edgeaccess = 0;

    // ------------------------------------------------------------------------
    // collect result candidates
    // ------------------------------------------------------------------------
    Hash cand;
    Array candlist;
    for (int i=0; i<a_cnt; i++)
    {
        const Array* a = a_distidx.getNode(a_src[i]);
        for (int j=0; j<a->size(); j++)
        {
            DistSignature* distsign = (DistSignature*)a->get(j);
            GroupObjectSearchResult* r =
                (GroupObjectSearchResult*)cand.get(distsign->m_oid);
            if (r == 0)
            {
                r = new GroupObjectSearchResult(-1,distsign->m_oid,a_cnt);
                cand.put(distsign->m_oid, r);
                candlist.append(r);
            }
            r->m_cost[i] = distsign->m_cost;
            delete distsign;
        }
        delete a;
    }

    // ------------------------------------------------------------------------
    // filter candidates
    // ------------------------------------------------------------------------
    candlist.sort(GroupObjectSearchResult::compare);
    for (int i=0; i<candlist.size(); i++)
    {
        GroupObjectSearchResult* r = (GroupObjectSearchResult*)candlist.get(i);
        if (a_result.size() >= a_k)
        {
            delete r;
        }
        else
        {
            if (r->allreached(a_cnt))
                a_result.append(r);
            else
                delete r;
        }
    }

    // ------------------------------------------------------------------------
    // network expansion
    // ------------------------------------------------------------------------
    for (int i=0; i<a_cnt; i++)
    {
        Set visited(1000);
        BinHeap heap(carrier::compare);
        for (int j=0; j<a_result.size(); j++)
        {
            GroupObjectSearchResult* r =
                (GroupObjectSearchResult*)a_result.get(j);
            heap.insert(new carrier(a_src[i], r->m_cost[i], r->m_oid));
        }

        // --------------------------------------------------------------------
        // network expansion
        // --------------------------------------------------------------------
        while (!heap.isEmpty())
        {
            carrier* c = (carrier*)heap.removeTop();

            a_visited.append((void*)c->m_nid);

            // ----------------------------------------------------------------
            // examine distance signature for next hop
            // ----------------------------------------------------------------
            const Array* a = a_distidx.getNode(c->m_nid);
            DistSignature d(c->m_oid,0,0);
            const int k = a->binSearch(&d, DistSignature::compareID);
            //for (int k=0; k<a->size(); k++)
            if (k >= 0)
            {
                DistSignature* distsign = (DistSignature*)a->get(k);
                if (distsign->m_oid == c->m_oid)
                {
                    // --------------------------------------------------------
                    // terminate the exploring for an object if it is found
                    // --------------------------------------------------------
                    if (distsign->m_cost == 0)
                    {
                        GroupObjectSearchResult* r =
                            (GroupObjectSearchResult*)cand.get(c->m_oid);
                        r->m_nid = c->m_nid;
                        r->m_path[i].copy(c->m_path);   // record the path
                    }
                    // --------------------------------------------------------
                    // continue the exploring
                    // --------------------------------------------------------
                    else
                    {
                        if (!visited.in((void*)c->m_nid))
                        {
                            a_nodeaccess++;
                            visited.insert((void*)c->m_nid);
                        }

                        Node* node = (Node*)a_graph.getNode(c->m_nid);
                        float cost = 0;
                        for (int l=0; l<node->m_edges.size(); l++)
                        {
                            Edge* e = (Edge*)node->m_edges.get(l);
                            if (e->m_neighbor == distsign->m_prev)
                            {
                                cost = e->m_cost;
                                a_edgeaccess++;
                                break;
                            }
                        }
                        heap.insert(
                            new carrier(distsign->m_prev, c->m_cost + cost,
                            c->m_path, distsign->m_oid));
                        delete node;
                    }
                    //break;
                }
            }
            // ----------------------------------------------------------------
            // clean up
            // ----------------------------------------------------------------
            for (int d=0; d<a->size(); d++)
                delete (DistSignature*)a->get(d);
            delete a;
            delete c;
        }
    }
}

