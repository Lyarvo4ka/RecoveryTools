#pragma once

#include "FileSystemDLL.h"
#include "const_fs.h"

#include <memory>

namespace FileSystem
{
	#pragma pack(1)
	struct fat_boot_sector {
	/* 0*/	BYTE	ignored[3];			/* Boot strap short or near jump */
	/* 3*/	BYTE	system_id[8];		/* Name - can be used to special case
										partition manager volumes */
	/* 11*/	WORD	BytesPerSector;		/* bytes per logical sector */
	/* 13*/	BYTE	sec_per_clus;		/* sectors/cluster */
	/* 14*/	WORD	reserved_sect;		/* reserved sectors */
	/* 16*/	BYTE	number_fats;		/* number of FATs */
	/* 17*/	WORD	root_entries;		/* root directory entries */
	/* 19*/	WORD	total_sect_small;	/* number of sectors */
	/* 21*/	BYTE	media;				/* media code */
	/* 22*/	WORD	fat_length_small;	/* sectors/FAT */
	/* 26*/	WORD	secs_track;			/* sectors per track */
	/* 28*/	WORD	heads;				/* number of heads */
	/* 28*/	DWORD	hidden;				/* hidden sectors (unused) */
	/* 32*/	DWORD	total_sect_large;	/* number of sectors (if sectors == 0) */

	/* The following fields are only used by FAT32 */
	/* 36*/	DWORD	fat_length_large;	/* sectors/FAT */
	/* 40*/	WORD	flags;				/* bit 8: fat mirroring, low 4: active fat */
	/* 42*/	WORD	FSVersion;			/* major, minor filesystem version */
	/* 44*/	DWORD	root_cluster;		/* first cluster in root directory */
	/* 48*/	WORD	info_sector;		/* filesystem info sector */
	/* 50*/	WORD	backup_boot;		/* backup boot sector */
	/* 52*/	WORD	reserved2[6];		/* Unused */
	/* 64*/	BYTE	BIOSdrive;			/* BIOS drive (hex, HD=8x) */
	/* 65*/	BYTE	reserved3;			/* Unused */ 
	/* 66*/	BYTE	boot_signature;		/*29h */
	/* 67*/ DWORD	volume_id;			/*Volume serial number*/
	/* 71*/ BYTE	volume_label[11];	/*Volume label*/
	/* 82*/ BYTE	file_system[8];		/*File system (FAT16, FAT32)*/
	///* 83*/ BYTE	other_data[428];
	///*510*/ WORD	Signature;			/*Signature 0x55AA*/
	};
	#pragma pack ()

	#pragma pack(1)
	struct msdos_dir_entry {
	/* 0*/	char	name[ms_dos_name];	/* name and extension */
	/*11*/	BYTE	attr;				/* attribute bits */
	/*12*/	BYTE    lcase;				/* Case for base and extension */
	/*13*/	BYTE	time_create_ms;		/* Creation time, milliseconds (0-199) */
	/*14*/	WORD	time_create;		/* Creation time */
	/*16*/	WORD	data_create;		/* Creation date */
	/*18*/	WORD	date_access;		/* Last access date */
	/*20*/	WORD	cluster_high;		/* High 16 bits of cluster in FAT32 */
	/*22*/	WORD	time_modify;		/* time of last write */
	/*24*/	WORD	date_modify;		/* data of last write */
	/*26*/	WORD	cluster_low;		/* low 16 bits of cluster */
	/*28*/	DWORD	file_size;			/* file size (in bytes) */
	};
	#pragma pack ()

	const int size_first_name = 5;
	const int size_second_name = 6;
	const int size_third_name = 2;

	const int size_long_name = size_first_name + size_second_name + size_third_name;

#pragma pack(1)
	struct msdos_long_dir_entry {
	/* 0*/	BYTE order_number;
	/* 1*/	WCHAR first_name[size_first_name];
	/*11*/	BYTE attr;
	/*12*/	BYTE reserved1;
	/*13*/	BYTE checksum;
	/*14*/	WCHAR second_name[size_second_name];
	/*26*/	WORD reserved2;
	/*28*/	WCHAR third_name[size_third_name];
	};
#pragma pack ()

	bool  CheckSignature(const BYTE *BootSector);
	FSType  GetFATType(const fat_boot_sector & BootSector);


	class  IBootSector
	{
	public:
		IBootSector() {/**/};
		virtual void InitBoot() = 0;
		virtual string TypeName() const = 0;
		virtual ~IBootSector(){/**/};
	};

	class  Fat16_boot
		: IBootSector
	{
	public:
		Fat16_boot(const fat_boot_sector & BootSector);
		~Fat16_boot(){/**/};
		virtual void InitBoot();
		virtual string TypeName() const;
		WORD Sectors() const;
		WORD FatSectors() const;
	private:
		fat_boot_sector BootSector_;
		WORD Sectors_;
		WORD FatSectors_;
	};

	class  Fat32_boot
		: IBootSector
	{
	public:
		Fat32_boot(const fat_boot_sector & BootSector);
		Fat32_boot(const BYTE *BootSector );
		~Fat32_boot(){/**/};
		void InitBoot();
		virtual string TypeName() const;
		DWORD sectors() const;				// total sectors 
		DWORD sectors_per_fat() const;		// 
		DWORD count_cluster() const;		// count clusters in partition
		DWORD root_cluster() const;			// number of root cluster
		DWORD data_sectors() const;			// start sector in data area
		DWORD root_entries() const;			
		DWORD cluster_size() const;
		DWORD bytes_per_sector() const;
		DWORD reserved_sectors() const;
		DWORD hidden() const;
		BYTE sectors_per_cluster() const;

	private:
		fat_boot_sector BootSector_;
		DWORD CountCluster_;
		DWORD DataSectors_;
		DWORD RootEntries_;
	};

	typedef std::shared_ptr<Fat32_boot> FatBootPtr;
}