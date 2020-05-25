#include "HardDisk.h"
#include<ctime>

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
}

void HardDisk::loadHardDisk()
{

}

void HardDisk::saveHardDisk()
{

}

bool HardDisk::isExist(string name)
{
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[hd_currentDirInode].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[hd_currentDirInode].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName) return true;
	}

	if (this->hd_inodeList[hd_currentDirInode].i_idaddr == 0) return false;
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[hd_currentDirInode].i_idaddr - 690];
	vector<int> addresses = indirectBlock.readIndirectBlock();
	//search each block pointed by indirect address
	for (int i = 0; i < addresses.size(); i++) 
	{
		Block tempBlock = this->hd_blockList[addresses[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName) return true;
	}
	return false;
}

short int HardDisk::findInode(string name)
{
	for (int i = 0; i < 10; i++)
	{
		if (this->hd_inodeList[hd_currentDirInode].i_daddr[i] == 0) break;
		// get the current dir's inode's block
		Block tempBlock = this->hd_blockList[this->hd_inodeList[hd_currentDirInode].i_daddr[i] - 690];
		vector<DirectoryBlockElement> tempVec = tempBlock.readDirectoryBlock();
		//search whether this block contains the specific pair
		for (int j = 0; j < tempVec.size(); j++)
			if (name == tempVec[j].fileName) return tempVec[j].inodeIndex;
	}

	if (this->hd_inodeList[hd_currentDirInode].i_idaddr == 0) return -1;
	Block indirectBlock = this->hd_blockList[this->hd_inodeList[hd_currentDirInode].i_idaddr - 690];
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

bool HardDisk::createDirectory(string name, short int& inode)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount==0) return false;

	int currentSize = this->hd_inodeList[this->hd_currentDirInode].i_size;
	//��������ռ������µ�dir
	if (currentSize + 16 > FILEMAXSIZE * 1024)
		return false;


	//�ı䵱ǰinode��block ������dir


	

	//find available inode
	int availableInode = 0;
	for (int i = 0; i < INODECOUNT; i++)
		if (this->hd_superBlock.sb_inodeBitmap[i] == 0)
			availableInode = i;
	//find available block
	int availableBlock = 690;
	for (int i = 0; i < DATABLOCKCOUNT; i++)
		if (this->hd_superBlock.sb_blockBitmap[i] == 0)
			availableBlock = i;

	//change hardDisk state
	this->hd_superBlock.sb_freeBlockCount -= 1;
	this->hd_superBlock.sb_freeInodeCount -= 1;
	this->hd_superBlock.sb_inodeBitmap[availableInode] = 1;
	this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
	//��ʼ��inode
	this->hd_inodeList[availableInode].i_type = 1;
	this->hd_inodeList[availableInode].i_size = 0;
	long int now = static_cast<long int>(time(0));
	this->hd_inodeList[availableInode].i_ctime = now;
	this->hd_inodeList[availableInode].i_mtime = now;
	this->hd_inodeList[availableInode].i_daddr[0] = availableBlock + 690;

	//�ı丸inode
	this->hd_inodeList[this->hd_currentDirInode].i_size += 16;
	this->hd_inodeList[this->hd_currentDirInode].i_mtime = now;

	inode = availableInode;
	return true;
}

bool HardDisk::createDir(vector<string> paths)
{
	// for all the dirs in paths
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, create, else�����dir 
		if (!isExist(paths[i]))
		{
			short int dirInode = 0;
			//���ɹ����� �����dir
			if (createDirectory(paths[i], dirInode))
				this->hd_currentDirInode = dirInode;
			else
				return false;
		}
		else
		{
			short int dirInode = findInode(paths[i]);
			if (dirInode == -1) return false;
			this->hd_currentDirInode = dirInode;
		}
	}
	return true;
}

void HardDisk::createFile()
{

}

bool HardDisk::changeDir(string)
{
	return false;
}
