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
	void saveHardDisk(int, string, string);
	void saveHardDisk2();
	int findAvailableBlock();
	int findAvailableInode();
	bool addToDirectoryBlock(int, int, string, int);
	bool isExist(string, int, int);
	short int findInode(string, int);
	bool createDirectory(string, int&, int);
	bool createDir(vector<string>);
	bool deleteDirectory(int);
	bool deleteDir(vector<string>);
	bool createFile(vector<string>, float);
	bool deleteFile(vector<string>);
	bool copyFile(vector<string>, vector<string>);
	void releaseBlock(int);
	void releaseInode(int);

	int calculate_size(int);
	void dir();
	bool changeDir(vector<string>);
	vector<string> split(const string& str, char delim);
	string cat(vector<string>);
	void sum();

public:
	int hd_currentDirInode;	 // specify which is the current directory
	string hd_currentDir;
	SuperBlock hd_superBlock;
	Inode hd_inodeList[INODECOUNT];
	Block hd_blockList[DATABLOCKCOUNT];

};

