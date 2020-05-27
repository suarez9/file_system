#include "Block.h"
#include <stdlib.h>
#include <time.h>
#include <string>

vector<string> split1(const string& str, char delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + 1;
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

Block::Block()
{
	int size = sizeof(this->content) / sizeof(this->content[0]);
	for (int i = 0; i < size; i++)
		this->content[i] = ' ';
}

vector<DirectoryBlockElement> Block::readDirectoryBlock()
{
	/*"abc.mp3|123|bgh.fuk|2|                            "
	abc.mp3
	123
	bgh.fuk
	2

	*/
	vector<DirectoryBlockElement> temps;
	string t(this->content);
	vector<string> datas = split1(t, '|');
	for (int i = 0; i < datas.size(); i += 2)
	{
		if (datas[i][0] == ' ') break;
		DirectoryBlockElement temp;
		temp.fileName = datas[i];
		temp.inodeIndex = (short int)std::stoi(datas[i + 1]);
		temps.push_back(temp);
	}
	return temps;
}

void Block::writeDirectoryBlock(string name, short int inode)
{
	int blankIndex = 0;
	while (this->content[blankIndex] != ' ') blankIndex++;
	// 写入dir name
	for (int i = 0; i < name.size(); i++)
		this->content[blankIndex + i] = name[i];
	// 添加'|'
	this->content[blankIndex + name.size()] = '|';
	blankIndex += (name.size() + 1);
	// 写入inode index
	string temp = to_string(inode);
	for (int i = 0; i < temp.size(); i++)
		this->content[blankIndex + i] = temp[i];
	// 添加'|'
	this->content[blankIndex + temp.size()] = '|';
}

string Block::readFileBlock()
{
	/*"87aA123A1B2D|                            "
	87aA123A1B2D

	*/
	string t(this->content);
	for (int i = 0; i < 1025; ++i)
		if (t[i] == '|')
			return t.substr(0, i);
}


void Block::writeFileBlock(int len)
{
	srand(time(NULL));
	for (int i = 0; i < len; ++i)
	{
		switch ((rand() % 3))
		{
		case 1:
			this->content[i] = 'A' + rand() % 26;
			break;
		case 2:
			this->content[i] = 'a' + rand() % 26;
			break;
		default:
			this->content[i] = '0' + rand() % 10;
			break;
		}
	}
	this->content[len] = '|';
}

void Block::writeFileBlock(string text)
{
	for (int i = 0; i < text.size(); i++)
	{
		this->content[i] = text[i];
	}
	this->content[text.size()] = '|';
}

vector<int> Block::readIndirectBlock()
{
	/*"87|123|1|2|                            "
	87
	123
	1
	2

	*/
	vector<int> temps;
	string t(this->content);
	vector<string> datas = split1(t, '|');
	for (int i = 0; i < datas.size(); i++)
	{
		if (datas[i][0] == ' ') break;
		temps.push_back((short int)std::stoi(datas[i]));
	}
	return temps;
}

void Block::writeIndirectBlock(short int addr)
{
	int blankIndex = 0;
	while (this->content[blankIndex] != ' ') blankIndex++;
	// 写入 addr
	string temp = to_string(addr);
	for (int i = 0; i < temp.size(); i++)
		this->content[blankIndex + i] = temp[i];
	// 添加'|'
	this->content[blankIndex + temp.size()] = '|';
}

void Block::clearBlock()
{
	int size = sizeof(this->content) / sizeof(this->content[0]);
	for (int i = 0; i < size; i++)
		this->content[i] = ' ';
}
