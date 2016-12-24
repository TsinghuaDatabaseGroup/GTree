First, I want to applogize for my bad coding style (all in one file +_+)
It is originally for experimental test, thus it is not well-formed in c++ style.
Hope not bring any troubles to you when reading the code.

-----

The original code was written in one file,
To make it clear, i split it into two files(parts):
	1. GTree build: (gtree_build.cpp)
		INPUT:  graph file(.cnode, .cedge)
		OUTPUT: GTree index(.gtree)
				GTree branch paths(.gpath)
				GTree distance matrix(.mind)
	2. GTree KNN Search: (gtree_query.cpp)
		INPUT:	graph file(.cnode, .cedge)
				GTree index(.gtree)
				GTree branch paths(.gpath)
				GTree distance matrix(.mind)
		TODO:   KNN Serach(knn_query())
Some annotations were written among the code.

-----

METIS is the essential part of the GTree, it is used to partition the graph.
Thus, before compile our code, you must install METIS in your linux system.

METIS link & download: http://glaros.dtc.umn.edu/gkhome/metis/metis/overview

[CAUTION]:
Beware the linking issue, in our case, we use "g++ ... -lmetis"
If it is not working, you can try "g++ ... -L/**/**/YOUR_METIS_LIB_PATH"
Make sure "metis.h" is in your default include(.h) directory.

-----

For better understanding of our code, we provide example(CAL dataset)
File use: (Note the file input format)
	cal.cnode (graph node file)
	cal.cedge (graph edge file)
	cal.object(candidate object list)

[CAUTION]:
In our code, we did not assert the input graph is connected graph
But connected graph must be guaranteed before METIS partition the graph
Hence, it is suggested you have to pre-process the input road network for your own dataset
Luckily, all the dataset used in our experiments are naturally connected:
	Dataset link & download:
		http://www.cs.fsu.edu/~lifeifei/SpatialDataset.htm
		http://www.dis.uniroma1.it/challenge9/index.shtml

-----

Quick Use:
	make gtree_query
	./gtree_query

Just for simple test!

----
