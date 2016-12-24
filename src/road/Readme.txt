1.索引文件生成方法：
./hiergraphloader -n BJ_washed.cnode -e BJ_washed.cedge  -t 4 -l 8 -h BJ_washed_roadtree_t4_l8.idx  
t和l是ROAD原文的参数，分别表示分支和层数
2.测试方法：
hiernn_gtree -h BJ_roadtree_t4_l8.idx -x testFile

testFile文件格式：
第一行：           num_obj        //  object 数目
接下来num_obj行：  Obje_id        //表示的是vertex的点的id    
query数目：        num_query      //query的数目
接下来num_query行  query_id       //表示的是vertex的点的id