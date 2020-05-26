#pragma once
#include<vector>
#include<string>
#include<iostream>
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
	void writeFileBlock(int);
	void writeFileBlock(string);
	vector<int> readIndirectBlock();
	void writeIndirectBlock(short int);
	void clearBlock();
	void printBlock();

public:
	char content[9999];
};

