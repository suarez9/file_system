#include "HardDisk.h"
#include<iomanip>
#include<ctime>
#include<iostream>
#include<fstream>
#include <cassert>
#include <string>

HardDisk::HardDisk()
{	
}

void HardDisk::initiate()
{
	//Ϊrootռ�õ�1��inode ָ���1��block
	this->hd_superBlock.sb_freeBlockCount -= 1;
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[0] = 1;
	this->hd_superBlock.sb_blockBitmap[0] = 1;
	//��ʼ����1��inode
	this->hd_inodeList[0].i_type = 1;
	this->hd_inodeList[0].i_size = 0;

	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[0].i_ctime = now;
	this->hd_inodeList[0].i_mtime = now;
	this->hd_inodeList[0].i_daddr[0] = 690;  //data block��690��ʼ

	this->hd_currentDirInode = 0;
	this->hd_currentDir = '/';
	this->Hash.insert({0, "\\"});
}

void HardDisk::reinit()
{

	string file2 = "C:\\Users\\USER\\Desktop\\old\\config.txt";
	ifstream test_in2(file2);
	if (test_in2)
	{
		test_in2.close();
		string command = "rmdir /s/q C:\\Users\\USER\\Desktop\\old";
		system(command.c_str());
	}

	int currentInode = 0;
	vector<string>name;
	name.push_back("");
	for (int i = 0; i < 10; i++)//��ȡ10��ֱ�ӵ�ַ�е�����
	{
		if (this->hd_inodeList[currentInode].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[currentInode].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1) {
				name[0] = tempVec[j].fileName;
				deleteDir(name, false);
			}
			else {
				name[0] = tempVec[j].fileName;
				deleteFile(name, false);
			}
		
	}
	if (this->hd_inodeList[currentInode].i_idaddr == 0) {
		initiate();
		return;
	}
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[currentInode].i_idaddr - 690];
	vector<int> addresses = indirectBlock.readIndirectBlock();
	//search each block pointed by indirect address
	for (int i = 0; i < addresses.size(); i++)
	{
		Block tempBlock = this->hd_blockList[addresses[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1) {
				name[0] = tempVec[j].fileName;
				deleteDir(name, false);
			}
			else {
				name[0] = tempVec[j].fileName;
				deleteFile(name, false);
			}
	}
	initiate();
}

void HardDisk::loadHardDisk()
{
	initiate();
	ifstream infile;
	string file = "C:\\Users\\USER\\Desktop\\new\\config.txt";
	string file2 = "C:\\Users\\USER\\Desktop\\old\\config.txt";
	string root_path = "C:\\Users\\USER\\Desktop\\new";

	ifstream test_in(file);
	ifstream test_in2(file2);
	if (!test_in) {
		if (test_in2)
		{
			cout << "System crashed, maybe you quit incorrect last time, restoring the system\n ����" << endl;
			string command = "xcopy C:\\Users\\USER\\Desktop\\old C:\\Users\\USER\\Desktop\\new /e /y /i /q";
			system(command.c_str());
			cout << "Succeed\n" << endl;
		}
		else {
			string command = "mkdir C:\\Users\\USER\\Desktop\\new";
			system(command.c_str());
			saveSystemConfig();
		}
	}
	test_in2.close();

	infile.open(file.data());   //���ļ����������ļ��������� 
	assert(infile.is_open());   //��ʧ��,�����������Ϣ,����ֹ�������� 

	string s;
	vector<string> info;
	vector<string> path;
	while (getline(infile, s))
	{
		info = split(s, ' ');
		if (info[1] == "\\") {
			this->hd_inodeList[0].i_ctime = stol(info[3]);
			this->hd_inodeList[0].i_mtime = stol(info[4]);
			continue;
		}
		else {
			//�����ļ���
			if (info[0] =="1") {
				path = split(info[1], '\\');
				createDir(path, false);
				//set time
				int currentDirInode = 0;
				for (int i = 0; i < path.size(); i++)
				{
					int dirInode = findInode(path[i], currentDirInode, 1);
					currentDirInode = dirInode;
				}
				this->hd_inodeList[currentDirInode].i_ctime = stol(info[3]);
				this->hd_inodeList[currentDirInode].i_mtime = stol(info[4]);
			}
			//�����ļ�
			else {
				path = split(info[1], '\\');
				createFile(path, stoi(info[2]) / 1024, false);
				//set time 
				int currentDirInode = 0;
				for (int i = 0; i < path.size() - 1; i++)
				{
					int dirInode = findInode(path[i], currentDirInode, 1);
					currentDirInode = dirInode;
				}
				// Ѱ����Ҫ����file��inode
				int fileInode = findInode(path[path.size() - 1], currentDirInode, 0);
				this->hd_inodeList[fileInode].i_ctime = stol(info[3]);
				this->hd_inodeList[fileInode].i_mtime = stol(info[4]);
				//set content
				string content;
				string local_path = "";
				string text;
				for (int i = 0; i < path.size(); ++i)
					local_path += "\\" + path[i];
				local_path = root_path + local_path;
				content = readTxt_all(local_path, stoi(info[2]));
				int count = 0;
				for (int i = 0; i < 10; ++i) {
					if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
					text = "";
					for (int k = 0; k < 1024; ++k)
						text += content[k + count];
					this->hd_blockList[this->hd_inodeList[fileInode].i_daddr[i] - 690].writeFileBlock(text);
					count += 1024;
				}
				
				if (this->hd_inodeList[fileInode].i_idaddr != 0)
				{
					Block indirectBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
					vector<int> addresses = indirectBlock.readIndirectBlock();
					//search each block pointed by indirect address
					for (int i = 0; i < addresses.size(); i++)
					{
						text = "";
						for (int k = 0; k < 1024; ++k)
							text += content[k + count];
						this->hd_blockList[addresses[i] - 690].writeFileBlock(text);
						count += 1024;
					}
				}
			}
		}
	}
	infile.close();             //�ر��ļ������� 
	changeTime();
}

void HardDisk::saveHardDisk(int inodeIndex, string absolute_path, string local_path)
{
	string name;
	string command;
	//Search each block pointed by direct address and save the information
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
			//If it is a directory, recursivly call the function
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
			{
				name = local_path + tempVec[j].fileName;
				this->Hash.insert({ tempVec[j].inodeIndex , name });

				name = absolute_path + tempVec[j].fileName;
				command = "mkdir " + name;
				system(command.c_str());
				string new_absolute_path = name + "\\";
				string new_local_path = local_path + tempVec[j].fileName + "\\";
				saveHardDisk(tempVec[j].inodeIndex, new_absolute_path, new_local_path);
			}
			//If it is a file, record the information directly.
			else
			{
				name = local_path + tempVec[j].fileName;
				this->Hash.insert({ tempVec[j].inodeIndex , name });
				string text = cat(split(name, '\\'), false);

				name = absolute_path + tempVec[j].fileName;
				ofstream out(name);
				if (!out)
					cerr << "Error in storage to the computer." << endl;
				out << text << endl;
			}
	}
	//Search each block pointed by indirect address and save information
	//The save procedure are similar to the above direct address
	if (this->hd_inodeList[inodeIndex].i_idaddr != 0)
	{
		Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
		vector<int> addresses = indirectBlock.readIndirectBlock();
		//search each block pointed by indirect address
		for (int i = 0; i < addresses.size(); i++)
		{
			Block tempBlock = this->hd_blockList[addresses[i] - 690];
			vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
			//search whether this block contains the specific pair
			for (int j = 0; j < tempVec.size(); j++)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
				{
					name = local_path + tempVec[j].fileName;
					this->Hash.insert({ tempVec[j].inodeIndex , name });

					name = absolute_path + tempVec[j].fileName;
					command = "mkdir " + name;
					system(command.c_str());
					string new_absolute_path = name + "\\";
					string new_local_path = local_path + tempVec[j].fileName + "\\";
					saveHardDisk(tempVec[j].inodeIndex, new_absolute_path, new_local_path);
				}
				else
				{
					name = local_path + tempVec[j].fileName;
					this->Hash.insert({ tempVec[j].inodeIndex , name });
					string text = cat(split(name, '\\'), false);

					name = absolute_path + tempVec[j].fileName;
					ofstream out(name);
					if (!out)
						cerr << "Error in storage to the computer." << endl;
					out << text << endl;
				}
		}
	}
}

void HardDisk::saveSystemConfig()
{
	
	ofstream out("C:\\Users\\USER\\Desktop\\new\\config.txt");
	if (!out)
		cerr << "Error in storage to the computer." << endl;
	string name;
	int inodeIndex = -1;

	map<short int, string>::iterator it;
	for (it = Hash.begin(); it != Hash.end(); it++) {
		name = it->second;
		inodeIndex = it->first;
		out << setw(4) << left << this->hd_inodeList[inodeIndex].i_type;
		out << setw(20) << left << name;
		out << setw(10) << left << this->hd_inodeList[inodeIndex].i_size;
		out << setw(15) << left << this->hd_inodeList[inodeIndex].i_ctime;
		out << setw(15) << left << this->hd_inodeList[inodeIndex].i_mtime;
		out << endl;
	}
}

short int HardDisk::findAvailableBlock()
{
	if (this->hd_superBlock.sb_freeBlockCount == 0) return -1;
	int availableBlock = 0;
	for (int i = 0; i < DATABLOCKCOUNT; i++)
	{
		if (this->hd_superBlock.sb_blockBitmap[i] == 0)
		{
			availableBlock = i;
			break;
		}
	}
	return availableBlock;
}

short int HardDisk::findAvailableInode()
{
	if (this->hd_superBlock.sb_freeInodeCount == 0) return -1;
	int availableInode = 0;
	for (int i = 0; i < INODECOUNT; i++)
	{
		if (this->hd_superBlock.sb_inodeBitmap[i] == 0)
		{
			availableInode = i;
			break;
		}
	}
	return availableInode;
}

bool HardDisk::addToDirectoryBlock(int currentSize, int fatherInode, string name, int sonInode)
{
	//�ı�fatherInode��block ������pair
	//��dir����д��10��ֱ�ӵ�ַ��block��
	if (currentSize + 16 <= 10 * 1024)
	{
		//�µ�dirҪ����ڵڼ���daddrָ���block
		int daddrIndex = (currentSize + 16 - 1) / 1024;
		//��inodeҪռ���µ�block
		if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
		if (currentSize % 1024 == 0 && currentSize != 0)
		{
			//find available block
			int availableBlock = findAvailableBlock();
			if (availableBlock == -1) return false;
			this->hd_inodeList[fatherInode].i_daddr[daddrIndex] = availableBlock + 690;
			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		}
		int writeBlockAddr = this->hd_inodeList[fatherInode].i_daddr[daddrIndex];
		this->hd_blockList[writeBlockAddr - 690].writeDirectoryBlock(name, sonInode);
		//this->hd_blockList[writeBlockAddr - 690].printBlock();
	}
	//��dirҪд��idaddrָ���block��
	else
	{
		//��inodeҪ��������idaddr��block
		if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
		if (this->hd_inodeList[fatherInode].i_idaddr == 0)
		{
			//find available block
			int availableBlock = findAvailableBlock();
			if (availableBlock == -1) return false;

			this->hd_inodeList[fatherInode].i_idaddr = availableBlock + 690;
			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		}

		//��inodeҪռ���µ�idaddrָ���block
		if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
		if (currentSize % 1024 == 0)
		{
			//find available block
			int availableBlock = findAvailableBlock();
			if (availableBlock == -1) return false;

			int idaddr = this->hd_inodeList[fatherInode].i_idaddr;
			//����ռ�õ�block�ĵ�ַд��idaddr block
			this->hd_blockList[idaddr - 690].writeIndirectBlock(availableBlock + 690);
			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		}
		//�µ�dirҪ�����idaddrָ��ĵڼ���block
		int idaddrIndex = (currentSize + 16 - 1) / 1024 - 10;
		Block tempBlock = this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690];
		vector<int> tempVec = tempBlock.readIndirectBlock();
		this->hd_blockList[tempVec[idaddrIndex] - 690].writeDirectoryBlock(name, sonInode);
	}
}

bool HardDisk::deleteFromDirectoryBlock(int currentSize, int fatherInode, string name, int type)
{
	//�ı�fatherInode��block ɾ��pair
	//�ض�pair��10��ֱ�ӵ�ַ��block��
	int locateBlockAddr = 0;
	int lastBlockAddr = this->hd_inodeList[fatherInode].i_daddr[(currentSize - 1) / 1024];
	vector<DirectoryBlockElement> locateBlockVec;
	if (currentSize <= 10 * 1024)
	{
		for (int i = 0; i < 10; i++)
		{
			if (this->hd_inodeList[fatherInode].i_daddr[i] == 0) break;
			// get the current dir's inode's block
			Block tempBlock = this->hd_blockList[this->hd_inodeList[fatherInode].i_daddr[i] - 690];
			locateBlockVec = tempBlock.readDirectoryBlock();
			//search whether this block contains the specific name inode index pair
			for (int j = 0; j < locateBlockVec.size(); j++)
				if (name == locateBlockVec[j].fileName)
					if (this->hd_inodeList[locateBlockVec[j].inodeIndex].i_type == type)
					{
						locateBlockAddr = this->hd_inodeList[fatherInode].i_daddr[i];
						break;
					}
		}
		Block tempBlock = this->hd_blockList[lastBlockAddr - 690];
		vector<DirectoryBlockElement> lastBlockVec = tempBlock.readDirectoryBlock();

		if (locateBlockAddr != lastBlockAddr)
		{
			// ɾ���ض���pair ����lastBlock�ƶ����1��Ԫ�ص�locateBlock���ȱ
			locateBlockVec.push_back(lastBlockVec.back());
			// ��дlocateBlock
			this->hd_blockList[locateBlockAddr - 690].clearBlock();
			for (int i = 0; i < locateBlockVec.size(); i++)
				//  ֻ��ӳ��ض�pair�����pair
				if (locateBlockVec[i].fileName != name || this->hd_inodeList[locateBlockVec[i].inodeIndex].i_type != type)
					this->hd_blockList[locateBlockAddr - 690].writeDirectoryBlock(locateBlockVec[i].fileName, locateBlockVec[i].inodeIndex);
			// ��дlastBlock
			this->hd_blockList[lastBlockAddr - 690].clearBlock();
			// ���lastBlock����pair 
			if ((currentSize - 16) % 1024 != 0)
			{
				// ֻ��ӳ����1��pair֮���pair
				for (int i = 0; i < lastBlockVec.size() - 1; i++)
					//  ֻ��ӳ��ض�pair�����pair
					if (lastBlockVec[i].fileName != name || this->hd_inodeList[lastBlockVec[i].inodeIndex].i_type != type)
						this->hd_blockList[lastBlockAddr - 690].writeDirectoryBlock(lastBlockVec[i].fileName, lastBlockVec[i].inodeIndex);
			}
			else if (currentSize - 16 != 0)
			{
				releaseBlock(lastBlockAddr - 690);
				this->hd_inodeList[fatherInode].i_daddr[(currentSize - 1) / 1024] = 0;
			}
		}
		else
		{
			// ��дlocateBlock
			this->hd_blockList[locateBlockAddr - 690].clearBlock();
			// ���locateBlock����pair 
			if ((currentSize - 16) % 1024 != 0)
			{
				for (int i = 0; i < locateBlockVec.size(); i++)
					//  ֻ��ӳ��ض�pair�����pair
					if (locateBlockVec[i].fileName != name || this->hd_inodeList[locateBlockVec[i].inodeIndex].i_type != type)
						this->hd_blockList[locateBlockAddr - 690].writeDirectoryBlock(locateBlockVec[i].fileName, locateBlockVec[i].inodeIndex);
			}
			else if (currentSize - 16 != 0)
			{
				releaseBlock(locateBlockAddr - 690);
				this->hd_inodeList[fatherInode].i_daddr[(currentSize - 1) / 1024] = 0;
			}
		}
	}
	//pair��idaddrָ���block��
	else
	{
		vector<DirectoryBlockElement> locateBlockVec;
		vector<int> idaddrVec = this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690].readIndirectBlock();
		int locateBlockAddr = 0;
		int lastBlockAddr = idaddrVec.back();
		// ��pair��10��ֱ�ӵ�ַָ���block��
		for (int i = 0; i < 10; i++)
		{
			if (this->hd_inodeList[fatherInode].i_daddr[i] == 0) break;
			// get the current dir's inode's block
			Block tempBlock = this->hd_blockList[this->hd_inodeList[fatherInode].i_daddr[i] - 690];
			locateBlockVec = tempBlock.readDirectoryBlock();
			//search whether this block contains the specific name inode index pair
			for (int j = 0; j < locateBlockVec.size(); j++)
				if (name == locateBlockVec[j].fileName)
					if (this->hd_inodeList[locateBlockVec[j].inodeIndex].i_type == type)
					{
						locateBlockAddr = this->hd_inodeList[fatherInode].i_daddr[i];
						break;
					}
		}
		// ��pair��idaddr���ָ���block��
		if (locateBlockAddr == 0)
		{
			for (int i = 0; i < idaddrVec.size(); i++)
			{
				Block tempBlock = this->hd_blockList[idaddrVec[i] - 690];
				locateBlockVec = tempBlock.readDirectoryBlock();
				//search whether this block contains the specific name inode index pair
				for (int j = 0; j < locateBlockVec.size(); j++)
					if (name == locateBlockVec[j].fileName)
						if (this->hd_inodeList[locateBlockVec[j].inodeIndex].i_type == type)
						{
							locateBlockAddr = idaddrVec[i];
							break;
						}
			}
		}
		Block tempBlock = this->hd_blockList[lastBlockAddr - 690];
		vector<DirectoryBlockElement> lastBlockVec = tempBlock.readDirectoryBlock();

		if (locateBlockAddr != lastBlockAddr)
		{
			// ɾ���ض���pair ����lastBlock�ƶ����1��Ԫ�ص�locateBlock���ȱ
			locateBlockVec.push_back(lastBlockVec.back());
			// ��дlocateBlock
			this->hd_blockList[locateBlockAddr - 690].clearBlock();
			for (int i = 0; i < locateBlockVec.size(); i++)
				//  ֻ��ӳ��ض�pair�����pair
				if (locateBlockVec[i].fileName != name || this->hd_inodeList[locateBlockVec[i].inodeIndex].i_type != type)
					this->hd_blockList[locateBlockAddr - 690].writeDirectoryBlock(locateBlockVec[i].fileName, locateBlockVec[i].inodeIndex);
			// ��дlastBlock ���lastBlock����pair 
			this->hd_blockList[lastBlockAddr - 690].clearBlock();
			if ((currentSize - 16) % 1024 != 0)
			{
				// ֻ��ӳ����1��pair֮���pair
				for (int i = 0; i < lastBlockVec.size() - 1; i++)
					//  ֻ��ӳ��ض�pair�����pair
					if (lastBlockVec[i].fileName != name || this->hd_inodeList[lastBlockVec[i].inodeIndex].i_type != type)
						this->hd_blockList[lastBlockAddr - 690].writeDirectoryBlock(lastBlockVec[i].fileName, lastBlockVec[i].inodeIndex);
			}
			else
			{
				releaseBlock(lastBlockAddr - 690);
				// ���ɾ���� idaddr���ָ���block����pair
				if ((currentSize - 16) > 10 * 1024)
				{
					// ����indirectBlock
					this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690].clearBlock();
					for (int j = 0; j < idaddrVec.size() - 1; j++)
						this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690].writeIndirectBlock(idaddrVec[j]);
				}
				else
				{
					releaseBlock(this->hd_inodeList[fatherInode].i_idaddr - 690);
					this->hd_inodeList[fatherInode].i_idaddr = 0;
				}
			}
		}
		else
		{
			// ��дlocateBlock
			this->hd_blockList[locateBlockAddr - 690].clearBlock();
			// ���locateBlock����pair 
			if ((currentSize - 16) % 1024 != 0)
			{
				for (int i = 0; i < locateBlockVec.size(); i++)
					//  ֻ��ӳ��ض�pair�����pair
					if (locateBlockVec[i].fileName != name || this->hd_inodeList[locateBlockVec[i].inodeIndex].i_type != type)
						this->hd_blockList[locateBlockAddr - 690].writeDirectoryBlock(locateBlockVec[i].fileName, locateBlockVec[i].inodeIndex);
			}
			else
			{
				releaseBlock(locateBlockAddr - 690);
				// ���ɾ���� idaddr���ָ���block����pair
				if ((currentSize - 16) > 10 * 1024)
				{
					// ����indirectBlock
					this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690].clearBlock();
					for (int j = 0; j < idaddrVec.size() - 1; j++)
						this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690].writeIndirectBlock(idaddrVec[j]);
				}
				else
				{
					releaseBlock(this->hd_inodeList[fatherInode].i_idaddr - 690);
					this->hd_inodeList[fatherInode].i_idaddr = 0;
				}
			}
		}
	}
	return true;
}

// �жϵ�ǰdir �Ƿ���ڸ�����name 
bool HardDisk::isExist(string name, int inodeIndex, int type)
{
	//Search each block pointed by direct address
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific name inode index pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == type)
					return true;
	}
	//If the current i-node do not have indirect address, return
	if (this->hd_inodeList[inodeIndex].i_idaddr == 0) return false;
	//Get the information of indirect block
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
	vector<int> addresses = indirectBlock.readIndirectBlock();
	//Search each block pointed by indirect address
	for (int i = 0; i < addresses.size(); i++)
	{
		Block tempBlock = this->hd_blockList[addresses[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == type)
					return true;
	}
	//If not found, return false
	return false;
}

// Ѱ�Ҹ���name��Ӧ��inode index
short int HardDisk::findInode(string name, short int inodeIndex, int type)
{
	//Search each block pointed by direct address
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == type)
					return tempVec[j].inodeIndex;
	}
	//If the current i-node do not have indirect address, return
	if (this->hd_inodeList[inodeIndex].i_idaddr == 0) return -1;
	//Get the information of indirect block
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
	vector<int> addresses = indirectBlock.readIndirectBlock();
	//Search each block pointed by indirect address
	for (int i = 0; i < addresses.size(); i++)
	{
		Block tempBlock = this->hd_blockList[addresses[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == type)
					return tempVec[j].inodeIndex;
	}
	//If not found, return -1
	return -1;
}

bool HardDisk::createDirectory(string name, int& sonInode, int fatherInode)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount == 0) {
		cout << "No free iNode or no free block in creating directory" << endl;
		return false;
	}

	int currentSize = this->hd_inodeList[fatherInode].i_size;
	//��������ռ������µ�dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) {
		cout << "Size error in creating Directory" << endl;
		return false;
	}

	//find available inode
	int availableInode = findAvailableInode();
	if (availableInode == -1) {
		cout << "No available Inode in creating directory" << endl;
		return false;
	}
	sonInode = availableInode;
	addToDirectoryBlock(currentSize, fatherInode, name, sonInode);

	//find available block
	int availableBlock = findAvailableBlock();
	if (availableBlock == -1) {
		cout << "No available block in creating directory" << endl;
		return false;
	}

	//change hardDisk state
	this->hd_superBlock.sb_freeBlockCount -= 1;
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[sonInode] = 1;
	this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;

	//��ʼ��inode
	this->hd_inodeList[sonInode].i_type = 1;
	this->hd_inodeList[sonInode].i_size = 0;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[sonInode].i_ctime = now;
	this->hd_inodeList[sonInode].i_mtime = now;
	this->hd_inodeList[sonInode].i_daddr[0] = availableBlock + 690;

	//�ı丸inode
	this->hd_inodeList[fatherInode].i_size += 16;
	this->hd_inodeList[fatherInode].i_mtime = now;

	return true;
}

// ��������path���ж��δ������dir ���ϵ���createDirectory
bool HardDisk::createDir(vector<string> paths, bool type)
{
	int currentDirInode = 0;
	string local_path;
	if (type == true) {
		currentDirInode = this->hd_currentDirInode;
		if (currentDirInode != 0)
			local_path = this->hd_currentDir;
	}
	int count = 0;
	// for all the dirs in paths
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, create, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1))
		{
			int dirInode = 0;
			if (createDirectory(paths[i], dirInode, currentDirInode))
				currentDirInode = dirInode;
			else
				return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return false;
			local_path += "/" + paths[i];
			currentDirInode = dirInode;
			count += 1;
		}
	}
	if (count == paths.size())
	{
		cout << "Dir: " << local_path << " is exist, change dir to this directory" << endl;
		this->hd_currentDir = local_path;
		this->hd_currentDirInode = currentDirInode;
	}
	return true;
}

bool HardDisk::deleteDirectory(vector<string> paths, int currentDirInode)
{
	// ɾ��10��ֱ�ӵ�ַָ�������
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[currentDirInode].i_daddr[i] == 0) break;
		Block tempBlock = this->hd_blockList[this->hd_inodeList[currentDirInode].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
		{
			// if it is a file
			vector<string> newPaths = paths;
			newPaths.push_back(tempVec[j].fileName);
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 0)
				deleteFile(newPaths, false);
			else
				deleteDirectory(newPaths, tempVec[j].inodeIndex);
		}
	}
	// ɾ����ӵ�ַָ�������
	if (this->hd_inodeList[currentDirInode].i_idaddr != 0)
	{
		Block tempBlock = this->hd_blockList[this->hd_inodeList[currentDirInode].i_idaddr - 690];
		vector<int> idaddrVec = tempBlock.readIndirectBlock();
		for (int i = 0; i < idaddrVec.size(); i++)
		{
			Block tempBlock = this->hd_blockList[idaddrVec[i] - 690];
			vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
			for (int j = 0; j < tempVec.size(); j++)
			{
				// if it is a file
				vector<string> newPaths = paths;
				newPaths.push_back(tempVec[j].fileName);
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 0)
					deleteFile(newPaths, false);
				else
					deleteDirectory(newPaths, tempVec[j].inodeIndex);
			}
		}
	}

	// ��ȡ��dir��fatherDirInode
	int fatherDirInode = 0;
	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths[i], fatherDirInode, 1)) return false;
		else
		{
			int dirInode = findInode(paths[i], fatherDirInode, 1);
			if (dirInode == -1) return false;
			fatherDirInode = dirInode;
		}
	}
	// �޸�dir��directoryBlock
	int currentSize = this->hd_inodeList[fatherDirInode].i_size;
	deleteFromDirectoryBlock(currentSize, fatherDirInode, paths[paths.size() - 1], 1);

	// �ͷ�daddrָ���block
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[currentDirInode].i_daddr[i] == 0) break;
		releaseBlock(this->hd_inodeList[currentDirInode].i_daddr[i] - 690);
	}
	// �ͷ�idaddr���ָ���block
	if (this->hd_inodeList[currentDirInode].i_idaddr != 0)
	{
		Block tempBlock = this->hd_blockList[this->hd_inodeList[currentDirInode].i_idaddr - 690];
		vector<int> tempVec = tempBlock.readIndirectBlock();
		for (int j = 0; j < tempVec.size(); j++)
			releaseBlock(tempVec[j] - 690);
		// �ͷ�indirectblock
		releaseBlock(this->hd_inodeList[currentDirInode].i_idaddr - 690);
	}
	// �ͷ�dir��inode
	releaseInode(currentDirInode);

	//�ı丸inode
	this->hd_inodeList[fatherDirInode].i_size -= 16;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[fatherDirInode].i_mtime = now;

	return true;
}

bool HardDisk::deleteDir(vector<string> paths, bool type)
{
	// Ѱ����Ҫɾ��dir��inode
	int currentDirInode = 0;
	string local_path;
	if (type == true)
	{
		currentDirInode = this->hd_currentDirInode;
		if (currentDirInode != 0)
			local_path = this->hd_currentDir;
	}

	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1)) {
			local_path += "/" + paths[i];
			cout << "Dir: " << local_path << " does not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
			local_path += "/" + paths[i];
		}
	}
	//���Ҫɾ����dir��current working directory, return false
	if (currentDirInode == this->hd_currentDirInode) {
		cout << "Can not delete the working directory" << endl;
		return false;
	}
	else
	{
		if (type == true)
			paths = split(local_path, '/');
		deleteDirectory(paths, currentDirInode);
	}
	return true;
}

bool HardDisk::createFile(vector<string> paths, float size, bool type)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount < size) {
		cout << "No free iNode or block in creating file" << endl;
		return false;
	}
	string local_path;
	// ����file���ڵ�dir
	int currentDirInode = 0;
	if (type == true) {
		currentDirInode = this->hd_currentDirInode;
		if (currentDirInode != 0)
			local_path = this->hd_currentDir;
	}

	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1)) {
			local_path += "/" + paths[i];
			cout << "Dir: " << local_path << " does not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
			local_path += "/" + paths[i];
		}
	}

	int fileInode = findInode(paths[paths.size() - 1], currentDirInode, 0);
	local_path += "/" + paths[paths.size() - 1];
	if (fileInode != -1) {
		cout << "File: "<<local_path<<" exist" << endl;
		return false;
	}

	int currentSize = this->hd_inodeList[currentDirInode].i_size;
	//��������ռ������µ�dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) return false;

	//find available inode
	int availableInode = findAvailableInode();
	if (availableInode == -1) {
		cout << "No available iNode in creating file" << endl;
		return false;
	}
	addToDirectoryBlock(currentSize, currentDirInode, paths[paths.size() - 1], availableInode);
	//change hardDisk state
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[availableInode] = 1;

	//��ʼ��inode
	this->hd_inodeList[availableInode].i_type = 0;
	int s = (int)(size * 1024);
	this->hd_inodeList[availableInode].i_size = s;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[availableInode].i_ctime = now;
	this->hd_inodeList[availableInode].i_mtime = now;

	//write blocks
	if (this->hd_inodeList[availableInode].i_size > 10 * 1024)
	{
		//find available block
		int availableBlock = findAvailableBlock();
		if (availableBlock == -1) {
			cout << "No available block in creating file" << endl;
			return false;
		}
		//change hardDisk state
		this->hd_superBlock.sb_freeBlockCount -= 1;
		this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		//change inode state
		this->hd_inodeList[availableInode].i_idaddr = availableBlock + 690;
	}
	int totalSize = 0;
	while (totalSize < this->hd_inodeList[availableInode].i_size)
	{
		//daddr
		if (totalSize < 10 * 1024)
		{
			//find available block
			int availableBlock = findAvailableBlock();
			if (availableBlock == -1) {
				cout << "No available block in creating file" << endl;
				return false;
			}
			int len = (this->hd_inodeList[availableInode].i_size - totalSize >= 1024) ? 1024 : (this->hd_inodeList[availableInode].i_size - totalSize);
			this->hd_blockList[availableBlock].writeFileBlock(len);

			totalSize += len;

			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
			//change inode state
			this->hd_inodeList[availableInode].i_daddr[(totalSize - 1) / 1024] = availableBlock + 690;
		}
		//idaddr
		else
		{
			//find available block
			int availableBlock = findAvailableBlock();
			if (availableBlock == -1) {
				cout << "No available block in creating file" << endl;
				return false;
			}
			int len = (this->hd_inodeList[availableInode].i_size - totalSize >= 1024) ? 1024 : (this->hd_inodeList[availableInode].i_size - totalSize);
			this->hd_blockList[availableBlock].writeFileBlock(len);

			totalSize += len;

			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
			//change inode state
			this->hd_blockList[this->hd_inodeList[availableInode].i_idaddr - 690].writeIndirectBlock(availableBlock + 690);
		}
	}
	//�ı丸inode
	this->hd_inodeList[currentDirInode].i_size += 16;
	this->hd_inodeList[currentDirInode].i_mtime = now;
	return true;
}

bool HardDisk::deleteFile(vector<string> paths, bool type)
{
	// ����file���ڵ�dir
	int currentDirInode = 0;
	string local_path;
	if (type == true)
	{
		currentDirInode = this->hd_currentDirInode;
		if (currentDirInode != 0)
			local_path = this->hd_currentDir;
	}
	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1)) {
			local_path += "/" + paths[i];
			cout << "Dir: " << local_path << " does not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) {
				return false;
			}
			local_path += "/" + paths[i];
			currentDirInode = dirInode;
		}
	}
	// Ѱ����Ҫɾ��file��inode
	int fileInode = findInode(paths[paths.size() - 1], currentDirInode, 0);
	local_path += "/" + paths[paths.size() - 1];
	// �Ҳ���file
	if (fileInode == -1) {
		cout << "File " << local_path << " not exist" << endl;
		return false;
	}
	// �ͷ�daddrָ���block
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
		releaseBlock(this->hd_inodeList[fileInode].i_daddr[i] - 690);
	}
	// �ͷ�idaddr���ָ���block
	if (this->hd_inodeList[fileInode].i_idaddr != 0)
	{
		Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
		vector<int> tempVec = tempBlock.readIndirectBlock();
		for (int j = 0; j < tempVec.size(); j++)
			releaseBlock(tempVec[j] - 690);
		// �ͷ�indirectblock
		releaseBlock(this->hd_inodeList[fileInode].i_idaddr - 690);
	}
	// �ͷ�file��inode
	releaseInode(fileInode);

	// �޸�dir��directoryBlock
	int currentSize = this->hd_inodeList[currentDirInode].i_size;
	deleteFromDirectoryBlock(currentSize, currentDirInode, paths[paths.size() - 1], 0);

	//�ı丸inode
	this->hd_inodeList[currentDirInode].i_size -= 16;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[currentDirInode].i_mtime = now;
	return true;
}

bool HardDisk::copyFile(vector<string> paths1, vector<string> paths2, bool type1, bool type2)
{
	// ����file1���ڵ�dir
	int currentDirInode1 = 0;
	string local_path1;
	if (type1 == true)
	{
		currentDirInode1 = this->hd_currentDirInode;
		if (currentDirInode1 != 0)
			local_path1 = this->hd_currentDir;
	}
	for (int i = 0; i < paths1.size() - 1; i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths1[i], currentDirInode1, 1)) {
			local_path1 += "/" + paths1[i];
			cout << "Dir: " << local_path1 << " does not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths1[i], currentDirInode1, 1);
			if (dirInode == -1) return false;
			currentDirInode1 = dirInode;
			local_path1 += "/" + paths1[i];
		}
	}
	// Ѱ����Ҫ����file��inode
	int fileInode = findInode(paths1[paths1.size() - 1], currentDirInode1, 0);
	local_path1 += "/" + paths1[paths1.size() - 1];
	if (fileInode == -1) {
		cout << "File " << local_path1 << " not exist" << endl;
		return false;
	}
	int size = this->hd_inodeList[fileInode].i_size;

	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount < size / 1024) {
		cout << "No free block or iNode in cp file" << endl;
		return false;
	}

	// ����file2���ڵ�dir
	int currentDirInode2 = 0;
	string local_path2;
	if (type2 == true)
	{
		currentDirInode2 = this->hd_currentDirInode;
		if (currentDirInode2 != 0)
			local_path2 = this->hd_currentDir;
	}
	for (int i = 0; i < paths2.size() - 1; i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths2[i], currentDirInode2, 1)) { 
			local_path2 += "/" + paths2[i];
			cout << "Dir: " << local_path2 << " not exist" << endl;
			return false; 
		}
		else
		{
			int dirInode = findInode(paths2[i], currentDirInode2, 1);
			if (dirInode == -1) return false;
			currentDirInode2 = dirInode;
			local_path2 += "/" + paths2[i];
		}
	}
	if (isExist(paths2[paths2.size() - 1], currentDirInode2, 0))
	{
		cout << "File " + local_path2 + " already exists!" << endl;
		return false;
	}

	int currentSize = this->hd_inodeList[currentDirInode2].i_size;
	//��������ռ������µ�dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) {
		cout << "No enough space in cp file" << endl;
		return false;
	}
	//find available inode
	int availableInode = findAvailableInode();
	if (availableInode == -1) {
		cout << "No available iNode in cp file" << endl;
		return false;
	}

	addToDirectoryBlock(currentSize, currentDirInode2, paths2[paths2.size() - 1], availableInode);

	//change hardDisk state
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[availableInode] = 1;

	//��ʼ��inode
	this->hd_inodeList[availableInode].i_type = 0;
	this->hd_inodeList[availableInode].i_size = size;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[availableInode].i_ctime = now;
	this->hd_inodeList[availableInode].i_mtime = now;

	//write blocks
	if (this->hd_inodeList[availableInode].i_size > 10 * 1024)
	{
		//find available block
		int availableBlock = findAvailableBlock();
		if (availableBlock == -1) {
			cout << "No available block in cp file" << endl;
			return false;
		}
		//change hardDisk state
		this->hd_superBlock.sb_freeBlockCount -= 1;
		this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		//change inode state
		this->hd_inodeList[availableInode].i_idaddr = availableBlock + 690;
	}
	//daddr
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
		//find available block
		int availableBlock = findAvailableBlock();
		if (availableBlock == -1) {
			cout << "No available block in cp file" << endl;
			return false;
		}
		string text = this->hd_blockList[this->hd_inodeList[fileInode].i_daddr[i] - 690].readFileBlock();
		this->hd_blockList[availableBlock].writeFileBlock(text);
		//change hardDisk state
		this->hd_superBlock.sb_freeBlockCount -= 1;
		this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		//change inode state
		this->hd_inodeList[availableInode].i_daddr[i] = availableBlock + 690;
	}
	//idaddr
	if (this->hd_inodeList[fileInode].i_idaddr == 0) {
		this->hd_inodeList[currentDirInode2].i_size += 16;
		this->hd_inodeList[currentDirInode2].i_mtime = now;
		return true;
	}
	Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
	vector<int> tempVec = tempBlock.readIndirectBlock();
	for (int i = 0; i < tempVec.size(); i++)
	{
		//find available block
		int availableBlock = findAvailableBlock();
		if (availableBlock == -1) {
			cout << "No available block in cp file" << endl;
			return false;
		}
		string text = this->hd_blockList[tempVec[i] - 690].readFileBlock();
		this->hd_blockList[availableBlock].writeFileBlock(text);
		//change hardDisk state
		this->hd_superBlock.sb_freeBlockCount -= 1;
		this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		//change inode state
		this->hd_blockList[this->hd_inodeList[availableInode].i_idaddr - 690].writeIndirectBlock(availableBlock + 690);
	}

	//�ı丸inode
	this->hd_inodeList[currentDirInode2].i_size += 16;
	this->hd_inodeList[currentDirInode2].i_mtime = now;
	return true;
}

void HardDisk::releaseBlock(short int blockIndex)
{
	this->hd_blockList[blockIndex].clearBlock();
	//�޸�superBlock
	this->hd_superBlock.sb_blockBitmap[blockIndex] = 0;
	this->hd_superBlock.sb_freeBlockCount += 1;
}

void HardDisk::releaseInode(short int inodeIndex)
{
	this->hd_inodeList[inodeIndex].clearInode();
	//�޸�superBlock
	this->hd_superBlock.sb_inodeBitmap[inodeIndex] = 0;
	this->hd_superBlock.sb_freeInodeCount += 1;
}

string HardDisk::readTxt_all(string file, int size)//���ڶ�ȡ�Ѿ������ڵ���Ӳ���е��ļ�����������ϵͳ
{
	ifstream infile;
	infile.open(file.data());   //���ļ����������ļ��������� 
	assert(infile.is_open());   //��ʧ��,�����������Ϣ,����ֹ�������� 
	int count = 0;
	char c;
	string content = "";
	while (count < size)
	{
		infile >> c;
		content += c;
		count += 1;
	}
	infile.close();             //�ر��ļ������� 
	return content;
}

int HardDisk::calculate_size(int inodeIndex)//�ݹ麯�������ڼ���dir���ļ��Ĵ�С
{
	int total_size = 0;
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
				total_size += calculate_size(tempVec[j].inodeIndex);
			else
				total_size += this->hd_inodeList[tempVec[j].inodeIndex].i_size;
	}

	if (this->hd_inodeList[inodeIndex].i_idaddr != 0)
	{
		Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
		vector<int> addresses = indirectBlock.readIndirectBlock();
		//search each block pointed by indirect address
		for (int i = 0; i < addresses.size(); i++)
		{
			Block tempBlock = this->hd_blockList[addresses[i] - 690];
			vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
			//search whether this block contains the specific pair
			for (int j = 0; j < tempVec.size(); j++)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
					total_size += calculate_size(tempVec[j].inodeIndex);
				else
					total_size += this->hd_inodeList[tempVec[j].inodeIndex].i_size;
		}
	}
	return total_size;
}

//change load file time to old one
void HardDisk::changeTime()
{
	ifstream infile;
	string file = "C:\\Users\\USER\\Desktop\\old\\config.txt";
	ifstream test_in(file);
	if (!test_in) {
		return;
	}
	else {
		infile.open(file.data());   //���ļ����������ļ��������� 
		assert(infile.is_open());   //��ʧ��,�����������Ϣ,����ֹ�������� 
		string s;
		vector<string> info;
		vector<string> path;
		while (getline(infile, s)) {
			info = split(s, ' ');
			//�����Ŀ¼
			if (info[1] == "\\") {
				this->hd_inodeList[0].i_ctime = stol(info[3]);
				this->hd_inodeList[0].i_mtime = stol(info[4]);
			}
			else {
				path = split(info[1], '\\');
				//�����ļ���
				if (info[0] == "1") {
					int currentDirInode = 0;
					for (int i = 0; i < path.size(); i++)
					{
						int dirInode = findInode(path[i], currentDirInode, 1);
						currentDirInode = dirInode;
					}
					this->hd_inodeList[currentDirInode].i_ctime = stol(info[3]);
					this->hd_inodeList[currentDirInode].i_mtime = stol(info[4]);
				}
				else {
					int currentDirInode = 0;
					for (int i = 0; i < path.size() - 1; i++)
					{
						int dirInode = findInode(path[i], currentDirInode, 1);
						currentDirInode = dirInode;
					}
					// Ѱ����Ҫ����file��inode
					int fileInode = findInode(path[path.size() - 1], currentDirInode, 0);
					this->hd_inodeList[fileInode].i_ctime = stol(info[3]);
					this->hd_inodeList[fileInode].i_mtime = stol(info[4]);
				}
			}
		}
		infile.close();
	}
}

void HardDisk::dir()
{
	int inodeIndex = this->hd_currentDirInode;
	vector<string> filename;  vector<time_t> filectime; vector<time_t> filemtime; vector<int> filesize;
	//Search each block pointed by direct address
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
			{
				filename.push_back(tempVec[j].fileName + "/");
				filectime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_ctime));
				filemtime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_mtime));
				filesize.push_back(calculate_size(tempVec[j].inodeIndex));
			}
			else
			{
				filename.push_back(tempVec[j].fileName);
				filectime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_ctime));
				filemtime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_mtime));
				filesize.push_back(this->hd_inodeList[tempVec[j].inodeIndex].i_size);
			}
	}
	//Get the information of indirect block
	if (this->hd_inodeList[inodeIndex].i_idaddr != 0)
	{
		Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
		vector<int> addresses = indirectBlock.readIndirectBlock();
		//search each block pointed by indirect address
		for (int i = 0; i < addresses.size(); i++)
		{
			Block tempBlock = this->hd_blockList[addresses[i] - 690];
			vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
			//search whether this block contains the specific pair
			for (int j = 0; j < tempVec.size(); j++)
				if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
				{
					filename.push_back(tempVec[j].fileName + "/");
					filectime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_ctime));
					filemtime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_mtime));
					filesize.push_back(calculate_size(tempVec[j].inodeIndex));
				}
				else
				{
					filename.push_back(tempVec[j].fileName);
					filectime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_ctime));
					filemtime.push_back(static_cast<time_t>(this->hd_inodeList[tempVec[j].inodeIndex].i_mtime));
					filesize.push_back(this->hd_inodeList[tempVec[j].inodeIndex].i_size);
				}
		}
	}
	//Format output the recorded information
	const char* wday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	cout << setw(15) << left << "filename" << setw(20) << left << "Filesize/B"
		<< setw(30) << left << "CreateTime" << setw(30) << left << "ModifiedTime" << endl;
	for (int i = 0; i < filectime.size(); ++i) {
		cout << setw(15) << left << filename[i] << setw(20) << left << filesize[i];		
		struct tm p1;
		struct tm p2;
		localtime_s(&p2, &filemtime[i]);
		localtime_s(&p1, &filectime[i]);
		printf("%d/%02d/%02d ", 1900 + p1.tm_year, 1 + p1.tm_mon, p1.tm_mday);
		printf("%s %02d:%02d:%02d       ", wday[p1.tm_wday], p1.tm_hour, p1.tm_min, p1.tm_sec);
		printf("%d/%02d/%02d ", 1900 + p2.tm_year, 1 + p2.tm_mon, p2.tm_mday);
		printf("%s %02d:%02d:%02d\n", wday[p2.tm_wday], p2.tm_hour, p2.tm_min, p2.tm_sec);
	}
}

bool HardDisk::changeDir(vector<string> paths, bool type)
{
	int currentDirInode = 0;
	string tempPath;
	string path = paths[0];//Special command ~ / . ..
	if (path == "/" || path == "~")
	{
		this->hd_currentDirInode = 0;
		this->hd_currentDir = "/";
		return true;
	}
	else if (path == "..")
	{
		if (this->hd_currentDirInode == 0)//�Ѿ����˵��˸�Ŀ¼
			return true;
		else {
			vector<string> all_path = split(this->hd_currentDir, '/');
			all_path.erase(all_path.end() - 1);
			if (all_path.size() != 0)
			{
				for (int i = 0; i < all_path.size(); i++)
				{
					int dirInode = findInode(all_path[i], currentDirInode, 1);
					if (dirInode == -1) return false;
					currentDirInode = dirInode;
					tempPath += ("/" + all_path[i]);
				}
				this->hd_currentDirInode = currentDirInode;
				this->hd_currentDir = tempPath;
				return true;
			}
			else {
				this->hd_currentDirInode = 0;
				this->hd_currentDir = "/";
				return true;
				
			}
		}
	}
	else if (path == ".")
	{
		return true;
	}

	//Not special command
	if (type == true)
	{
		currentDirInode = this->hd_currentDirInode;
		if(currentDirInode != 0)
			tempPath = this->hd_currentDir;
	}	
	for (int i = 0; i < paths.size(); i++)
	{
		if (!isExist(paths[i], currentDirInode, 1))
		{
			tempPath += "/" + paths[i];
			cout << "Dir: " << tempPath << " not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) {
				cout << "Error in finding Dir: " << paths[i] << " iNode" << endl;
				return false;
			}
			currentDirInode = dirInode;
			tempPath += ("/" + paths[i]);
		}
	}
	//If find, change directory to the input one
	this->hd_currentDirInode = currentDirInode;
	this->hd_currentDir = tempPath;
	return true;
}

vector<string> HardDisk::split(const string& str, char delim)
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

string HardDisk::cat(vector<string> paths, bool type)
{
	//Make sure the exist of the file and then find the i-node index of the file
	int currentDirInode = 0;
	string tempPath;	
	if (type == true)
	{
		currentDirInode = this->hd_currentDirInode;
		if(currentDirInode != 0)
			tempPath = this->hd_currentDir;
	}
	for (int i = 0; i < paths.size() - 1; i++)//ע��
	{
		if (!isExist(paths[i], currentDirInode, 1)) {
			tempPath += "/" + paths[i];
			return "Dir: " + tempPath + "  not exist\n";
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return "Dir " + paths[i] + " iNode not find\n";
			currentDirInode = dirInode;
			tempPath += ("/" + paths[i]);
		}
	}
	//Find the inode index of the file
	int fileInode = findInode(paths[paths.size() - 1], currentDirInode, 0);
	tempPath += "/" + paths[paths.size() - 1];
	if (fileInode == -1) return "File " + tempPath + " not exist\n";
	if (this->hd_inodeList[fileInode].i_type != 0) return "File " + tempPath + "not exist, you may entered a directory name\n";
	//Begin reading the file
	string content;
	//Read each block pointed by direct address
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		string text = this->hd_blockList[this->hd_inodeList[fileInode].i_daddr[i] - 690].readFileBlock();
		content += text;
	}
	//If the file do not have indirect address, return
	if (this->hd_inodeList[fileInode].i_idaddr == 0) return content;
	//Get the information of indirect block
	Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
	vector<int> tempVec = tempBlock.readIndirectBlock();
	//Read each block pointed by indirect address
	for (int i = 0; i < tempVec.size(); i++)//��ȡ����ӵ�ַ�е����ݣ�����ȡ�������ļ���ַ������
	{
		//find available block
		string text = this->hd_blockList[tempVec[i] - 690].readFileBlock();
		content += text;
	}
	return content;
}

void HardDisk::sum()
{
	double unused_ratio = double(this->hd_superBlock.sb_freeBlockCount) / 15694.0;
	double used_ratio = 1 - unused_ratio;
	cout << "There are total 16384 blocks, we use 690 blocks to matian the system" << endl;
	cout << this->hd_superBlock.sb_freeBlockCount <<
		"/15694 blocks are unused (" << 100 * unused_ratio << "%)" << endl;
	cout << 15694 - this->hd_superBlock.sb_freeBlockCount <<
		"/15694 blocks are used (" << 100 * used_ratio << "%)" << endl;
	int used_storage = 0;
	for (int i = 0; i < 15694; ++i) {
		used_storage += this->hd_inodeList[i].i_size;
	}
	used_storage = used_storage  + 690 * 1024;
	double used_storage_ratio = double(used_storage) / 16777216.0;
	cout << "Used storage is: " << used_storage << "B/16777216B (" << used_storage_ratio * 100 << "%)" << endl;
}
