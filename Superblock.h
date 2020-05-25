#pragma once

/*Super Block �ӵ�0��Block��ʼ ռ��1��Block*/

class SuperBlock
{
public:
	SuperBlock();
	~SuperBlock() {};

public:
	short int sb_blockCount;						// Block����
	short int sb_superBlockCount;			// SuperBlock��ռ�õ�Block�� 
	short int sb_inodeBlockCount;			// Inode��ռ�õ�Block�� 
	short int sb_freeBlockCount;				// ����Data Block���� 
	short int sb_blockBitmap[15694];		// Data Block������ 
	short int sb_freeInodeCount;				// ����Inode���� 
	short int sb_inodeBitmap[15694];		// Inode������ 
};

