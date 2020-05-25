#pragma once
#include"Superblock.h"
#include"Inode.h"
#include"Block.h"
#include<iostream>

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
	bool isExist(string);
	short int findInode(string);
	bool modifyBlock();
	bool createDirectory(string, short int&);
	bool createDir(vector<string>);
	void createFile();
	bool changeDir(string);

public:
	int hd_currentDirInode;	 // specify which is the current directory
	SuperBlock hd_superBlock;
	Inode hd_inodeList[INODECOUNT];
	Block hd_blockList[DATABLOCKCOUNT];
};

