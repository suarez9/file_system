#include "HardDisk.h"
#include<ctime>
#include<iostream>

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
}

void HardDisk::loadHardDisk()
{

}

void HardDisk::saveHardDisk()
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


// �жϵ�ǰdir �Ƿ���ڸ�����name 
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

// Ѱ�Ҹ���name��Ӧ��inode index
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

bool HardDisk::modifyBlock()
{
	return false;
}



bool HardDisk::createDirectory(string name, int& sonInode, int fatherInode)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount == 0) return false;

	int currentSize = this->hd_inodeList[fatherInode].i_size;
	//��������ռ������µ�dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) return false;

	//find available inode
	int availableInode = findAvailableBlock();
	if (availableInode == -1) return false;
	sonInode = availableInode;

	//�ı䵱ǰinode��block ������dir
	//��dir����д��10��ֱ�ӵ�ַ��block��
	int writeBlockAddr = 0;
	if (currentSize + 16 <= 10 * 1024)
	{
		//�µ�dirҪ����ڵڼ���daddrָ���block
		int daddrIndex = (currentSize + 16 - 1) / 1024;
		//��inodeҪռ���µ�block
		if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
		if (currentSize % 1024 == 0)
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

	//find available block
	int availableBlock = findAvailableBlock();
	if (availableBlock == -1) return false;

	//change hardDisk state
	this->hd_superBlock.sb_freeBlockCount -= 1;
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[sonInode] = 1;
	this->hd_superBlock.sb_blockBitmap[sonInode] = 1;

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
bool HardDisk::createDir(vector<string> paths)
{
	int currentDirInode = this->hd_currentDirInode;
	// for all the dirs in paths
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, create, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1))
		{
			int dirInode = 0;
			//���ɹ����� �����dir
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
	// Ѱ����Ҫɾ��dir��inode
	int currentDirInode = 0;
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1)) return false;
		else
		{
			int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	//���Ҫɾ����dir��current working directory, return false
	if (currentDirInode == this->hd_currentDirInode) return false;
	else
	{

	}
	return true;
}

vector<string> HardDisk::dir()
{
	int inodeIndex = this->hd_currentDirInode;
	vector<string> filename;
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[inodeIndex].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[inodeIndex].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific name inode index pair
		for (int j = 0; j < tempVec.size(); j++)
			if (this->hd_inodeList[tempVec[j].inodeIndex].i_type == 1)
				filename.push_back(tempVec[j].fileName + "/");
			else
				filename.push_back(tempVec[j].fileName);
	}

	if (this->hd_inodeList[inodeIndex].i_idaddr == 0) 
		return filename;
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
				filename.push_back(tempVec[j].fileName + "/");
			else
				filename.push_back(tempVec[j].fileName);
	}
	return filename;
}

void HardDisk::createFile()
{

}


bool HardDisk::deleteFile(vector<string> paths)
{
	// ����file���ڵ�dir
	int currentDirInode = 0;
	for (int i = 0; i < paths.size() - 1; i++)
	{
		// if dir is not exist, return false, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1)) return false;
		else
		{
			int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
	// Ѱ����Ҫɾ��file��inode
	int fileInode = findInode(paths[paths.size() - 1], currentDirInode);
	// �Ҳ���file
	if (fileInode == -1) return false;
	// �ͷ�daddrռ�õ�block
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
	}
	// �ͷ�idaddrռ�õ�block
	releaseBlock(this->hd_inodeList[fileInode].i_idaddr - 690);
	// �ͷ�file��inode
	releaseInode(fileInode);

	// �޸�dir��directoryBlock

	return true;
}

void HardDisk::releaseBlock(int blockIndex)
{
	this->hd_blockList[blockIndex].clearBlock();
	//�޸�superBlock
	this->hd_superBlock.sb_blockBitmap[blockIndex] = 0;
	this->hd_superBlock.sb_freeBlockCount += 1;
}

void HardDisk::releaseInode(int inodeIndex)
{
	this->hd_inodeList[inodeIndex].clearInode();
	//�޸�superBlock
	this->hd_superBlock.sb_inodeBitmap[inodeIndex] = 0;
	this->hd_superBlock.sb_freeInodeCount += 1;
}

bool HardDisk::changeDir(vector<string> paths)
{
	int currentDirInode = 0;
	string tempPath;

	if (paths.size() == 1) {
		string path = paths[0];
		if (path == "/" || path == "~")
		{
			currentDirInode = 0;
			tempPath = "/";
		}
		else if(path == "..")
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
				}
				else {
					currentDirInode = 0;
					tempPath = "/";
				}
			}
		}
		else if (path == ".")
		{
			return true;
		}
	}
	
	else {
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
