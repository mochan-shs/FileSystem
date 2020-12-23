#include "filesystem.h"


char *myvhard;             // 虚拟盘块起始地址
struct USEROPEN openfilelist[MAXOPENFILE]; //用户打开文件表数据
int curdir; //当前目录文件描述符fd
char currentdir[80];//当前目录的目录名（包括目录路径）
char *startp;//数据区的起始位置
char *starti;//i节点区的起始位置

void startsys();
void my_format();
void my_cd();
void my_mkdir();
void my_rmdir();
void my_ls();
int my_create(char *filename);
void my_rm(char *filename);
int my_open(char *filename);
void my_close();
int my_write(int fd);
int my_do_write(int fd, char *text, int len, char wstyle);
int my_read(int fd, int len);
int my_do_read(int fd, int len, char *text);
void my_exitsys();
void copytouser(int i);
int main()
{

	curdir = 0;
	strcpy(currentdir, "/");
	startsys();
	// printf("------\n");
	while (1)
	{
		printf("%s>", currentdir);
		char input[1005];
		fgets(input, sizeof(input), stdin);
		char cmd[50][100];
		char * p = strtok(input, " ");
		int i = 0;
		while (p) {
			strcpy(cmd[i], p);
			i++;
			p = strtok(NULL, " ");
		}
		for (int j = 0; cmd[i - 1][j] != '\0'; j++) {
			if (cmd[i - 1][j] == '\n') {
				cmd[i - 1][j] = '\0';
				break;
			}
				
		}
		if (strcmp(cmd[0], "my_exitsys") == 0)
		{
			my_exitsys();
			break;
		}
		else if(strcmp(cmd[0],"my_create")==0)
		{
			printf("开始创建文件\n");
			my_create(cmd[1]);
			FILE *fp = fopen(SYS_PATH, "w");
			fwrite(myvhard, SIZE, 1, fp);
		}
		else if (strcmp(cmd[0], "my_rm") == 0) {
			my_rm(cmd[1]);
		}
		else {
			printf("命令不存在!\n");
		}
	}
	system("pause");
	return 0;
}

void startsys()
{
	myvhard = (char *)malloc(SIZE);
	startp = myvhard + 18 * BLOCKSIZE;
	starti = myvhard + 2 * BLOCKSIZE;
	FILE *fp;
	int i;
	//printf("-------\n");
	fp = fopen(SYS_PATH, "r");
	if (fp == NULL)
	{
		//fclose(fp);
		printf("System is not init,now we will install it and create filesystem.\n");
		printf("Please not exit.\n");
		my_format();
		//printf("1111\n");
	}
	//fclose(fp);
	fp = fopen(SYS_PATH, "r");
	//fwrite(myvhard, SIZE, 1, fp);
	fread(myvhard, SIZE, 1, fp);
	struct iNode * p = (struct iNode *)(myvhard + 2 * BLOCKSIZE);
	LocalTime(p->creattime);
	LocalTime(p->fixtime);
	printf("filesize:%u\n", p->filesize);
	struct FCB* r = (struct FCB*) (myvhard + 18 * BLOCKSIZE);
	printf("%s\n", r->filename);
	unsigned short * index = (unsigned short *)(myvhard + 21 * BLOCKSIZE);
	while (*index != 65535) {
		printf("%u\n", *index);
		index++;
	}
	fclose(fp);
	copytouser(0, INODE_OST,PGRAPH_OST);
	printf("%s\n", openfilelist[0].dir);
	r= (struct FCB*) (myvhard + 20 * BLOCKSIZE)+1;
	printf("%s\n", r->filename);
	printf("%d\n", r->id);
	printf("%u\n", *PGRAPH_OST >> 28 & 1);
	
}

void my_format()
{
	FILE *fp = fopen(SYS_PATH, "w");
	struct BLOCK0 block;
	strcpy(block.information, "1024,1024");
	block.root = 19;
	block.startblock = startp;
	unsigned int pgraph[32];
	struct iNode * inode;
	memset(pgraph, 0, sizeof(pgraph));
	for (int i = 31; i >=11; i--) {
		pgraph[0] |= (1 << i);
	}
	//pgraph[0]=1;
	struct FCB fcb[3];
	strcpy(fcb[0].filename, "/");
	fcb[0].free = 0;
	fcb[0].id = 0;
	inode = (struct iNode *)starti;
	inode->count = 1;
	time_t t = time(NULL);
	inode->creattime = time(&t);
	inode->filesize = 0;
	inode->filetype[0] = '0';
	inode->filetype[1] = '3';
	inode->fixtime = inode->creattime;
	inode->number = 20;
	unsigned short index[2] = { 21,-1 };
	
	/*strcpy(fcb[1].filename, "usr");
	fcb[1].free = 0;
	fcb[1].id = 1;
	inode = (struct iNode *)(starti + sizeof(struct iNode));
	inode->count = 1;
	inode->creattime = time(&t);
	inode->filesize = 0;
	inode->filetype[0] = '0';
	inode->filetype[1] = '3';
	inode->fixtime = inode->creattime;
	inode->number = 21;
	strcpy(fcb[2].filename, "var");
	fcb[2].free = 0;
	fcb[2].id = 2;
	inode = (struct iNode *)(starti + 2 * sizeof(struct iNode));
	inode->count = 1;
	inode->creattime = time(&t);
	inode->filesize = 0;
	inode->filetype[0] = '0';
	inode->filetype[1] = '3';
	inode->fixtime = inode->creattime;
	inode->number = 22;*/
	fwrite(&block, BLOCKSIZE, 1, fp);
	fwrite(pgraph, BLOCKSIZE, 1, fp);
	fwrite(starti, 16 * BLOCKSIZE, 1, fp);
	fwrite(fcb, BLOCKSIZE, 1, fp);
	fwrite(index, BLOCKSIZE, 1, fp);
	fclose(fp);
	// strcmp(openfilelist[0].name,currentdir);
	// openfilelist[0].filesize=0;
	// openfilelist[0].number=20;
	// openfilelist[0].filetype=0;
	// openfilelist[0].count=1;
	// openfilelist[0].createtime=
	return;
}

void my_exitsys() {
	printf("bye\n");
	FILE * fp = fopen(SYS_PATH, "w");
	if (fp == NULL) {
		printf("保存失败\n");
		return;
	}
	fwrite(myvhard, SIZE, 1, fp);
	fclose(fp);
	printf("文件系统已保存!\n");
	free(myvhard);
}

void copytouser(int i, struct iNode *p, struct FCB* r) {
	//struct iNode *p = INODE_OST + i;
	//struct FCB* r = ROOT_OST + i;
	strcpy(openfilelist[i].filetype, p->filetype);
	openfilelist[i].count = p->count;
	openfilelist[i].filesize = p->filesize;
	openfilelist[i].number = p->number;
	openfilelist[i].creattime = p->creattime;
	openfilelist[i].fixtime = p->fixtime;
	strcpy(openfilelist[i].dir, currentdir);
	/*
	if (p->filetype[0] == '0') {
		char temp[50];
		strcpy(temp, r->filename);
		strcpy(openfilelist[i].dir, strcat(temp, "/"));
	}*/
	openfilelist[i].fcbstate = 0;
	openfilelist[i].topenfile = 1;

}

int my_create(char *filename) {
	int useropen_free = -1;
	//分配空闲用户打开表项
	int i;
	for ( i= 0; i < MAXOPENFILE; i++) {
		if (!openfilelist[i].topenfile) {
			useropen_free = i;
			break;
		}
	}
	if (i == MAXOPENFILE) {
		printf("用户文件打开数超过上限!\n");
		return -1;
	}
	int index = openfilelist[curdir].number;
	printf("%d\n", index);
	unsigned short* indexFirst = (unsigned short*)(myvhard + (index - 1)*BLOCKSIZE);
	int rename = 0;
	struct FCB * fcb_free = NULL;
	int first = 1;
	while (*indexFirst !=65535 ) {
		printf("%u\n", *indexFirst);
		struct FCB * fcb = (struct FCB *)(myvhard + (*indexFirst - 1)*BLOCKSIZE);
		for (int i = 0; i < 64; i++) {
			if (!fcb->free) {
				if (first) {
					fcb_free = fcb;
					first = 0;
				}
			}
			else {
				if (strcmp(fcb->filename, filename) == 0) {
					rename = 1;
					break;
				}
			}
			fcb++;
		}
		indexFirst++;
	}
	if (rename) {
		printf("文件名已存在!\n");
		return -1;
	}
	printf("-------------\n");
	strcpy(fcb_free->filename, filename);
	fcb_free->free = 1;
	struct iNode * p = INODE_OST;
	unsigned short  count = 0;
	while (p->count != -1) {
		if (p->count == 0) {
			break;
		}
		count++;
		p++;
	}
	fcb_free->id = count;
	printf("id:%u\n", count);
	p->count = 1;
	time_t t = time(NULL);
	p->creattime = time(&t);
	LocalTime(p->creattime);
	p->fixtime = p->creattime;
	p->filesize = 1;
	p->filetype[0] = '1';
	p->filetype[1] = '3';
	unsigned int * freebit = (unsigned int *)(myvhard + BLOCKSIZE);
	unsigned short x, y;
	int flag = 0;
	for (unsigned short i = 0; i < 32; i++) {
		for (unsigned short j = 0; j < 32; j++) {
			int b =	freebit[i] >> (31 - j) & 1;
			if (b == 0) {
				x = i+1;
				y = j+1;
				freebit[i] |= (1 << (31 - j));
				flag = 1;
				break;
			}
		}
		if (flag) break;
	}
	if (!flag) {
		printf("磁盘空间不足!创建失败\n");
		return -1;
	}
	p->number = (x-1) * 32 + y;
	printf("%u,%u", x, y);
	printf("number:%u\n", p->number);
	indexFirst = (unsigned short*)(myvhard + (p->number - 1) *BLOCKSIZE);
	flag = 0;
	for (unsigned short i = 0; i < 32; i++) {
		for (unsigned short j = 0; j < 32; j++) {
			int b = freebit[i] >> (31 - j) & 1;
			if (b == 0) {
				x = i+1;
				y = j+1;
				freebit[i] |= (1 << (31 - j));
				flag = 1;
				break;
			}
		}
		if (flag) break;
	}
	if (!flag) {
		printf("磁盘空间不足!创建失败\n");
		return -1;
	}
	printf("%u,%u", x, y);
	printf("number:%u\n", (x-1) * 32 + y);
	*indexFirst = (x - 1) * 32 + y;
	*(indexFirst + 1) = -1;
	copytouser(useropen_free, p, fcb_free);

	//修改父目录信息
	char * q = strtok(currentdir, "/");
	struct iNode * pdir = INODE_OST;
	pdir->fixtime = p->creattime;
	pdir->filesize += p->filesize;
	while (q) {
		indexFirst = (unsigned short *)(myvhard + (pdir->number - 1)*BLOCKSIZE);
		struct FCB * fcb=NULL;
		while (*indexFirst != -1) {
			fcb= (struct FCB *)(myvhard + (*indexFirst - 1)*BLOCKSIZE);
			for (int i = 0; i < 64; i++) {
				if (strcmp(fcb->filename, q) == 0) {
					break;
				}
				fcb++;
			}
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = p->creattime;
		pdir->filesize += p->filesize;
		q = strtok(NULL, "/");
	}
	return 0;
}

void my_rm(char * filename) {
	unsigned short indexnum = openfilelist[curdir].number;//索引表块号
	unsigned short * index = (unsigned short *)(myvhard + (indexnum - 1)*BLOCKSIZE);//索引表指针
	unsigned short id;
	while (*index != 65535) {
		int i = 0;
		struct FCB* fcb = (struct FCB *)(myvhard + (*index - 1)*BLOCKSIZE);
		for (i = 0; i < 64; i++) {
			if (fcb->free&&strcmp(fcb->filename,filename)==0) {
				fcb->free = 0;
				id = fcb->id;
				break;
			}
			fcb++;
		}
		if (i < 64) break;
		index++;
	}
	if (*index == 65535) {
		printf("该文件不存在\n");
		return;
	}
	struct iNode * inode = INODE_OST + id;
	inode->count--;
	time_t t = time(NULL);
	inode->fixtime = time(&t);
	printf("文件已删除\n");
	int x, y;
	unsigned int * pgraph = PGRAPH_OST;
	if (inode->count == 0) {
		index = (unsigned short *)(myvhard + (inode->number - 1)*BLOCKSIZE);
		x = inode->number / 32;
		y = inode->number % 32 - 1;
		if (y == -1) {
			y = 31;
			x -= 1;
		}
		pgraph[x] &= ~(1 << (31 - y));//释放索引表
		while (*index != 65535) {
			x = *index / 32;
			y = (*index % 32) - 1;
			if (y == -1) {
				y = 31;
				x -= 1;
			}
			pgraph[x] &= ~(1 << (31 - y));//释放文件所占盘块
			index++;
		}
	}
	//修改父目录信息
	char * q = strtok(currentdir, "/");
	struct iNode * pdir = INODE_OST;
	pdir->fixtime = inode->fixtime;
	pdir->filesize -= inode->filesize;
	while (q) {
		index = (unsigned short *)(myvhard + (pdir->number - 1)*BLOCKSIZE);
		struct FCB * fcb = NULL;
		while (*index != -1) {
			fcb = (struct FCB *)(myvhard + (*index - 1)*BLOCKSIZE);
			int i = 0;
			for (i = 0; i < 64; i++) {
				if (strcmp(fcb->filename, q) == 0) {
					break;
				}
				fcb++;
			}
			if (i < 64) break;
		}
		pdir = INODE_OST + fcb->id;
		pdir->fixtime = inode->fixtime;
		pdir->filesize -= inode->filesize;
		q = strtok(NULL, "/");
	}
}

int my_write(intfd) {

}

int do_write() {

}

