#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
//#include"convertTime.c"

#define BLOCKSIZE 1024
#define SIZE 1024*1024
#define END 0xffff
#define FREE 0x0000
#define ROOTBLOCKNUM 2
#define MAXOPENFILE 10
#define FATNUM 2
#define BLOCK0_OST (struct BLOCK0 *)(myvhard)
#define PGRAPH_OST (unsigned int *)(myvhard+BLOCKSIZE)
#define INODE_OST (struct iNode *)(myvhard+2*BLOCKSIZE)
#define ROOT_OST (struct FCB *)(myvhard+18*BLOCKSIZE)
#define DATA_OST (char *)(myvhard+19*BLOCKSIZE)
#define MAXBUFFSIZE 20*BLOCKSIZE
#define SYS_PATH "SYSTEM.txt"

/*FCB 文件目录项 16B*/
struct FCB
{
	char filename[13];//文件名
	unsigned short id;//i节点编号
	char free;//是否为空
};

/*i节点  16B 16个盘块*/
struct iNode
{
	char filetype[2];//文件类型
	unsigned short count;//链接数
	unsigned short filesize;//文件大小
	unsigned short number;//盘块号
	unsigned int creattime;//创建时间
	unsigned int fixtime;//最近修改时间
	//char ;//保留位
};

/*USEROPEN*/
struct USEROPEN
{
	char filename[13];
	char filetype[2];//文件类型
	unsigned short count;//链接数
	unsigned short filesize;//文件大小
	unsigned short number;//盘块号
	unsigned int creattime;//创建时间
	unsigned int fixtime;//最近修改时间
	//char keep[1];//保留位
	char dir[100];  //文件路径
	unsigned int off;//读写指针
	char fcbstate;  //FCB是否被修改 1 修改 0 未修改
	char topenfile; //打开表项是否为空 0 空 1 不空
};

/*引导块*/
struct BLOCK0
{
	char information[200];      //describtion
	unsigned short root;       //根目录文件的起始盘块号
	char *startblock; //虚拟磁盘开始位置
};


