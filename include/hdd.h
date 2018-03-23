/* This file is initally written by Hyunsub Song on 2017.12.27 */
#ifndef __HDD_H__
#define __HDD_H__

#include <sync.h>
#include <dio.h>
#include <memutils.h>
#include <timer.h>
#include <console.h>
#include <pic.h>


#define HDD_PRIMARY		1
#define HDD_SECONDARY	0
#define HDD_MASTER		1
#define HDD_SLAVE		0

#define HDD_PORT_PRIMARY				0x1F0
#define HDD_PORT_SECONDARY				0x170

#define HDD_INDEX_DATA					0x00
#define HDD_INDEX_SECTORCOUNT			0x02
#define HDD_INDEX_SECTORNUMBER			0x03
#define HDD_INDEX_CYLINDERLSB			0x04
#define HDD_INDEX_CYLINDERMSB			0x05
#define HDD_INDEX_DRIVEANDHEAD			0x06
#define HDD_INDEX_STATUS				0x07
#define HDD_INDEX_COMMAND				0x07
#define HDD_INDEX_DIGITALOUTPUT		0x206

#define HDD_COMMAND_READ					0x20
#define HDD_COMMAND_WRITE					0x30
#define HDD_COMMAND_IDENTIFY				0xEC

#define HDD_STATUS_ERROR					0x01
#define HDD_STATUS_INDEX					0x02
#define HDD_STATUS_CORRECTEDDATA			0x04
#define HDD_STATUS_DATAREQUEST				0x08
#define HDD_STATUS_SEEKCOMPLETE				0x10
#define HDD_STATUS_WRITEFAULT				0x20
#define HDD_STATUS_READY					0x40
#define HDD_STATUS_BUSY						0x80

#define HDD_DRIVEANDHEAD_LBA				0xE0
#define HDD_DRIVEANDHEAD_SLAVE				0x10

#define HDD_DIGITALOUTPUT_RESET				0x04
#define HDD_DIGITALOUTPUT_DISABLEINTERRUPT	0x01

#define HDD_WAITTIME						500 // ticks
#define HDD_MAXBULKSECTORCOUNT				256
#define HDD_SECTOR_SIZE		512
#define HDD_INTERRUPT		1
#define HDD_NOINTERRUPT		0

#pragma pack( push, 1 )

struct hdd_info
{
	unsigned short config;

	unsigned short cyl_count;
	unsigned short reserved1;

	unsigned short head_count;
	unsigned short track_bytes;
	unsigned short sector_bytes;

	/* Per cylinder */
	unsigned short sector_count;
	unsigned short gap_sector;
	unsigned short phaselock_bytes;
	unsigned short word_count;

	unsigned short serial[10];
	unsigned short cntr_type;
	unsigned short buf_size;
	unsigned short ecc_bytes;
	unsigned short firm_revision[4];

	unsigned short model[20];
	unsigned short reserved2[13];

	unsigned int total_sector;
	unsigned short reserved3[196];
};

#pragma pack( pop )

struct hdd
{
	unsigned char detected;
	unsigned char writable;

	volatile unsigned char int_primary;
	volatile unsigned char int_secondary;
	struct mutex mutex;

	struct hdd_info hdd_info;
};

int init_hdd(void);
int read_hdd_info(unsigned char primary, unsigned char master, 
		          struct hdd_info *hdd_info);
int read_hdd_sector(unsigned char primary, unsigned char master,
					unsigned int lba, int sectors, char *buf);
int write_hdd_sector(unsigned char primary, unsigned char master,
		             unsigned int lba, int sectors, char *buf);
void set_hdd_interrupt_state(unsigned char primary, unsigned char state);

static void swap_byte_in_word(unsigned short *data, int word_count);
static unsigned char read_hdd_stat(unsigned char primary);
static int wait_hdd_nobusy(unsigned char primary);
static int wait_hdd_ready(unsigned char primary);
static int wait_hdd_interrupt(unsigned char primary);


#endif
