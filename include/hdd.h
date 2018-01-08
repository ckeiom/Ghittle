/* This file is initally written by Hyunsub Song on 2017.12.27 */
#ifndef __HDD_H__
#define __HDD_H__

#include <sync.h>
#include <dio.h>
#include <memutils.h>
#include <timer.h>
#include <console.h>
#include <pic.h>

/* Information of first and second PARA ports */
#define HDD_PORT_PRIMARYBASE				0x1F0
#define HDD_PORT_SECONDARYBASE				0x170

/* Macro related on port index */
#define HDD_PORT_INDEX_DATA					0x00
#define HDD_PORT_INDEX_SECTORCOUNT			0x02
#define HDD_PORT_INDEX_SECTORNUMBER			0x03
#define HDD_PORT_INDEX_CYLINDERLSB			0x04
#define HDD_PORT_INDEX_CYLINDERMSB			0x05
#define HDD_PORT_INDEX_DRIVEANDHEAD			0x06
#define HDD_PORT_INDEX_STATUS				0x07
#define HDD_PORT_INDEX_COMMAND				0x07
#define HDD_PORT_INDEX_DIGITALOUTPUT		0x206

/* Macro related on command register */
#define HDD_COMMAND_READ					0x20
#define HDD_COMMAND_WRITE					0x30
#define HDD_COMMAND_IDENTIFY				0xEC

/* Macro related on status register */
#define HDD_STATUS_ERROR					0x01
#define HDD_STATUS_INDEX					0x02
#define HDD_STATUS_CORRECTEDDATA			0x04
#define HDD_STATUS_DATAREQUEST				0x08
#define HDD_STATUS_SEEKCOMPLETE				0x10
#define HDD_STATUS_WRITEFAULT				0x20
#define HDD_STATUS_READY					0x40
#define HDD_STATUS_BUSY						0x80

/* Macro related on drive/head register */
#define HDD_DRIVEANDHEAD_LBA				0xE0
#define HDD_DRIVEANDHEAD_SLAVE				0x10

/* Macro related on digital output register */
#define HDD_DIGITALOUTPUT_RESET				0x04
#define HDD_DIGITALOUTPUT_DISABLEINTERRUPT	0x01

#define HDD_WAITTIME						500
#define HDD_MAXBULKSECTORCOUNT				256

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

struct hdd_mgr
{
	unsigned char detected;
	unsigned char write;

	volatile unsigned char p_intr;
	volatile unsigned char s_intr;
	struct mutex mut;

	struct hdd_info h_info;
};

unsigned char init_hdd( void );
unsigned char read_hdd_info( unsigned char p,
							 unsigned char m,
							 struct hdd_info *h_info);
int read_hdd_sector( unsigned char p, unsigned char m,
					 unsigned int lba, int sector_count,
					 char *buf);
int write_hdd_sector( unsigned char p, unsigned char m,
		              unsigned int lba, int sector_count,
					  char *buf);
void set_hdd_intr_flag( unsigned char p, unsigned char flag );

static void swap_byte_in_word( unsigned short *data, int wcount );
static unsigned char read_hdd_stat( unsigned char p );
static unsigned char wait_hdd_nobusy( unsigned char p );
static unsigned char wait_hdd_ready( unsigned char p );
static unsigned char wait_hdd_intr( unsigned char p );


#endif
