#include"Superblock.h"
#include<iostream>

SuperBlock::SuperBlock()
{
	this->sb_blockCount = 16384;						
	this->sb_superBlockCount = 62;			
	this->sb_inodeBlockCount = 328 ;		
	this->sb_freeBlockCount = this->sb_blockCount - this->sb_superBlockCount - this->sb_inodeBlockCount;				
	this->sb_freeInodeCount = this->sb_freeBlockCount;
	for (int i = 0; i < this->sb_freeBlockCount; i++)
	{
		this->sb_blockBitmap[i] = 0;
		this->sb_inodeBitmap[i] = 0;
	}
}