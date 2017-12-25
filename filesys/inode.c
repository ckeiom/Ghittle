#include <inode.h>
#include <types.h>

int root_inode;

void init_inode(void)
{
	root_inode = NUM_BOOT_SECTOR + NUM_KERNEL_SECTOR + 1;
	

}
