#include "Compress.h"

int main(int argc, char* argv[])
{
    int option;     //操作选项
    int flag = 0;       //操作成功与否标志
    char ifname[50], ofname[50];

    /*printf("输入源文件名: ");
    fflush(stdin);
    scanf("%s", ifname);

    printf("输入目标文件名: ");
    fflush(stdin);
    scanf("%s", ofname);

    flag = Decompress(ifname, ofname);

    if(flag == -1)
        printf("输入文件名错误！\n");
    else
        printf("操作完成！\n");*/


    printf("------------------------------------\n");
    printf("----------文件压缩&解压缩程序---------\n");
    printf("------------------------------------\n");
    printf("  1.压缩文件  2.解压缩文件  3.退出程序 \n");
    printf("*************************************\n");   

    while(1)
    {
        printf("请选择操作:  "); 
        scanf("%d", &option);
        if(option != 1 && option != 2 && option != 3)
        {
            printf("输入操作错误！请输入正确操作！\n\n");
        }
        else if(option == 3)
        {
            printf("程序退出！\n");
            break;
        }
        else
        {
            printf("输入源文件名: ");
            fflush(stdin);
            scanf("%s", ifname);

            printf("输入目标文件名: ");
            fflush(stdin);
            scanf("%s", ofname);

            switch(option)
            {
            case 1:
                printf("压缩中......\n");
                flag = Compress(ifname, ofname);
                break;
            
            case 2:
                printf("解压中......\n");
                flag = Decompress(ifname, ofname);
                break;
            }

            if(flag == -1)
                printf("文件 \"%s\" 不存在！\n", ifname);
            else
                printf("操作完成！\n\n");
        }
    }

    return 0;
}