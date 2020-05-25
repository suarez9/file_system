#pragma once
#include<vector>
#include<string>
using namespace std;

#define NAMEMAXSIZE 14

struct DirectoryBlockElement
{
	string fileName;
	unsigned short int inodeIndex;
};

class Block
{
public:
	Block();
	~Block() {};
	vector<DirectoryBlockElement> readDirectoryBlock();
	//bool writeDirectoryBlock(char[]);
	void writeDirectoryBlock(string, short int);
	char* readFileBlock();
	void writeFileBlock();
	vector<int> readIndirectBlock();
	void writeIndirectBlock(short int);
	void clearBlock();
	//void genFile(int);

public:
	char content[9999];
};

