#pragma once
#include"Superblock.h"
#include"Inode.h"
#include"Block.h"

#define INODECOUNT 15694
#define DATABLOCKCOUNT 15694
#define SUPERBLOCKCOUNT 62
#define FILEMAXSIZE 351

class HardDisk
{
public:
	HardDisk();
	~HardDisk() {};
	void initiate();
	void loadHardDisk();
	void saveHardDisk();
	int findAvailableBlock();
	int findAvailableInode();
	bool isExist(string, int, int);
	short int findInode(string, int);
	bool modifyBlock();
	bool createDirectory(string, int&, int);
	bool createDir(vector<string>);
	bool deleteDirectory(int);
	bool deleteDir(vector<string>);
	void createFile();
	bool deleteFile(vector<string>);
	void releaseBlock(int);
	void releaseInode(int);
	vector<string> dir();
	bool changeDir(vector<string>);
	vector<string> split(const string& str, char delim);

public:
	int hd_currentDirInode;	 // specify which is the current directory
	string hd_currentDir;
	SuperBlock hd_superBlock;
	Inode hd_inodeList[INODECOUNT];
	Block hd_blockList[DATABLOCKCOUNT];

};

