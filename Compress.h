/*
    头文件：
    函数原型和类型声明
*/
#ifndef COMPRESS_H_
#define COMPRESS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define CH_KINDS 256

//统计字符频率的节点
typedef struct
{
    unsigned char uch;     //8位无符号字符
    unsigned long weight;  //每种字符的频率，也即权重
} chFreqNode;

//Huffman树节点
typedef struct
{
    unsigned char uch;           //8位无符号字符
    unsigned long weight;        //每种字符的频率，也即权重
    char * code;                 //每种字符对应的哈夫曼编码（动态分配存储空间）
    int parent, lchild, rchild;  //定义双亲和左右孩子
} HufTreeNode, *HufTree;

//统计输入文件中字符出现的频率
unsigned int Statistic(FILE * pInfile, chFreqNode * pChar_freq, unsigned long * pFile_len);

//选择频率最小和次小的两个节点
void Select_min(HufTreeNode * pHuf_tree, unsigned int curHufTreeNode_num, unsigned int * pIndex1, unsigned int * pIndex2);

//建立Huffman树
void CreateHufTree(HufTreeNode * pHuf_tree, unsigned int Ch_kinds_num, unsigned int HufTreeNode_num);

//生成Huffman编码
void CreateHufCode(HufTreeNode * pHuf_tree, unsigned int Ch_kinds_num);

//文件压缩
int Compress(char* ifname, char* ofname);

//文件解压缩
int Decompress(char* ifname, char* ofname);

#endif