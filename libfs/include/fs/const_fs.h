#pragma once

#include <string>
using std::string;
#include "windows.h"

namespace FileSystem
{
	enum FSType {Unknown = 0, FAT12  = 1, FAT16 = 2, FAT32 = 3, NTFS = 4};

	// mbr partition types
	
	enum partition_types
	{
		free = 0x00,
		fat12_chs = 0x01,
		fat16_chs_small = 0x04,
		extended_chs = 0x05,
		fat16_chs_large = 0x06,
		fat32_chs = 0x0b,
		fat32_lba = 0x0c,
		fat16_lba_large = 0x0e,
		extended_lba = 0x0f
	};

	const DWORD ms_dos_name = 11;
	const string one_dot	= ".          ";
	const string double_dot = "..         ";
	const WORD boot_signature = 0xAA55;
	const WORD boot_signature_offset = 510;

	const string fat16_name =	"FAT16";
	const string fat32_name =	"FAT32";
	const string ntfs_name  =	"NTFS";
	

	//const DWORD default_sector_size = 512;

	//	bad clusters marker
	const WORD bad_cluster_fat12 = 0x0FF7;	
	const WORD bad_cluster_fat16 = 0xFFF7;
	const DWORD bad_cluster_fat32 = 0x0FFFFFF7;
	//	first reserved clusters  marker
	const WORD	first_cluster_fat12 = 0x0FF8;	
	const WORD	first_cluster_fat16 = 0xFFF8;
	const DWORD first_cluster_fat32 = 0x0FFFFFF8;

	//	end clusters  marker
	const DWORD end_cluster_fat32 = 0x0FFFFFFF;

	//	sizes
	const BYTE fat16_cell	= 2;
	const BYTE fat32_cell	= 4;

	const BYTE dir_entry_size = 32;


	// directory attributes
	const BYTE attr_read_only	= 0x01;
	const BYTE attr_hidden		= 0x02;
	const BYTE attr_system		= 0x04;
	const BYTE attr_volume_id	= 0x08;
	const BYTE attr_directory	= 0x10;
	const BYTE attr_archive		= 0x20;
	const BYTE attr_long_name	= attr_read_only | 
								  attr_hidden |
								  attr_system |
								  attr_volume_id;
	const BYTE attr_long_name_mask = attr_read_only | 
									 attr_hidden |
									 attr_system |
									 attr_volume_id |
									 attr_directory |
									 attr_archive;
	const BYTE attr_last_log_entry = 0x40;
	const BYTE attr_file = attr_read_only | attr_hidden | attr_system;
}