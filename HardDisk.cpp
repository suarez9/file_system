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
	//为root占用第1个inode 指向第1个block
	this->hd_superBlock.sb_freeBlockCount -= 1;
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[0] = 1;
	this->hd_superBlock.sb_blockBitmap[0] = 1;
	//初始化第1个inode
	this->hd_inodeList[0].i_type = 1;
	this->hd_inodeList[0].i_size = 0;

	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[0].i_ctime = now;
	this->hd_inodeList[0].i_mtime = now;
	this->hd_inodeList[0].i_daddr[0] = 690;  //data block从690开始

	this->hd_currentDirInode = 0;
	this->hd_currentDir = '/';
	this->Hash.insert({0, "\\"});
}

void HardDisk::loadHardDisk()
{
	initiate();
	ifstream infile;
	string file = "C:\\Users\\USER\\Desktop\\new\\config.txt";
	string root_path = "C:\\Users\\USER\\Desktop\\new";

	ifstream test_in(file);
	if (!test_in) {
		saveSystemConfig();
	}

	infile.open(file.data());   //将文件流对象与文件连接起来 
	assert(infile.is_open());   //若失败,则输出错误消息,并终止程序运行 

	string s;
	vector<string> info;
	vector<string> path;
	while (getline(infile, s))
	{
		info = split(s, ' ');
		//处理根目录
		if (info[1] == "/") {
			this->hd_inodeList[0].i_ctime = stol(info[3]);
			this->hd_inodeList[0].i_mtime = stol(info[4]);
		}
		else {
			//处理文件夹
			if (info[0] =="1") {
				path = split(info[1], '\\');
				createDir(path);
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
			//处理文件
			else {
				path = split(info[1], '\\');
				createFile(path, stoi(info[2]) / 1024);
				//set time 
				int currentDirInode = 0;
				for (int i = 0; i < path.size() - 1; i++)
				{
					int dirInode = findInode(path[i], currentDirInode, 0);
					currentDirInode = dirInode;
				}
				// 寻找需要复制file的inode
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
	
	
	infile.close();             //关闭文件输入流 
	
}

void HardDisk::saveHardDisk(int inodeIndex, string absolute_path, string local_path)
{
	string name;
	string command;

	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		for (int j = 0; j < tempVec.size(); j++)
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
			{
				name = local_path + tempVec[j].fileName;
				this->Hash.insert({ tempVec[j].inodeIndex , name });

				name = absolute_path + tempVec[j].fileName;
				command = "mkdir " + name;
				system(command.c_str());
				string new_absolute_path = name + "\\";
				string new_local_path = local_path + "\\" + tempVec[j].fileName + "\\";
				saveHardDisk(tempVec[j].inodeIndex, new_absolute_path, new_local_path);
			}
			else
			{
				name = local_path + tempVec[j].fileName;
				this->Hash.insert({ tempVec[j].inodeIndex , name });
				string text = cat(split(name, '\\'));

				name = absolute_path + tempVec[j].fileName;
				ofstream out(name);
				if (!out)
					cerr << "Error in storage to the computer." << endl;
				out << text << endl;
			}
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
				{
					name = local_path + tempVec[j].fileName;
					this->Hash.insert({ tempVec[j].inodeIndex , name });

					name = absolute_path + tempVec[j].fileName;
					command = "mkdir " + name;
					system(command.c_str());
					string new_absolute_path = name + "\\";
					string new_local_path = local_path + "\\" + tempVec[j].fileName + "\\";
					saveHardDisk(tempVec[j].inodeIndex, new_absolute_path, new_local_path);
				}
				else
				{
					name = local_path + tempVec[j].fileName;
					this->Hash.insert({ tempVec[j].inodeIndex , name });
					string text = cat(split(name, '\\'));

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

	map<int, string>::iterator it;
	for (it = Hash.begin(); it != Hash.end(); it++) {
		name = it->second;
		inodeIndex = it->first;
		out << setw(4) << left << this->hd_inodeList[inodeIndex].i_type;
		out << setw(20) << left << name;
		out << setw(10) << left << this->hd_inodeList[inodeIndex].i_size;
		out << setw(15) << left << this->hd_inodeList[inodeIndex].i_ctime;
		out << setw(15) << left << this->hd_inodeList[inodeIndex].i_mtime;
		if (inodeIndex == 0)
			name = "";
		out << endl;
	}
}

int HardDisk::findAvailableBlock()
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

int HardDisk::findAvailableInode()
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
	//改变fatherInode的block 增加新pair
	//新dir可以写在10个直接地址的block里
	if (currentSize + 16 <= 10 * 1024)
	{
		//新的dir要添加在第几个daddr指向的block
		int daddrIndex = (currentSize + 16 - 1) / 1024;
		//此inode要占用新的block
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
	//新dir要写在idaddr指向的block里
	else
	{
		//此inode要创建储存idaddr的block
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

		//此inode要占用新的idaddr指向的block
		if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
		if (currentSize % 1024 == 0)
		{
			//find available block
			int availableBlock = findAvailableBlock();
			if (availableBlock == -1) return false;

			int idaddr = this->hd_inodeList[fatherInode].i_idaddr;
			//将新占用的block的地址写入idaddr block
			this->hd_blockList[idaddr - 690].writeIndirectBlock(availableBlock + 690);
			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		}
		//新的dir要添加在idaddr指向的第几个block
		int idaddrIndex = (currentSize + 16 - 1) / 1024 - 10;
		Block tempBlock = this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690];
		vector<int> tempVec = tempBlock.readIndirectBlock();
		this->hd_blockList[tempVec[idaddrIndex] - 690].writeDirectoryBlock(name, sonInode);
	}
}

bool HardDisk::deleteFromDirectoryBlock(int currentSize, int fatherInode, string name, int type)
{
	//改变fatherInode的block 删除pair
	//特定pair在10个直接地址的block里
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
		// 删除特定的pair 并从lastBlock移动最后1个元素到locateBlock填补空缺
		locateBlockVec.push_back(lastBlockVec.back());
		// 重写locateBlock
		this->hd_blockList[locateBlockAddr - 690].clearBlock();
		for (int i = 0; i < locateBlockVec.size(); i++)
			//  只添加除特定pair以外的pair
			if (locateBlockVec[i].fileName != name || this->hd_inodeList[locateBlockVec[i].inodeIndex].i_type != type)
				this->hd_blockList[locateBlockAddr - 690].writeDirectoryBlock(locateBlockVec[i].fileName, locateBlockVec[i].inodeIndex);

		// 重写lastBlock 如果lastBlock还有pair 如果lastBlock不同于locateBlock
		if (lastBlockAddr != locateBlockAddr)
		{
			if ((currentSize - 16) % 1024 != 0)
			{
				this->hd_blockList[lastBlockAddr - 690].clearBlock();
				// 只添加除最后1个pair之外的pair
				for (int i = 0; i < lastBlockVec.size() - 1; i++)
					//  只添加除特定pair以外的pair
					if (lastBlockVec[i].fileName != name || this->hd_inodeList[lastBlockVec[i].inodeIndex].i_type != type)
						this->hd_blockList[lastBlockAddr - 690].writeDirectoryBlock(lastBlockVec[i].fileName, lastBlockVec[i].inodeIndex);
			}
			else if (currentSize - 16 != 0)
			{
				releaseBlock(lastBlockAddr - 690);
				this->hd_inodeList[fatherInode].i_daddr[(currentSize - 1) / 1024] = 0;
			}
		}
	}
	//pair在idaddr指向的block里
	else
	{
		vector<DirectoryBlockElement> locateBlockVec;
		vector<int> idaddrVec = this->hd_blockList[this->hd_inodeList[fatherInode].i_idaddr - 690].readIndirectBlock();
		int locateBlockAddr = 0;
		int lastBlockAddr = idaddrVec.back();
		// 此pair在10个直接地址指向的block里
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
		// 此pair在idaddr间接指向的block里
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
		// 删除特定的pair 并从lastBlock移动最后1个元素到locateBlock填补空缺
		locateBlockVec.push_back(lastBlockVec.back());
		// 重写locateBlock
		this->hd_blockList[locateBlockAddr - 690].clearBlock();
		for (int i = 0; i < locateBlockVec.size(); i++)
			//  只添加除特定pair以外的pair
			if (locateBlockVec[i].fileName != name || this->hd_inodeList[locateBlockVec[i].inodeIndex].i_type != type)
				this->hd_blockList[locateBlockAddr - 690].writeDirectoryBlock(locateBlockVec[i].fileName, locateBlockVec[i].inodeIndex);

		// 重写lastBlock 如果lastBlock还有pair 如果lastBlock不同于locateBlock
		if (lastBlockAddr != locateBlockAddr)
		{
			if ((currentSize - 16) % 1024 != 0)
			{
				this->hd_blockList[lastBlockAddr - 690].clearBlock();
				// 只添加除最后1个pair之外的pair
				for (int i = 0; i < lastBlockVec.size() - 1; i++)
					//  只添加除特定pair以外的pair
					if (lastBlockVec[i].fileName != name || this->hd_inodeList[lastBlockVec[i].inodeIndex].i_type != type)
						this->hd_blockList[lastBlockAddr - 690].writeDirectoryBlock(lastBlockVec[i].fileName, lastBlockVec[i].inodeIndex);
			}
			else
			{
				releaseBlock(lastBlockAddr - 690);
				// 如果删除完 idaddr间接指向的block还有pair
				if ((currentSize - 16) > 10 * 1024)
				{
					// 更改indirectBlock
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

// 判断当前dir 是否存在给定的name 
bool HardDisk::isExist(string name, int inodeIndex, int type)
{
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

	if (this->hd_inodeList[inodeIndex].i_idaddr == 0) return false;
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
	vector<int> addresses = indirectBlock.readIndirectBlock();
	//search each block pointed by indirect address
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
	return false;
}

// 寻找给定name对应的inode index
short int HardDisk::findInode(string name, int inodeIndex, int type)
{
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

	if (this->hd_inodeList[inodeIndex].i_idaddr == 0) return -1;
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_idaddr - 690];
	vector<int> addresses = indirectBlock.readIndirectBlock();
	//search each block pointed by indirect address
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
	return -1;
}

bool HardDisk::createDirectory(string name, int& sonInode, int fatherInode)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount == 0) {
		cout << "No free iNode or no free block in creating directory" << endl;
		return false;
	}

	int currentSize = this->hd_inodeList[fatherInode].i_size;
	//如果不够空间增加新的dir
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

	//初始化inode
	this->hd_inodeList[sonInode].i_type = 1;
	this->hd_inodeList[sonInode].i_size = 0;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[sonInode].i_ctime = now;
	this->hd_inodeList[sonInode].i_mtime = now;
	this->hd_inodeList[sonInode].i_daddr[0] = availableBlock + 690;

	//改变父inode
	this->hd_inodeList[fatherInode].i_size += 16;
	this->hd_inodeList[fatherInode].i_mtime = now;

	return true;
}

// 如果输入的path中有多个未创建的dir 不断调用createDirectory
bool HardDisk::createDir(vector<string> paths)
{
	int currentDirInode = 0;
	// for all the dirs in paths
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, create, else进入此dir 
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
			currentDirInode = dirInode;
		}
	}
	//这里可能需要一个current dir的变换
	return true;
}

bool HardDisk::deleteDirectory(int currentDirInode)
{
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[currentDirInode].i_daddr[i] == 0) break;
		Block tempBlock = this->hd_blockList[this->hd_inodeList[currentDirInode].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//
		if (this->hd_inodeList[currentDirInode].i_type == 1)
		{

		}
		else
		{

		}
	}

	if (this->hd_inodeList[currentDirInode].i_idaddr == 0) return true;

	Block tempBlock = this->hd_blockList[this->hd_inodeList[currentDirInode].i_idaddr - 690];

	return true;
}

bool HardDisk::deleteDir(vector<string> paths)
{
	// 寻找需要删除dir的inode
	int currentDirInode = 0;
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths[i], currentDirInode, 1)) return false;
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	//如果要删除的dir是current working directory, return false
	if (currentDirInode == this->hd_currentDirInode) return false;
	else
	{

	}
	return true;
}

bool HardDisk::createFile(vector<string> paths, float size)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount < size) {
		cout << "No free iNode or block in creating file" << endl;
		return false;
	}

	// 进入file所在的dir
	int currentDirInode = 0;
	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths[i], currentDirInode, 1)) {
			cout << "Dir: " << paths[i] << " does not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	int currentSize = this->hd_inodeList[currentDirInode].i_size;
	//如果不够空间增加新的dir
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

	//初始化inode
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
	//改变父inode
	this->hd_inodeList[currentDirInode].i_size += 16;
	this->hd_inodeList[currentDirInode].i_mtime = now;
	return true;
}

bool HardDisk::deleteFile(vector<string> paths)
{
	// 进入file所在的dir
	int currentDirInode = 0;
	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths[i], currentDirInode, 1)) return false;
		else
		{
			int dirInode = findInode(paths[i], currentDirInode, 1);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	// 寻找需要删除file的inode
	int fileInode = findInode(paths[paths.size() - 1], currentDirInode, 0);
	// 找不到file
	if (fileInode == -1) return false;

	// 释放daddr指向的block
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
		releaseBlock(this->hd_inodeList[fileInode].i_daddr[i] - 690);
	}
	// 释放idaddr间接指向的block
	if (this->hd_inodeList[fileInode].i_idaddr != 0)
	{
		Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
		vector<int> tempVec = tempBlock.readIndirectBlock();
		for (int j = 0; j < tempVec.size(); j++)
			releaseBlock(tempVec[j] - 690);
		// 释放indirectblock
		releaseBlock(this->hd_inodeList[fileInode].i_idaddr - 690);
	}
	// 释放file的inode
	releaseInode(fileInode);

	// 修改dir的directoryBlock
	int currentSize = this->hd_inodeList[currentDirInode].i_size;
	deleteFromDirectoryBlock(currentSize, currentDirInode, paths[paths.size() - 1], 0);

	//改变父inode
	this->hd_inodeList[currentDirInode].i_size -= 16;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[currentDirInode].i_mtime = now;

	return true;
}

bool HardDisk::copyFile(vector<string> paths1, vector<string> paths2)
{
	// 进入file1所在的dir
	int currentDirInode1 = 0;
	for (int i = 0; i < paths1.size() - 1; i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths1[i], currentDirInode1, 1)) return false;
		else
		{
			int dirInode = findInode(paths1[i], currentDirInode1, 1);
			if (dirInode == -1) return false;
			currentDirInode1 = dirInode;
		}
	}
	// 寻找需要复制file的inode
	int fileInode = findInode(paths1[paths1.size() - 1], currentDirInode1, 0);
	if (fileInode == -1) return false;
	int size = this->hd_inodeList[fileInode].i_size;
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount < size / 1024) return false;

	// 进入file2所在的dir
	int currentDirInode2 = 0;
	for (int i = 0; i < paths2.size() - 1; i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths2[i], currentDirInode2, 1)) return false;
		else
		{
			int dirInode = findInode(paths2[i], currentDirInode2, 1);
			if (dirInode == -1) return false;
			currentDirInode2 = dirInode;
		}
	}
	if (isExist(paths2[paths2.size() - 1], currentDirInode2, 0))
	{
		cout << "File already exists!" << endl;
		return false;
	}

	int currentSize = this->hd_inodeList[currentDirInode2].i_size;
	//如果不够空间增加新的dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) return false;

	//find available inode
	int availableInode = findAvailableInode();
	if (availableInode == -1) return false;

	addToDirectoryBlock(currentSize, currentDirInode2, paths2[paths2.size() - 1], availableInode);

	//change hardDisk state
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[availableInode] = 1;

	//初始化inode
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
		if (availableBlock == -1) return false;
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
		if (availableBlock == -1) return false;
		string text = this->hd_blockList[this->hd_inodeList[fileInode].i_daddr[i] - 690].readFileBlock();
		this->hd_blockList[availableBlock].writeFileBlock(text);
		//change hardDisk state
		this->hd_superBlock.sb_freeBlockCount -= 1;
		this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		//change inode state
		this->hd_inodeList[availableInode].i_daddr[i] = availableBlock + 690;
	}
	//idaddr
	if (this->hd_inodeList[fileInode].i_idaddr == 0) return true;
	Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
	vector<int> tempVec = tempBlock.readIndirectBlock();
	for (int i = 0; i < tempVec.size(); i++)
	{
		//find available block
		int availableBlock = findAvailableBlock();
		if (availableBlock == -1) return false;
		string text = this->hd_blockList[tempVec[i] - 690].readFileBlock();
		this->hd_blockList[availableBlock].writeFileBlock(text);
		//change hardDisk state
		this->hd_superBlock.sb_freeBlockCount -= 1;
		this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		//change inode state
		this->hd_blockList[this->hd_inodeList[availableInode].i_idaddr - 690].writeIndirectBlock(availableBlock + 690);
	}

	//改变父inode
	this->hd_inodeList[currentDirInode2].i_size += 16;
	this->hd_inodeList[currentDirInode2].i_mtime = now;

	return true;
}

void HardDisk::releaseBlock(int blockIndex)
{
	this->hd_blockList[blockIndex].clearBlock();
	//修改superBlock
	this->hd_superBlock.sb_blockBitmap[blockIndex] = 0;
	this->hd_superBlock.sb_freeBlockCount += 1;
}

void HardDisk::releaseInode(int inodeIndex)
{
	this->hd_inodeList[inodeIndex].clearInode();
	//修改superBlock
	this->hd_superBlock.sb_inodeBitmap[inodeIndex] = 0;
	this->hd_superBlock.sb_freeInodeCount += 1;
}

string HardDisk::readTxt_all(string file, int size)//用于读取已经保存在电脑硬盘中的文件，用来加载系统
{
	ifstream infile;
	infile.open(file.data());   //将文件流对象与文件连接起来 
	assert(infile.is_open());   //若失败,则输出错误消息,并终止程序运行 
	int count = 0;
	char c;
	string content = "";
	while (count < size)
	{
		infile >> c;
		content += c;
		count += 1;
	}
	infile.close();             //关闭文件输入流 
	return content;
}

int HardDisk::calculate_size(int inodeIndex)//递归函数，用于计算dir里文件的大小
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

void HardDisk::dir()
{
	int inodeIndex = this->hd_currentDirInode;
	vector<string> filename;  vector<time_t> filectime; vector<time_t> filemtime; vector<int> filesize;
	for (int i = 0; i < 10; i++)//对直接地址进行遍历
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

	if (this->hd_inodeList[inodeIndex].i_idaddr != 0)//对间接地址进行遍历
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
	//格式化输出
	const char* wday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	cout << setw(15) << left << "filename" << setw(20) << left << "Filesize/B"
		<< setw(30) << left << "CreateTime" << setw(30) << left << "ModifiedTime" << endl;
	for (int i = 0; i < filectime.size(); ++i) {
		cout << setw(15) << left << filename[i] << setw(20) << left << filesize[i];
		struct tm* p1;
		struct tm* p2;
		p1 = localtime(&filemtime[i]);
		p2 = localtime(&filectime[i]);
		printf("%d/%02d/%02d ", 1900 + p1->tm_year, 1 + p1->tm_mon, p1->tm_mday);
		printf("%s %02d:%02d:%02d       ", wday[p1->tm_wday], p1->tm_hour, p1->tm_min, p1->tm_sec);
		printf("%d/%02d/%02d ", 1900 + p2->tm_year, 1 + p2->tm_mon, p2->tm_mday);
		printf("%s %02d:%02d:%02d\n", wday[p2->tm_wday], p2->tm_hour, p2->tm_min, p2->tm_sec);
	}
}

bool HardDisk::changeDir(vector<string> paths)
{
	int currentDirInode = 0;
	string tempPath;

	string path = paths[0];//特殊指令 ~ / . ..
	if (path == "/" || path == "~")
	{
		this->hd_currentDirInode = 0;
		this->hd_currentDir = "/";
		return true;
	}
	else if (path == "..")
	{
		if (this->hd_currentDirInode == 0)//已经回退到了根目录
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
		return true;

	//非特殊指令
	for (int i = 0; i < paths.size(); i++)
	{
		if (!isExist(paths[i], currentDirInode, 1))
		{
			cout << "Dir: " << paths[i] << " does not exist" << endl;
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

string HardDisk::cat(vector<string> paths1)
{
	//确定文件是否存在，并找到文件的inode所在
	int currentDirInode1 = 0;
	string content;
	for (int i = 0; i < paths1.size() - 1; i++)//注意
	{
		if (!isExist(paths1[i], currentDirInode1, 1)) return "Dir " + paths1[i] + " does not exist";
		else
		{
			int dirInode = findInode(paths1[i], currentDirInode1, 1);
			if (dirInode == -1) return "Dir " + paths1[i] + " iNode can not find";
			currentDirInode1 = dirInode;
		}
	}
	// 寻找需要复制file的inode
	int fileInode = findInode(paths1[paths1.size() - 1], currentDirInode1, 0);
	if (fileInode == -1) return "File not exist";
	if (this->hd_inodeList[fileInode].i_type != 0)return "File not exist, you may entered a directory name";
	//开始读取文件内容
	for (int i = 0; i < 10; i++)//读取10个直接地址中的内容
	{
		if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		string text = this->hd_blockList[this->hd_inodeList[fileInode].i_daddr[i] - 690].readFileBlock();
		content += text;
	}
	if (this->hd_inodeList[fileInode].i_idaddr == 0) return content;
	Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
	vector<int> tempVec = tempBlock.readIndirectBlock();
	for (int i = 0; i < tempVec.size(); i++)//读取到间接地址中的内容，并读取真正的文件地址的内容
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
