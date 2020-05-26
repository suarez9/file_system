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
	void writeDirectoryBlock(string, short int);
	string readFileBlock();
	char* readFileBlock(int);
	void writeFileBlock(int);
	void writeFileBlock(string);
	void writeFileBlock(char*);
	vector<int> readIndirectBlock();
	void writeIndirectBlock(short int);
	void clearBlock();
	void printBlock();

public:
	char content[9999];
};

