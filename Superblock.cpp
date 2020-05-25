#include"Superblock.h"
#include<iostream>

SuperBlock::SuperBlock()
{
	this->sb_blockCount = 16384;
	this->sb_superBlockCount = 62;
	this->sb_inodeBlockCount = 628;
	this->sb_freeBlockCount = 15694;
	this->sb_freeInodeCount = 15694;
	for (int i = 0; i < this->sb_freeBlockCount; i++)
	{
		this->sb_blockBitmap[i] = 0;
		this->sb_inodeBitmap[i] = 0;
	}
}