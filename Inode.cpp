#include "Inode.h"
#include <ctime>

Inode::Inode()
{
	this->i_type =0;
	this->i_size = 0;
	this->i_ctime = 0;
	this->i_mtime = 0;
	this->i_idaddr = 0;
	for (int i = 0; i < 10; i++)
		this->i_daddr[i] = 0;
}


void Inode::clearInode()
{
	this->i_type = 0;
	this->i_size = 0;
	this->i_ctime = 0;
	this->i_mtime = 0;
	this->i_idaddr = 0;
	for (int i = 0; i < 10; i++)
		this->i_daddr[i] = 0;
}
