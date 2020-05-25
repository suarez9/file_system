#include "Block.h"

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
	for (int i = 0; i < 9999; i++)
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
	for (int i = 0; i < datas.size(); i+=2)
	{
		if (datas[i][0] == ' ') break;
		DirectoryBlockElement temp;
		temp.fileName = datas[i];
		temp.inodeIndex = (short int)std::stoi(datas[i+1]);
		temps.push_back(temp);
	}
	return temps;
}

bool Block::writeDirectoryBlock(char addContent[])
{
	return false;
}

char* Block::readFileBlock()
{
	return this->content;
}

void Block::writeFileBlock()
{
	
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

void Block::writeIndirectBlock()
{

}


