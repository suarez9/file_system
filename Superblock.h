#pragma once

/*Super Block 从第0个Block开始 占据1个Block*/

class SuperBlock
{
public:
	SuperBlock();
	~SuperBlock() {};

public:
	short int sb_blockCount;						// Block总数
	short int sb_superBlockCount;			// SuperBlock区占用的Block数 
	short int sb_inodeBlockCount;			// Inode区占用的Block数 
	short int sb_freeBlockCount;				// 空闲Data Block数量 
	short int sb_blockBitmap[15694];		// Data Block索引表 
	short int sb_freeInodeCount;				// 空闲Inode数量 
	short int sb_inodeBitmap[15694];		// Inode索引表 
};

