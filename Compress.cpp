/*
    函数实现
*/
#include "Compress.h"

//统计输入文件中字符出现的频率
unsigned int Statistic(FILE * pInfile, chFreqNode * pChar_freq, unsigned long * pFile_len)
{
    int i, j;
    unsigned int Ch_kinds_num;
    //初始化字符频率节点
    for(i = 0; i < CH_KINDS; ++i)
    {
        pChar_freq[i].uch = (unsigned char)i;  //256个下标对应256种字符
        pChar_freq[i].weight = 0;
    }

    unsigned char char_temp;
    fread((char *)&char_temp, sizeof(unsigned char), 1, pInfile);  //读入一个字符
    while(!feof(pInfile))
    {
        ++pChar_freq[char_temp].weight;   //ch_temp对应下标的字符频率加一
        ++(*pFile_len);                   //文件长度加一
        fread((char *)&char_temp, sizeof(unsigned char), 1, pInfile); //读入下一个字符
    }

    //将字符频率数组排序，降序排列，频率为0的放在最后
    chFreqNode temp_node;
    for(i = 0; i < CH_KINDS - 1; ++i)
        for(j = i + 1; j < CH_KINDS; ++j)
        {
            if(pChar_freq[i].weight < pChar_freq[j].weight)
            {
                temp_node = pChar_freq[i];
                pChar_freq[i] = pChar_freq[j];
                pChar_freq[j] = temp_node;
            }
        }

    //统计实际出现的字符种类数量
    for(i = 0; i < CH_KINDS; ++i)
        if(pChar_freq[i].weight == 0)
            break;
    Ch_kinds_num = i;

    return Ch_kinds_num;
}

//选择频率最小和次小的两个Huffman节点
void Select_min(HufTreeNode * pHuf_tree, unsigned int curHufTreeNode_num, unsigned int * pIndex1, unsigned int * pIndex2)
{
    //找到最小的节点后，将parent成员设置为1，表示已选中
    //找最小
    unsigned int i;
    unsigned long minWeight = ULONG_MAX;
    for(i = 0; i < curHufTreeNode_num; i++)
    {
        if(pHuf_tree[i].parent == 0 && pHuf_tree[i].weight < minWeight)
        {
            minWeight = pHuf_tree[i].weight;
            *pIndex1 = i;
        }
    }

    pHuf_tree[*pIndex1].parent = 1;   //标记此节点以被选中

    //找次小
    minWeight = ULONG_MAX;
    for(i = 0; i < curHufTreeNode_num; ++i)
    {
        if(pHuf_tree[i].parent == 0 && pHuf_tree[i].weight < minWeight)
        {
            minWeight = pHuf_tree[i].weight;
            *pIndex2 = i;
        }
    }
}

//建立Huffman树
void CreateHufTree(HufTreeNode * pHuf_tree, unsigned int Ch_kinds_num, unsigned int HufTreeNode_num)
{
    unsigned int index1, index2;
    for(unsigned int i = Ch_kinds_num; i < HufTreeNode_num; ++i)
    {
        Select_min(pHuf_tree, i, &index1, &index2);
        pHuf_tree[i].weight = pHuf_tree[index1].weight + pHuf_tree[index2].weight;
        pHuf_tree[i].lchild = index1;
        pHuf_tree[i].rchild = index2;
        pHuf_tree[index1].parent = pHuf_tree[index2].parent = i;
    }
}

//生成Huffman编码
void CreateHufCode(HufTreeNode * pHuf_tree, unsigned int Ch_kinds_num)
{
    unsigned int cur, next, index;
    char * pCode_temp = (char *)malloc(256 * sizeof(char)); //暂存编码，最多256个叶子，编码长度不超过255
    pCode_temp[255] = '\0';

    for(unsigned int i = 0; i < Ch_kinds_num; ++i)
    {
        index = 255;    //编码临时空间索引初始化

        //从叶节点向根节点反向遍历求编码
        cur = i;
        next = pHuf_tree[i].parent;
        while(next != 0)
        {
            if(pHuf_tree[next].lchild == cur)
                pCode_temp[--index] = '0';    //左孩子为'0'
            else
                pCode_temp[--index] = '1';    //右孩子为'1'
            cur = next;
            next = pHuf_tree[next].parent;
        }
        pHuf_tree[i].code = (char *)malloc((256 - index) * sizeof(char));  //为第i个字符编码动态分配内存
        strcpy(pHuf_tree[i].code, &pCode_temp[index]);   //正向保存编码到相应树节点
    }
    free(pCode_temp);  //释放编码临时空间
}

//文件压缩
int Compress(char* ifname, char* ofname)
{
    unsigned int i;
    unsigned int Ch_kinds_num;     //字符种类数量
    unsigned int HufTreeNode_num;
    unsigned long file_len = 0;    //文件长度
    unsigned char char_temp;       //8位字符缓存
    char code_buf[256] = "\0";     //待存编码缓存
    unsigned int code_len;
    
    FILE * pInfile, * pOutfile;
    HufTree pHuf_tree;

    pInfile = fopen(ifname, "rb");
    //判断文件是否存在
    if(pInfile == NULL)
        return -1;

    //动态分配CH_KINDS个的字符频率节点的内存
    //统计并拷贝到树节点后需要立即释放
    chFreqNode * pChar_freq = (chFreqNode *)malloc(CH_KINDS * sizeof(chFreqNode));
    //统计字符种类和频率
    Ch_kinds_num = Statistic(pInfile, pChar_freq, &file_len);
    fclose(pInfile);

    if (Ch_kinds_num == 1)   //只有一种字符
    {
        pOutfile = fopen(ofname, "wb");    //打开压缩后将生成的文件
        fwrite((char *)&Ch_kinds_num, sizeof(unsigned int), 1, pOutfile);           //写入字符种类
        fwrite((char *)&pChar_freq[0].uch, sizeof(unsigned char), 1, pOutfile);     //写入唯一的字符
        fwrite((char *)&pChar_freq[0].weight, sizeof(unsigned long), 1, pOutfile);  //写入字符频率，也就是文件长度
        free(pChar_freq);  //释放字符频率节点空间
        fclose(pOutfile);
    }
    else
    {
        //创建Huffman森林
        HufTreeNode_num = 2 * Ch_kinds_num - 1;      //根据字符种类树，计算Huffman编码树节点数
        pHuf_tree = (HufTreeNode *)malloc(HufTreeNode_num * sizeof(HufTreeNode));   ////动态分配Huffman森林所需内存
        
        for(i = 0; i < Ch_kinds_num; ++i)
        {
            //将字符频率节点的值拷贝到Huffman森林
            pHuf_tree[i].uch = pChar_freq[i].uch;
            pHuf_tree[i].weight = pChar_freq[i].weight;
            pHuf_tree[i].parent = 0;
        }
        free(pChar_freq);  //释放字符频率节点空间
        
        //初始化后HufTreeNode_num - Ch_kinds_num个节点
        for(; i < HufTreeNode_num; ++i)
        {
            pHuf_tree[i].parent = 0;
        }
        
        //创建Huffman编码树
        CreateHufTree(pHuf_tree, Ch_kinds_num, HufTreeNode_num);

        //生成Huffman编码
        CreateHufCode(pHuf_tree, Ch_kinds_num);

        //写入字符和相应的权重，供解压时重建Huffman编码树
        pOutfile = fopen(ofname, "wb");
        fwrite((char *)&Ch_kinds_num, sizeof(unsigned int), 1, pOutfile);  //写入字符种类数目
        for (i = 0; i < Ch_kinds_num; ++i)
        {
            fwrite((char *)&pHuf_tree[i].uch, sizeof(unsigned char), 1, pOutfile);   //写入字符，已排序，读出后顺序不变
            fwrite((char *)&pHuf_tree[i].weight, sizeof(unsigned long), 1, pOutfile);  //写入相应权重
        }

        //紧接着字符种类和权重信息后面写入文件长度和字符编码
        fwrite((char *)&file_len, sizeof(unsigned long), 1, pOutfile);  //写入文件长度

        pInfile = fopen(ifname, "rb");    //以二进制形式打开源文件

        fread((char *)&char_temp, sizeof(unsigned char), 1, pInfile);   //每次读取1字节,即8位
        while(!feof(pInfile))
        {
            //匹配字符对应的编码
            for(i = 0; i < Ch_kinds_num; ++i)
            {
                if(char_temp == pHuf_tree[i].uch)
                    strcat(code_buf, pHuf_tree[i].code);
            }

            //以8位为处理单元
            while(strlen(code_buf) >= 8)
            {
                char_temp = '\0';         //清空字符暂存空间，改为暂存字符对应编码
                for(i = 0; i < 8; ++i)
                {
                    char_temp <<= 1;      //左移1位，为下一个bit腾出位置
                    if(code_buf[i] == '1')
                        char_temp |= 1;   //当编码为'1'时，通过或操作符将其添加到字节的最低位
                }
                fwrite((char *)&char_temp, sizeof(unsigned char), 1, pOutfile);  //将字节对应编码存入文件
                strcpy(code_buf, code_buf + 8);   //编码缓存去除已处理的前8位
            }
            fread((char *)&char_temp, sizeof(unsigned char), 1, pInfile);  //每次读取1字节，即8位
        }

        //处理最后不足8位的编码
        code_len = strlen(code_buf);
        if(code_len > 0)
        {
            char_temp = '\0';
            for(i = 0; i < code_len; ++i)
            {
                char_temp <<= 1;
                if(code_buf[i] == '1')
                    char_temp |= 1;
            }
            char_temp <<= 8 - code_len;     //将编码字段从尾部移到字节的高位
            fwrite((char *)&char_temp, sizeof(unsigned char), 1, pOutfile);  //存入最后一个字节
        }

        //关闭文件
        fclose(pInfile);
        fclose(pOutfile);

        //释放内存
        for(i = 0; i < Ch_kinds_num; ++i)
            free(pHuf_tree[i].code);
        free(pHuf_tree);
    }

    return 1;
}

//文件解压缩
int Decompress(char * ifname, char * ofname)
{
    unsigned int i;
    unsigned long file_len;         //文件长度
    unsigned long writen_len = 0;   //控制文件写入长度

    FILE * pInfile, * pOutfile;     //输入输出文件流

    unsigned int Ch_kinds_num;      //字符种类数
    unsigned int HufTreeNode_num;   //编码树节点数
    HufTree pHuf_tree;              //编码树
    unsigned int HufTreeRoot;       //编码树根节点位置

    unsigned char char_temp;        //暂存8位编码

    pInfile = fopen(ifname, "rb");
    //判断文件是否存在
    if (pInfile == NULL)
        return -1;

    //读取压缩文件前端的字符以及对应的编码，用于重建Huffman编码树
    fread((char *)&Ch_kinds_num, sizeof(unsigned int), 1, pInfile);   //读取字符种类数
    if (Ch_kinds_num == 1)
    {      
        fread((char *)&char_temp, sizeof(unsigned char), 1, pInfile);  //读取唯一的字符
        fread((char *)&file_len, sizeof(unsigned long), 1, pInfile);   //读取文件长度
        pOutfile = fopen(ofname, "wb");     //二进制可写形式打开输出文件
        while(file_len--)
        {
            fwrite((char *)&char_temp, sizeof(unsigned char), 1, pOutfile);
        }
        fclose(pInfile);
        fclose(pOutfile);
    }
    else
    {
        HufTreeNode_num = 2 * Ch_kinds_num - 1;    //根据字符频率节点数计算树节点数
        pHuf_tree = (HufTreeNode *)malloc(HufTreeNode_num * sizeof(HufTreeNode));  //动态分配Hufman节点内存空间
        //读取字符及对应权重，存入树节点
        for(i = 0; i < Ch_kinds_num; ++i)
        {
            fread((char *)&pHuf_tree[i].uch, sizeof(unsigned char), 1, pInfile);     //保存字符
            fread((char *)&pHuf_tree[i].weight, sizeof(unsigned long), 1, pInfile);  //保存权重
            pHuf_tree[i].parent = 0;
        }
        //初始化后HufTreeNode_num - Ch_kinds_num 个节点的parent
        for(; i < HufTreeNode_num; ++i)
            pHuf_tree[i].parent = 0;

        //重建Huffman编码树，与压缩时一样
        CreateHufTree(pHuf_tree, Ch_kinds_num, HufTreeNode_num);

        //读取文件长度和编码
        fread((char *)&file_len, sizeof(unsigned long), 1, pInfile);

        pOutfile = fopen(ofname, "wb");  //二进制可写形式打开输出文件
        HufTreeRoot = HufTreeNode_num - 1;

        while(1)
        {
            fread((char *)&char_temp, sizeof(unsigned char), 1, pInfile);   //读取一个字符长度的编码

            //处理一个字节长度的编码
            for(i = 0; i < 8; ++i)
            {
                //由根节点开始从上往下遍历
                if(char_temp & 0x80)
                    HufTreeRoot = pHuf_tree[HufTreeRoot].rchild;   //编码为1时，遍历右孩子
                else
                    HufTreeRoot = pHuf_tree[HufTreeRoot].lchild;   //编码为0时, 遍历左孩子
                
                if (HufTreeRoot < Ch_kinds_num)   //到达叶节点时
                {
                    fwrite((char *)&pHuf_tree[HufTreeRoot].uch, sizeof(unsigned char), 1, pOutfile);
                    ++writen_len;                //写入文件长度加一
                    if (writen_len == file_len)   //控制文件长度，跳出内层循环
                        break;
                    HufTreeRoot = HufTreeNode_num - 1;  //回到根节点
                }
                char_temp <<= 1;    //将编码缓存的次高位左移到最高位，进行匹配
            }
            if(writen_len == file_len)           //控制文件长度，跳出外层循环
                break;
        }

        //关闭文件
        fclose(pInfile);
        fclose(pOutfile);

        //释放动态内存空间
        free(pHuf_tree);
    }
    return 1;
}