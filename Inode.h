#pragma once

//40B per inode, that's 1024B/40B=25 inodes per block
class Inode
{
public:
	Inode();
	~Inode() {};

public:
	unsigned short int i_type;				// regular file (0) or directory (1)
	unsigned int i_size;							// size of file (or directory file)
	unsigned long int i_ctime;				// time that this file is created
	unsigned long int i_mtime;				//time that this file is modified
	unsigned short int i_daddr[10];		//direct address array
	unsigned short int i_idaddr;			//indirect address
};
