#pragma once
#include"Superblock.h"
#include"Inode.h"
#include"Block.h"
#include<map>

#define INODECOUNT 15694
#define DATABLOCKCOUNT 15694
#define SUPERBLOCKCOUNT 62
#define FILEMAXSIZE 522

class HardDisk
{
public:
	HardDisk();
	~HardDisk() {};
	
	void initiate();
	void reinit();
	short int findAvailableBlock();
	short int findAvailableInode();
	bool addToDirectoryBlock(int, int, string, int);
	bool deleteFromDirectoryBlock(int, int, string, int);
	bool isExist(string, int, int);
	short int findInode(string, short int, int);
	bool createDirectory(string, int&, int);
	bool createDir(vector<string>, bool);
	bool deleteDirectory(vector<string>, int);
	bool deleteDir(vector<string>, bool);
	bool createFile(vector<string>, float, bool);
	bool deleteFile(vector<string>, bool);
	bool copyFile(vector<string>, vector<string>, bool, bool);
	void releaseBlock(short int);
	void releaseInode(short int);
	void loadHardDisk();
	void saveHardDisk(int, string, string);
	void saveSystemConfig();
	string readTxt_all(string file, int size);
	int calculate_size(int);
	void changeTime();
	void dir();
	bool changeDir(vector<string>, bool);
	vector<string> split(const string& str, char delim);
	string cat(vector<string>, bool);
	void sum();
public:
	int hd_currentDirInode;	 // specify which is the current directory
	string hd_currentDir;
	SuperBlock hd_superBlock;
	Inode hd_inodeList[INODECOUNT];
	Block hd_blockList[DATABLOCKCOUNT];
	map<short int, string> Hash;

};

