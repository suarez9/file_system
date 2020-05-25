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
}

void HardDisk::loadHardDisk()
{

}

void HardDisk::saveHardDisk()
{

}

// �жϵ�ǰdir �Ƿ���ڸ�����name 
bool HardDisk::isExist(string name, short int inodeIndex, int type)
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
short int HardDisk::findInode(string name, short int inodeIndex)
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

bool HardDisk::createDirectory(string name, short int& sonInode, short int fatherInode)
{
	if (this->hd_superBlock.sb_freeInodeCount == 0 || this->hd_superBlock.sb_freeBlockCount == 0) return false;

	int currentSize = this->hd_inodeList[fatherInode].i_size;
	//��������ռ������µ�dir
	if (currentSize + 16 > FILEMAXSIZE * 1024) return false;

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
			int availableBlock = 0;
			for (int i = 0; i < DATABLOCKCOUNT; i++)
				if (this->hd_superBlock.sb_blockBitmap[i] == 0)
					availableBlock = i;
			this->hd_inodeList[fatherInode].i_daddr[daddrIndex] = availableBlock + 690;
			//change hardDisk state
			this->hd_superBlock.sb_freeBlockCount -= 1;
			this->hd_superBlock.sb_blockBitmap[availableBlock] = 1;
		}
		int writeBlockAddr = this->hd_inodeList[fatherInode].i_daddr[daddrIndex];
		this->hd_blockList[writeBlockAddr - 690].writeDirectoryBlock(name, sonInode);
	}
	//��dirҪд��idaddrָ���block��
	else
	{
		//��inodeҪ��������idaddr��block
		if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
		if (this->hd_inodeList[fatherInode].i_idaddr == 0)
		{
			//find available block
			int availableBlock = 0;
			for (int i = 0; i < DATABLOCKCOUNT; i++)
				if (this->hd_superBlock.sb_blockBitmap[i] == 0)
					availableBlock = i;
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
			int availableBlock = 0;
			for (int i = 0; i < DATABLOCKCOUNT; i++)
				if (this->hd_superBlock.sb_blockBitmap[i] == 0)
					availableBlock = i;
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

	//find available inode
	int availableInode = 0;
	for (int i = 0; i < INODECOUNT; i++)
		if (this->hd_superBlock.sb_inodeBitmap[i] == 0)
			availableInode = i;
	sonInode = availableInode;

	//find available block
	if (this->hd_superBlock.sb_freeBlockCount == 0) return false;
	int availableBlock = 0;
	for (int i = 0; i < DATABLOCKCOUNT; i++)
		if (this->hd_superBlock.sb_blockBitmap[i] == 0)
			availableBlock = i;

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
	int currentDirInode = 0;
	// for all the dirs in paths
	for (int i = 0; i < paths.size(); i++)
	{
		// if dir is not exist, create, else�����dir 
		if (!isExist(paths[i], currentDirInode, 1))
		{
			short int dirInode = 0;
			//���ɹ����� �����dir
			if (createDirectory(paths[i], dirInode, currentDirInode))
				currentDirInode = dirInode;
			else
				return false;
		}
		else
		{
			short int dirInode = findInode(paths[i], currentDirInode);
			if (dirInode == -1) return false;
			currentDirInode = dirInode;
		}
	}
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
			short int dirInode = findInode(paths[i], currentDirInode);
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

void HardDisk::createFile()
{

}

