#include <iostream>
#include <iomanip>
#include <string>
#include <conio.h>
#include <vector>
#include <array>
#include "HardDisk.h"
#include"Superblock.h"

#define NAMEMAXSIZE 14

using namespace std;

array<string, 12> command_set = { "dir", "cp", "sum", "cat", "exit", "help",
								  "initiate",   "createFile", "deleteFile",
								  "createDir", "deleteDir",  "changeDir", };

unsigned int FindMaxSubstr(string pcStr1, string pcStr2)
{
	int num = 0;
	int n1, n2;
	int i, j, k, z;
	char a[50] = {};
	z = 0;
	n1 = pcStr1.size();
	n2 = pcStr2.size();

	for (int i = 0; i < pcStr1.size(); i++)
		pcStr1[i] = tolower(pcStr1[i]);
	

	for (int i = 0; i < pcStr2.size(); i++)
		pcStr2[i] = tolower(pcStr2[i]);
	
	for (i = 0; i < n1; i++) //遍历pcStr1
	{
		k = 1;
		for (j = 0; j < n2; j++) //遍历pcStr2
		{
			if (pcStr2[j] == pcStr1[i]) //如果遇到相同字符串就继续判断下一个字符是否匹配
			{
				z = 0; //统计匹配字符的个数，
				for (k = 0; k < n2 - j; k++)
				{
					if (pcStr2[j + k] == pcStr1[i + k]) z++;
					else break;
				}
			}
			if (z > num) //如果当前匹配字符串长度大于已存储的字符串，将新的更长的字符串放到a[]中
			{
				for (k = 0; k < z; k++)
				{
					a[k] = pcStr2[j + k];
				}
				a[k] = '\0';
				num = z;
			}
		}
	}
	return num;
}

string most_similar(string input) {
	int max_size = 0;
	int index = 0;
	for (int i = 0; i < command_set.size(); ++i) {
		int size = FindMaxSubstr(command_set[i], input);
		if (size > max_size) {
			max_size = size;
			index = i;
		}
	}
	if (max_size <= 2)
		return "no similar command";
	else
		return command_set[index];
}

int  menu()
{
	cout << "**********************************Welcome!*********************************" << endl;        
	cout << "*                              Filesystem by                              *" << endl; 
	cout << "*                           201730601035 余嘉祺                           *" << endl;
	cout << "*                           201730601035 余嘉祺                           *" << endl;
	cout << "*           0、 initiate --first time run                                 *" << endl;
	cout << "*           1、 createFile <fileName> <fileSize>  --create file           *" << endl;
	cout << "*           2、 deleteFile <fileName>  --delete file                      *" << endl;
	cout << "*           3、 createDir <dirName>  --create directory                   *" << endl;
	cout << "*           4、 deleteDir <dirName> --delete directory                    *" << endl;
	cout << "*           5、 changeDir <dirName>  --change working directory           *" << endl;
	cout << "*           6、 dir  --list all files and sub-directories                 *" << endl;
	cout << "*           7、 cp <fileName1> <fileName2>  --copy a file                 *" << endl;
	cout << "*           8、 sum  --display storage usage                              *" << endl;
	cout << "*           9、 cat <fileName>  --print out the file contents             *" << endl;
	cout << "*           10、exit  --exit                                              *" << endl;
	cout << "***************************************************************************" << endl;
	return 0;
}

void help() {
	cout << "***************************************************************************" << endl;
	cout << "*           1、 createFile <fileName> <fileSize>  --create file           *" << endl;
	cout << "*           2、 deleteFile <fileName>  --delete file                      *" << endl;
	cout << "*           3、 createDir <dirName>  --create directory                   *" << endl;
	cout << "*           4、 deleteDir <dirName> --delete directory                    *" << endl;
	cout << "*           5、 changeDir <dirName>  --change working directory           *" << endl;
	cout << "*           6、 dir  --list all files and sub-directories                 *" << endl;
	cout << "*           7、 cp <fileName1> <fileName2>  --copy a file                 *" << endl;
	cout << "*           8、 sum  --display storage usage                              *" << endl;
	cout << "*           9、 cat <fileName>  --print out the file contents             *" << endl;
	cout << "*           10、exit  --exit                                              *" << endl;
	cout << "***************************************************************************" << endl;
}

vector<string> split(const string& str, char delim)
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

bool checkFilenameStart(string fileName)
{
	char start = fileName[0];
	if (start != '/')			//if path is not started with '/'
	{
		cout << "Error! File/Directory name should start with '/'!" << endl;
		return true;
	}
	return false;
}

bool checkFilenameLength(vector<string> vec)
{
	int fileNameSize = vec.back().length();
	if (fileNameSize > NAMEMAXSIZE)		//if file name is too long
	{
		cout << "Error! File/Directory name is too long!" << endl;
		return true;
	}
	return false;
}

//global hardDisk
HardDisk *hardDisk = new HardDisk();

int main()
{
	menu();
	string s;

	// load fs

	while (1)
	{
		cout << "$ ";
		cin >> s;
		if (s == "initiate")
		{
			hardDisk->initiate();
		}
		else if (s == "createFile")
		{
			string fileName;
			string temp;
			cin >> fileName;
			cin >> temp;
			int fileSize = stoi(temp);

			if (checkFilenameStart(fileName)) continue;

			vector<string> splitString = split(fileName, '/');
			if (checkFilenameLength(splitString)) continue;

			if (fileSize > FILEMAXSIZE)		//if file size is too large, in KB
			{
				cout << "Error! File size is too large!" << endl;
				continue;
			}
			//create file, pass splitString
		}
		else if (s == "deleteFile")
		{
			string fileName;
			cin >> fileName;
			if (checkFilenameStart(fileName)) continue;

			vector<string> splitString = split(fileName, '/');
			if (checkFilenameLength(splitString)) continue;

			//delete file, pass splitString
		}
		else if (s == "createDir")
		{
			string dirName;
			cin >> dirName;
			if (checkFilenameStart(dirName)) continue;

			vector<string> splitString = split(dirName, '/');
			if (checkFilenameLength(splitString)) continue;

			hardDisk->createDir(splitString);
		}
		else if (s == "deleteDir")
		{
			string dirName;
			cin >> dirName;
			if (checkFilenameStart(dirName)) continue;

			vector<string> splitString = split(dirName, '/');
			if (checkFilenameLength(splitString)) continue;

			//delete directory, pass splitString
		}
		else if (s == "changeDir")
		{
			string dirName;
			cin >> dirName;
			if (checkFilenameStart(dirName)) continue;

			vector<string> splitString = split(dirName, '/');
			if (checkFilenameLength(splitString)) continue;

			//change directory, pass splitString
		}
		else if (s == "dir")
		{
			//dir
		}
		else if (s == "cp")
		{
			string fileName1, fileName2;
			cin >> fileName1;
			cin >> fileName2;
			if (checkFilenameStart(fileName1)) continue;
			if (checkFilenameStart(fileName2)) continue;

			vector<string> splitString1 = split(fileName1, '/');
			if (checkFilenameLength(splitString1)) continue;
			vector<string> splitString2 = split(fileName2, '/');
			if (checkFilenameLength(splitString2)) continue;

			//copy file, pass splitString1 and splitString2
		}
		else if (s == "sum")
		{
			//sum
		}
		else if (s == "cat")
		{
			string dirName;
			cin >> dirName;
			if (checkFilenameStart(dirName)) continue;

			vector<string> splitString = split(dirName, '/');
			if (checkFilenameLength(splitString)) continue;

			//print current content, pass splitString
		}
		else if (s == "exit")
		{
			break;
		}
		else if (s == "help") {
			help();
		}
		else 
		{
			cout << endl;
			cout << s << " is not a correct command. Type 'help' for help." << endl;
			cout << "The most similar command is: " << most_similar(s) << endl;
		}
		cout << endl;
	}
}