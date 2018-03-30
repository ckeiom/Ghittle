#include <hdd.h>

static struct hdd hdd;

int init_hdd(void)
{
	init_mutex(&hdd.mutex);

	hdd.int_primary = 0;
	hdd.int_secondary = 0;
	
	out_b(HDD_PORT_PRIMARY + HDD_INDEX_DIGITALOUTPUT, 0);
	out_b(HDD_PORT_SECONDARY + HDD_INDEX_DIGITALOUTPUT, 0);
	
	if(read_hdd_info(HDD_PRIMARY, HDD_MASTER, &hdd.hdd_info) < 0)
	{
		hdd.detected = 0;
		hdd.writable = 0;
		return -1;
	}

	hdd.detected = 1;
	if(memcmp(hdd.hdd_info.model, "QEMU", 4) == 0)
		hdd.writable = 1;
	else
		hdd.writable = 0;

	return 0;
}

static unsigned char read_hdd_stat(unsigned char primary)
{
	if(primary)
		return in_b(HDD_PORT_PRIMARY + HDD_INDEX_STATUS);

	return in_b(HDD_PORT_SECONDARY + HDD_INDEX_STATUS);
}

static int wait_hdd_nobusy(unsigned char primary)
{
	unsigned long tick_start;
	unsigned char stat;

	tick_start = tick;

	while((tick - tick_start) <= HDD_WAITTIME)
	{
		stat = read_hdd_stat(primary);
		if(!(stat & HDD_STATUS_BUSY))
			return 0;
		
		/* On implementing multi-tasking version, 
		   mdelay() changes to schedule() */
		mdelay(1); 
	}
	return -1;
}

int read_hdd_info(unsigned char primary,
				  unsigned char master,
				  struct hdd_info *hdd_info)
{
	unsigned short port;
	unsigned long last_tcount;
	unsigned char stat;
	unsigned char dflag;
	int i;
	unsigned short temp;
	unsigned char wait_result;

	if(primary)
		port = HDD_PORT_PRIMARY;
	else
		port = HDD_PORT_SECONDARY;

	lock(&hdd.mutex);
	
	if(wait_hdd_nobusy(primary) < 0)
		goto err_out;

	if(master)
		dflag = HDD_DRIVEANDHEAD_LBA;
	else
		dflag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

	out_b(port + HDD_INDEX_DRIVEANDHEAD, dflag);

	if(wait_hdd_ready(primary) < 0)
		goto err_out;

	set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
	out_b(port + HDD_INDEX_COMMAND, HDD_COMMAND_IDENTIFY);

	if(wait_hdd_interrupt(primary) < 0)
		goto err_out;

	stat = read_hdd_stat(primary);
	if(stat & HDD_STATUS_ERROR)
		goto err_out;

	/* Read one sector */
	for(i = 0; i < HDD_SECTOR_SIZE / sizeof(unsigned short); i++)
		((unsigned short* )hdd_info)[i] = in_w(port + HDD_INDEX_DATA);

	swap_byte_in_word(hdd_info->model, sizeof(hdd_info->model) / 2);
	swap_byte_in_word(hdd_info->serial, sizeof(hdd_info->serial) / 2);

	unlock(&hdd.mutex);
	return 0;

err_out:
	unlock(&hdd.mutex);
	return -1;
}

static void swap_byte_in_word(unsigned short *data, int word_count)
{
	int i;
	unsigned short tmp;

	for(i = 0; i < word_count; i++)
	{
		tmp = data[i];
		data[i] = (tmp >> 8) | (tmp << 8);
	}
}

static int wait_hdd_ready(unsigned char primary)
{
	unsigned long tick_start;
	unsigned char stat;

	tick_start = tick;

	while((tick - tick_start) <= HDD_WAITTIME)
	{
		stat = read_hdd_stat(primary);
		if(stat & HDD_STATUS_READY)
			return 0;
		
		/* On implementing multi-tasking version, 
		   mdelay() changes to schedule() */
		mdelay(1);
	}
	return -1;
}

static int wait_hdd_interrupt(unsigned char primary)
{
	unsigned long tick_start;

	tick_start = tick;

	while((tick - tick_start) <= HDD_WAITTIME)
	{
		if(primary && hdd.int_primary)
			return 0;
		else if(!primary && hdd.int_secondary)
			return 0;

		mdelay(1);
	}
	return -1;
}

void set_hdd_interrupt_state(unsigned char primary, unsigned char state)
{
	if(primary)
		hdd.int_primary = state;
	else
		hdd.int_secondary = state;
}

int read_hdd_sector(unsigned char primary, unsigned char master,
					unsigned int lba, int sectors, char *buf)
{
	unsigned short port;
	int i, j;
	unsigned char dflag;
	unsigned char stat;
	
	if(sectors <= 0 || sectors > 256)
		return -1;

	lock(&hdd.mutex);
	
	if(!hdd.detected)
		goto err_out;

	if((lba + sectors) >= hdd.hdd_info.total_sector)
		goto err_out;
	
	if(primary)
		port = HDD_PORT_PRIMARY;
	else
		port = HDD_PORT_SECONDARY;

	if(wait_hdd_nobusy(primary) < 0)
		goto err_out;

	out_b(port + HDD_INDEX_SECTORCOUNT, sectors);
	out_b(port + HDD_INDEX_SECTORNUMBER, lba);
	out_b(port + HDD_INDEX_CYLINDERLSB, lba >> 8);
	out_b(port + HDD_INDEX_CYLINDERMSB, lba >> 16);
				
	if(master)
		dflag = HDD_DRIVEANDHEAD_LBA;
	else
		dflag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

	out_b(port + HDD_INDEX_DRIVEANDHEAD, dflag | ((lba >> 24) & 0x0F));

	if(wait_hdd_ready(primary) < 0)
		goto err_out;

	set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
	out_b(port + HDD_INDEX_COMMAND, HDD_COMMAND_READ);

	/* Receive data after waiting interrupt */
	for(i = 0; i < sectors; i++)
	{
		stat = read_hdd_stat(primary);
		if(stat & HDD_STATUS_ERROR)
			goto err_out;

		if(!(stat & HDD_STATUS_DATAREQUEST))
		{
			if(wait_hdd_interrupt(primary) < 0)
			{
				set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
				goto err_out;
			}
			set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
		}
		
		for(j = 0; j < HDD_SECTOR_SIZE / sizeof(unsigned short); j++)
			((unsigned short* )buf)[j] = in_w(port + HDD_INDEX_DATA);
		buf += HDD_SECTOR_SIZE;
	}
	unlock(&hdd.mutex);
	return 0;

err_out:
	unlock(&hdd.mutex);
	return -1;
}

int write_hdd_sector(unsigned char primary, unsigned char master,
		             unsigned int lba, int sectors, char *buf)
{
	unsigned short port;
	unsigned short tmp;
	int i, j;
	unsigned char dflag;
	unsigned char stat;
	long rcount = 0;
	unsigned char wait_result;

	if((sectors <= 0) || (sectors > 256))
		return -1;

	lock(&hdd.mutex);

	if((!hdd.detected) || (!hdd.writable))
		goto err_out;

	if((lba + sectors) >= hdd.hdd_info.total_sector)
		goto err_out;

	if(primary)
		port = HDD_PORT_PRIMARY;
	else
		port = HDD_PORT_SECONDARY;

	if(wait_hdd_nobusy(primary) < 0)
		goto err_out;

	out_b(port + HDD_INDEX_SECTORCOUNT, sectors);
	out_b(port + HDD_INDEX_SECTORNUMBER, lba);
	out_b(port + HDD_INDEX_CYLINDERLSB, lba >> 8);
	out_b(port + HDD_INDEX_CYLINDERMSB, lba >> 16);

	if(master)
		dflag = HDD_DRIVEANDHEAD_LBA;
	else
		dflag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

	out_b(port + HDD_INDEX_DRIVEANDHEAD, dflag | ((lba >> 24) & 0x0F));

	if(wait_hdd_ready(primary) < 0)
		goto err_out;

	set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
	out_b(port + HDD_INDEX_COMMAND, HDD_COMMAND_WRITE);

	for(i = 0; i < sectors; i++)
	{
		set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
		for(j = 0; j < HDD_SECTOR_SIZE / sizeof(unsigned short); j++)
			out_w(port + HDD_INDEX_DATA, ((unsigned short *) buf)[j]);

		stat = read_hdd_stat(primary);
		if(stat & HDD_STATUS_ERROR)
			goto err_out;

		if(!(stat & HDD_STATUS_DATAREQUEST))
		{
			if(wait_hdd_interrupt(primary) < 0)
			{
				set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
				goto err_out;
			}
			set_hdd_interrupt_state(primary, HDD_NOINTERRUPT);
		}
		buf += HDD_SECTOR_SIZE;
	}
	unlock(&hdd.mutex);
	return 0;

err_out:
	unlock(&hdd.mutex);
	return -1;
}


