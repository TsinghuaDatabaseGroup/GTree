/*************************************************************************
	> File Name: gen_rand_obj.cpp
	> Author: Bilong Shen
	> Mail: shenbilong@gmail.com
	> Created Time: Mon 20 Jun 2016 11:57:12 PM CST

	生成供测试用的obj文件
	文件格式为
	line1:obj_num 测试的obj数量
	line2:obj的编号
	.....
 /*************************************************************************/

#include <vector>
#include<iostream>
#include<algorithm>
#include<stdio.h>
#include "param.h"

#define col_node_num 435666
#define cal_node_num 21048
using namespace std;

template<typename T>
void gen_rand_obj(int SIZE, int candi_num, vector <T> &rst);
int write_obj_file(string &file_name, int file_num);
void helpmsg(const char* pgm)
{
	cerr << "Experiment Generate by Bilong Shen 2016/6/21" << endl;
	cerr << "Suggested arguments for GPtree test road:" << endl;
	cerr << "> " << pgm << " ";
	cerr << "-o col -f 2 -n 43333 -b 10000 -q 32" << endl;
	cerr << "explanations:" << endl;
	cerr << "-o: dataset name" << endl;
	cerr << "-n: node num of dataset"<< endl;
	cerr << "-f: file num to generate" << endl;
	cerr << "-b: #object number" << endl;
	cerr << "-q: #queries number" << endl;

}



template<typename T>
void gen_rand_obj(int SIZE, int candi_num, vector <T> &rst) {
	vector <T> temp;
	for (int i = 0; i < SIZE; i++) {
		temp.push_back(i);
	}
	random_shuffle(temp.begin(), temp.end());

	for (int i = 0; i < temp.size(); i++) {
		rst.push_back(temp[i]);
	}
	while (rst.size() > candi_num) {
		rst.pop_back();
	}
}

/*
生成供测试用的obj文件
文件格式为
line1:obj_num 测试的obj数量
line2:obj的编号
.....
*/
int write_obj_file(string &file_name, int file_num , int full_size, int obj_num, int lq_num) {
	//输入测试集合的名称，文件个数file_num, 文件中obj的总数 full_size ,生成obj数目：obj_num, 请求的数量：lq_num

	for (int k = 0; k < file_num; k++) { //output all the files
		char file_object[100];
		cout << file_name << endl;

		sprintf(file_object, "%s_fullsize_%d_objNum_%d_lq_num_%d.%d", file_name.c_str(), full_size, obj_num,lq_num, file_num, k);
		cout << file_object;
		FILE *fout = fopen(file_object, "w");
		vector<int> rest;

		/*generate object data*/
		gen_rand_obj(full_size, obj_num, rest);

		/*Out put obj to  file */

		fprintf(fout, "%d\n", obj_num);
		for (int i = 0; i < rest.size(); i++) {
			fprintf(fout, "%d\n", rest[i]);
		}
		rest.clear();


		/*Out put lq to  file */
		gen_rand_obj(full_size, lq_num, rest);
		fprintf(fout, "%d\n", lq_num);
		for (int i = 0; i < rest.size(); i++) {
			fprintf(fout, "%d\n", rest[i]);
		}


		fclose(fout);
		rest.clear();
	}
	return 0;
}

int main(const int a_argc, const char** a_argv)
{
	if (a_argc == 1)
	{
		helpmsg(a_argv[0]);
		return -1;
	}

	cerr << "kNN object search on hierarchical graph" << endl;
	//-------------------------------------------------------------------------
	// initialization
	//-------------------------------------------------------------------------
	const char* objflname = Param::read(a_argc, a_argv, "-o", "");
	const char* c_node_num = Param::read(a_argc, a_argv, "-n", "1000");
	const char* c_file_num = Param::read(a_argc, a_argv, "-f", "1");
	const char* c_obj_num = Param::read(a_argc, a_argv, "-b", "100");
	const char* c_query_num = Param::read(a_argc, a_argv, "-q", "10");
	
	int node_num = atoi(c_node_num);
	int obj_num = atoi(c_obj_num);
	int file_num = atoi(c_file_num);
	int query_num = atoi(c_query_num);
	string test(objflname);


	write_obj_file(test, file_num, node_num, obj_num , query_num);
		//输入测试集合的名称，文件个数file_num, 文件中obj的总数 full_size ,生成obj数目：obj_num, 请求的数量：lq_num


}
