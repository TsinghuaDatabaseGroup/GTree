#-------------------------------------------------------------------------------
#  Author: Ken C. K. Lee
#  Email:  cklee@cse.psu.edu
#  Web:    http://www.cse.psu.edu/~cklee
#  Date:   Jan, 2008
#
#  Copyright(c) 2008
#  This library is for non-commerical use only.
#
#-------------------------------------------------------------------------------

# ==============================================================================
# compiler settings
# ==============================================================================
CC	= g++
CFLAGS	= -c 
LFLAGS	= -lm 

# ==============================================================================
# output directory 
# ==============================================================================
BIN	= ./bin

# ==============================================================================
# objects 
# ==============================================================================
coll		= collection.o
param		= param.o
psdraw		= psdraw.o
graph		= graph.o node.o graphsearch.o 
graphplot	= graphplot.o
hiergraph	= hiergraph.o bordernode.o shortcuttreenode.o $(graph)
memory		= segmmem.o segfmem.o access.o iomeasure.o
plaingraphobj	= nodemap.o objectsearch.o
hiergraphobj	= graphmap.o hierobjsearch.o $(plaingraphobj)
spatialgraphobj	= spatialmap.o spatialsearch.o
distidx		= distidx.o distidxsearch.o
spqdtree        = spqdtree.o spqdtreenode.o spqdtreerec.o point.o bound.o
distbrws	= distbrws.o distbrwssearch.o $(spqdtree)

# ==============================================================================
# binaries 
# ==============================================================================
all:	graphobjloader graphobjqueryloader \
	hiergraphloader distidxloader distbrwsloader \
	plainnn hiernn spatialnn distidxnn distbrwsnn \
	plainrange hierrange spatialrange distidxrange \
	plaingnn hiergnn spatialgnn distidxgnn \
	plaingrange hiergrange spatialgrange distidxgrange \
	hiernn_gdist hiernn_gtree_density

# ==============================================================================
# index builder
# ==============================================================================
graphobjloader:	graphobjloader.o objectgen.o $(coll) $(graph) $(graphplot) \
		$(psdraw) $(memory) $(param)
	$(CC) $(LFLAGS) \
	graphobjloader.o objectgen.o \
	$(coll) $(graph) $(graphplot) $(psdraw) $(memory) $(param) \
	-o $(BIN)/graphobjloader

graphobjqueryloader:	graphobjqueryloader.o objectgen.o $(coll) $(graph) $(graphplot) \
		$(psdraw) $(memory) $(param)
	$(CC) $(LFLAGS) \
	graphobjqueryloader.o objectgen.o \
	$(coll) $(graph) $(graphplot) $(psdraw) $(memory) $(param) \
	-o $(BIN)/graphobjqueryloader

hiergraphloader: hiergraphloader.o graphpartition.o $(coll) $(hiergraph) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	hiergraphloader.o graphpartition.o \
	$(coll) $(hiergraph) $(memory) $(param) \
	-o $(BIN)/hiergraphloader

distidxloader:	distidxloader.o $(coll) $(graph) $(distidx) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distidxloader.o $(coll) $(graph) $(distidx) $(memory) $(param) \
	-o $(BIN)/distidxloader

distbrwsloader:	distbrwsloader.o $(coll) $(graph) $(distbrws) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distbrwsloader.o $(coll) $(graph) $(distbrws) $(memory) $(param) \
	-o $(BIN)/distbrwsloader

# ==============================================================================
# single point nearest neighbor search
# ==============================================================================
plainnn:	plainnn.o $(coll) $(graph) $(plaingraphobj) $(memory) $(param)
	$(CC) $(LFLAGS) \
	plainnn.o $(coll) $(graph) $(plaingraphobj) $(memory) $(param) \
	-o $(BIN)/plainnn

hiernn:	hiernn.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param)
	$(CC) $(LFLAGS) \
	hiernn.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hiernn

hiernn_gtree:	hiernn_gtree.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param)
	$(CC) $(LFLAGS) \
	hiernn_gtree.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hiernn_gtree
#add by bilong shen for test for gtree 2016.06.21

hiernn_gtree_density:	hiernn_gtree_density.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param)
	$(CC) $(LFLAGS) \
	hiernn_gtree_density.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hiernn_gtree_density
#add by bilong Shen for test density 2016.10.07
hiernn_gdist:	hiernn_gptree_dif_dist_test.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param)
	$(CC) $(LFLAGS) \
	hiernn_gptree_dif_dist_test.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hiernn_gdist
#add by bilong shen for test for gtree 2016.06.29

spatialnn:	spatialnn.o $(coll) $(graph) $(spatialgraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	spatialnn.o $(coll) $(graph) $(spatialgraphobj) $(memory) $(param) \
	-o $(BIN)/spatialnn

distidxnn:	distidxnn.o $(coll) $(graph) $(distidx) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distidxnn.o $(coll) $(graph) $(distidx) $(memory) $(param) \
	-o $(BIN)/distidxnn

distbrwsnn:	distbrwsnn.o $(coll) $(graph) $(distbrws) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distbrwsnn.o $(coll) $(graph) $(distbrws) $(memory) $(param) \
	-o $(BIN)/distbrwsnn

# ==============================================================================
# single point range search
# ==============================================================================
plainrange:	plainrange.o $(coll) $(graph) $(plaingraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	plainrange.o $(coll) $(graph) $(plaingraphobj) $(memory) $(param) \
	-o $(BIN)/plainrange

hierrange:	hierrange.o $(coll) $(hiergraph) $(hiergraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	hierrange.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hierrange

spatialrange:	spatialrange.o $(coll) $(graph) $(spatialgraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	spatialrange.o $(coll) $(graph) $(spatialgraphobj) $(memory) $(param) \
	-o $(BIN)/spatialrange

distidxrange:	distidxrange.o $(coll) $(graph) $(distidx) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distidxrange.o $(coll) $(graph) $(distidx) $(memory) $(param) \
	-o $(BIN)/distidxrange

distbrwsrange:	distbrwsrange.o $(coll) $(graph) $(distbrws) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distbrwsrange.o $(coll) $(graph) $(distbrws) $(memory) $(param) \
	-o $(BIN)/distbrwsrange

# =============================================================================
# multi-point nearest neighbor search
# =============================================================================
plaingnn:	plaingnn.o $(coll) $(graph) $(plaingraphobj) $(memory) $(param)
	$(CC) $(LFLAGS) \
	plaingnn.o $(coll) $(graph) $(plaingraphobj) $(memory) $(param) \
	-o $(BIN)/plaingnn

hiergnn:	hiergnn.o $(coll) $(hiergraph) $(hiergraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	hiergnn.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hiergnn

spatialgnn:	spatialgnn.o $(coll) $(graph) $(spatialgraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	spatialgnn.o $(coll) $(graph) $(spatialgraphobj) $(memory) $(param) \
	-o $(BIN)/spatialgnn

distidxgnn:	distidxgnn.o $(coll) $(graph) $(distidx) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distidxgnn.o $(coll) $(graph) $(distidx) $(memory) $(param) \
	-o $(BIN)/distidxgnn

# =============================================================================
# multi-point range search
# =============================================================================
plaingrange:	plaingrange.o $(coll) $(graph) $(plaingraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	plaingrange.o $(coll) $(graph) $(plaingraphobj) $(memory) $(param) \
	-o $(BIN)/plaingrange

hiergrange:	hiergrange.o $(coll) $(hiergraph) $(hiergraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	hiergrange.o $(coll) $(hiergraph) $(hiergraphobj) $(memory) $(param) \
	-o $(BIN)/hiergrange

spatialgrange:	spatialgrange.o $(coll) $(graph) $(spatialgraphobj) \
		$(memory) $(param)
	$(CC) $(LFLAGS) \
	spatialgrange.o $(coll) $(graph) $(spatialgraphobj) $(memory) $(param) \
	-o $(BIN)/spatialgrange

distidxgrange:	distidxgrange.o $(coll) $(graph) $(distidx) $(memory) $(param)
	$(CC) $(LFLAGS) \
	distidxgrange.o $(coll) $(graph) $(distidx) $(memory) $(param) \
	-o $(BIN)/distidxgrange


# =============================================================================
# common rules
# =============================================================================
.cc.o:
	$(CC) $(CFLAGS) -O3 -std=c++11 $<
	#$(CC) $(CFLAGS) -O2 $<

# =============================================================================
# clean up 
# =============================================================================
clean:
	rm -Rf *.o
	rm -Rf $(BIN)/*

