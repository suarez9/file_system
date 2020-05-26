#include "HardDisk.h"
#include<iomanip>
#include<ctime>
#include<iostream>
#include<fstream>


HardDisk::HardDisk()
{
	/*
	cout << this->hd_superBlock.sb_freeBlockCount << endl;
	cout << this->hd_superBlock.sb_freeInodeCount << endl;
	for (int i = 0; i < 15694; i++)
		cout << this->hd_superBlock.sb_blockBitmap[i];
	cout << endl;
	*/
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
}

void HardDisk::loadHardDisk()
{

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
		cout << "!=0\n";
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

void HardDisk::saveHardDisk2()
{



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
	//改变fatherInode的block 增加新dir
	//新dir可以写在10个直接地址的block里
	int writeBlockAddr = 0;
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
short int HardDisk::findInode(string name, int inodeIndex)
{
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName) return tempVec[j].inodeIndex;
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
			if (name == tempVec[j].fileName) return tempVec[j].inodeIndex;
	}
	return -1;
}

bool HardDisk::createDirectory(string name, int& sonInode, int fatherInode)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount == 0) return false;

	int currentSize = this->hd_inodeList[fatherInode].i_size;
	//如果不够空间增加新的dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) return false;

	//find available inode
	int availableInode = findAvailableInode();
	if (availableInode == -1) return false;
	sonInode = availableInode;

	addToDirectoryBlock(currentSize, fatherInode, name, sonInode);

	//find available block
	int availableBlock = findAvailableBlock();
	if (availableBlock == -1) return false;

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
			//若成功创建 进入此dir
			if (createDirectory(paths[i], dirInode, currentDirInode))
				currentDirInode = dirInode;
			else
				return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
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
			int dirInode = findInode(paths[i], currentDirInode);
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
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount < size) return false;

	// 进入file所在的dir
	int currentDirInode = 0;
	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths[i], currentDirInode, 1)) return false;
		else
		{
			int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	int currentSize = this->hd_inodeList[currentDirInode].i_size;
	//如果不够空间增加新的dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) return false;

	//find available inode
	int availableInode = findAvailableInode();
	if (availableInode == -1) return false;

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
		if (availableBlock == -1) return false;
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
			if (availableBlock == -1) return false;
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
			if (availableBlock == -1) return false;
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
			int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	// 寻找需要删除file的inode
	int fileInode = findInode(paths[paths.size() - 1], currentDirInode);
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
	}
	// 释放idaddr指向的block
	releaseBlock(this->hd_inodeList[fileInode].i_idaddr - 690);
	// 释放file的inode
	releaseInode(fileInode);

	// 修改dir的directoryBlock

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
			int dirInode = findInode(paths1[i], currentDirInode1);
			if (dirInode == -1) return false;
			currentDirInode1 = dirInode;
		}
	}
	// 寻找需要复制file的inode
	int fileInode = findInode(paths1[paths1.size() - 1], currentDirInode1);
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
			int dirInode = findInode(paths2[i], currentDirInode2);
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


int HardDisk::calculate_size(int inodeIndex) {
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
	vector<string> filename;
	vector<time_t> filectime;
	vector<time_t> filemtime;
	vector<int> filesize;
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

	string path = paths[0];
	if (path == "/" || path == "~")
	{
		this->hd_currentDirInode = 0;
		this->hd_currentDir = "/";
		return true;
	}
	else if (path == "..")
	{
		if (this->hd_currentDirInode == 0)
			return true;
		else {
			vector<string> all_path = split(this->hd_currentDir, '/');
			all_path.erase(all_path.end() - 1);
			if (all_path.size() != 0)
			{
				for (int i = 0; i < all_path.size(); i++)
				{
					int dirInode = findInode(all_path[i], currentDirInode);
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

	
	for (int i = 0; i < paths.size(); i++)
	{
		if (!isExist(paths[i], currentDirInode, 1))
		{
			cout << "Dir: " << paths[i] << " does not exist" << endl;
			return false;
		}
		else
		{
			int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
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
	//确定这文件是否存在，并找到文件的inode所在
	int currentDirInode1 = 0;
	string content;
	for (int i = 0; i < paths1.size() - 1; i++)
	{
		// if dir is not exist, return false, else进入此dir 
		if (!isExist(paths1[i], currentDirInode1, 1)) return "File not exist";
		else
		{
			int dirInode = findInode(paths1[i], currentDirInode1);
			if (dirInode == -1) return "File not exist";
			currentDirInode1 = dirInode;
		}
	}
	// 寻找需要复制file的inode
	int fileInode = findInode(paths1[paths1.size() - 1], currentDirInode1);
	if (fileInode == -1) return "File not exist";
	//开始读取文件内容
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[fileInode].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		string text = this->hd_blockList[this->hd_inodeList[fileInode].i_daddr[i] - 690].readFileBlock();
		//cout << text << endl;
		content += text;
	}


	if (this->hd_inodeList[fileInode].i_idaddr == 0) return content;
	Block tempBlock = this->hd_blockList[this->hd_inodeList[fileInode].i_idaddr - 690];
	vector<int> tempVec = tempBlock.readIndirectBlock();
	for (int i = 0; i < tempVec.size(); i++)
	{
		//find available block
		
		string text = this->hd_blockList[tempVec[i] - 690].readFileBlock();
		//cout << text << endl;
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
