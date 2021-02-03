#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <windows.h>
#include <string>
#include <list>
#include <stdint.h>

#define SIZEOF_ARRAY( a ) (sizeof( a ) / sizeof( a[ 0 ] ))

//typedef unsigned char		uint8_t;
//typedef unsigned long		uint32_t;
//typedef unsigned long long  uint64_t;

const uint64_t ERROR_RESULT = _UI64_MAX;
const uint64_t ERROR_OPEN_FILE = _UI64_MAX - 1;
const uint64_t ERROR_READ_FILE = _UI64_MAX - 2;
const uint64_t FOUND_NEW_HEADER = _UI64_MAX - 3;




namespace IO
{
	using path_string = std::wstring;
	using path_list = std::list<path_string>;
	using ByteArray = uint8_t *;

	struct Buffer
	{
		uint32_t data_size;
		ByteArray data;

		Buffer(const uint32_t buffer_size)
			:data_size(buffer_size)
			, data(new uint8_t[buffer_size])
		{

		}
		~Buffer()
		{
			if (data)
			{
				delete[] data;
				data = nullptr;
			}
		}
	};

	//using BufferPtr = shared_ptr<Buffer>;


	enum class OpenMode : uint32_t { OpenRead, OpenWrite, Create };

	const wchar_t back_slash = L'\\';
	const path_string UNC_prefix = L"\\\\?\\";
	const path_string OneDot = L".";
	const path_string TwoDot = L"..";
	const path_string mask_all = L"*.*";

	const path_list listExtensions2007 = { L".docx",L".xlsx",L".pptx" };
	const path_list listExtensions2003 = { L".doc",L".xls",L".ppt" };


	inline void toBE64(uint64_t & val)
	{
		val = _byteswap_uint64(val);
	}
	inline void toBE32(uint32_t & val)
	{
		val = _byteswap_ulong(val);
	}
	inline void toBE16(uint16_t & val)
	{
		val = _byteswap_ushort(val);
	}


};

#include <array>
#include <vector>

namespace MLVKeywords
{

	static const char *mlv_array[] =	{ "MLVI", "VIDF", "AUDF", "RAWI", "WAVI", "EXPO", "LENS", "RTCI", 
										  "IDNT", "XREF", "INFO", "DISO", "NULL", "ELVL", "WBAL", "STYL", 
										  "MARK", "SUBC", "SUBI", "COLP", "DARK", "VIGN", "VERS", "DEBG" };
	constexpr auto mlv_array_size_v2 = std::size(mlv_array);
	const int mlv_array_size = SIZEOF_ARRAY(mlv_array);

	//constexpr std::array mlv_array_v2 = { "MLVI", "VIDF", "AUDF", "RAWI", "WAVI", "EXPO", "LENS", "RTCI",
	//									  "IDNT", "XREF", "INFO", "DISO", "NULL", "ELVL", "WBAL", "STYL",
	//									  "MARK", "SUBC", "SUBI", "COLP", "DARK", "VIGN", "VERS", "DEBG" };


};



namespace Signatures
{

	const BYTE qt_header[] =  { 0x00 , 0x00 , 0x00 , 0x18 , 0x66 , 0x74 , 0x79 , 0x70 , 0x71 , 0x74 , 0x20 , 0x20 , 0x20 , 0x07 , 0x09 , 0x00 };
	const BYTE qt_header2[] = { 0x00 , 0x00 , 0x00 , 0x20 , 0x66 , 0x74 , 0x79 , 0x70 , 0x71 , 0x74 , 0x20 , 0x20 , 0x20 , 0x05 , 0x03 , 0x00 };
	const BYTE qt_header3[] = { 0x00 , 0x00 , 0x00 , 0x14 , 0x66 , 0x74 , 0x79 , 0x70 , 0x71 , 0x74 , 0x20 , 0x20 , 0x00 , 0x00 , 0x00 , 0x00 };
	const BYTE qt_header4[] = { 0x00 , 0x00 , 0x00 , 0x20 , 0x66 , 0x74 , 0x79 , 0x70 , 0x61 , 0x76 , 0x63 , 0x31 , 0x00 , 0x00 , 0x00 , 0x00 };
	const BYTE qt_header5[] = { 0x00 , 0x00 , 0x00 , 0x1C , 0x66 , 0x74 , 0x79 , 0x70 , 0x6D , 0x70 , 0x34 , 0x32 , 0x00 , 0x00 , 0x00 , 0x01 };
	const BYTE qt_header_def[] = { 0x00 , 0x00 , 0x00 , 0x20 , 0x66 , 0x74 , 0x79 , 0x70 };

	const BYTE qt_header_hts[] = { 0x00 , 0x00 , 0x00 , 0x18 , 0x66 , 0x74 , 0x79 , 0x70 , 0x69 , 0x73 , 0x6F , 0x6D , 0x00 , 0x00 , 0x00 , 0x00 };
	const int qt_header_size = SIZEOF_ARRAY( qt_header );
	const std::string moov = "mdat";
	const BYTE mdat[] = { 0x6D , 0x64 , 0x61 , 0x74 };
	const int mdat_size = SIZEOF_ARRAY( mdat );
	const BYTE dvr_header[] = { 0x22 , 0x00 , 0x00 , 0x00 , 0x03 , 0x00 , 0x00 , 0x00 , 0x07 , 0x02 , 0x03 , 0x16 , 0x02 , 0x01 , 0x14 , 0x90 };
	const int dvr_header_size  = SIZEOF_ARRAY( dvr_header );
	const BYTE mxf_header[] = { 0x06 , 0x0E , 0x2B , 0x34 , 0x02 , 0x05 , 0x01 , 0x01 , 0x0D , 0x01 , 0x02 , 0x01 , 0x01 , 0x02 , 0x04 , 0x00};
	const int mxf_header_size = SIZEOF_ARRAY( mxf_header );
	const BYTE mxf_footer[] = { 0x06 , 0x0E , 0x2B , 0x34 , 0x02 , 0x05 , 0x01 , 0x01 , 0x0D , 0x01 , 0x02 , 0x01 , 0x01 , 0x04 , 0x04 , 0x00 };
	const int mxf_footer_size = SIZEOF_ARRAY( mxf_header );

	const BYTE at_t_header[] = {0x41 , 0x54 , 0x26 , 0x54 , 0x46 , 0x4F , 0x52 , 0x4D};
	const int at_t_header_size = SIZEOF_ARRAY( at_t_header );
	//const BYTE djvu_header[] = {0x44 , 0x4A , 0x56 , 0x4D , 0x44 , 0x49 , 0x52 , 0x4D};
	//const int djvu_header_size = SIZEOF_ARRAY( djvu_header );

	const BYTE r3d_header[] = { 0x00 , 0x00 , 0x03 , 0x72 , 0x52 , 0x45 , 0x44 , 0x32 };
	const int r3d_header_size = SIZEOF_ARRAY( r3d_header);
	const BYTE r3d_footer[] = { 0x00 , 0x00 , 0x03 , 0x92 , 0x52 , 0x45 , 0x4F };
	const int r3d_footer_size = SIZEOF_ARRAY( r3d_footer);

	const BYTE mts_header[] = { 0x47, 0x40, 0x00, 0x10, 0x00, 0x00, 0xB0, 0x11, 0x00 };
	const int mts_header_size = SIZEOF_ARRAY(mts_header);
	const int mts_header_offset = 4;
	const BYTE mts_footer[] = { 0x47, 0x1F, 0xFF, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	const int mts_footer_size = SIZEOF_ARRAY(mts_footer);


	const BYTE bad_sector_header[] = { 0x62, 0x61, 0x64, 0x20, 0x73, 0x65, 0x63, 0x74, 0x6F, 0x72 };
	const uint32_t bad_sector_header_size = SIZEOF_ARRAY(Signatures::bad_sector_header);

	const BYTE marker_0x27[] = { 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27 };
	const int marker_0x27_size = SIZEOF_ARRAY(marker_0x27);


	const uint8_t psd_header[] = { 0x38 , 0x42 , 0x50 , 0x53 , 0x00 , 0x01 };
	const int psd_header_size = SIZEOF_ARRAY(psd_header);

	const uint8_t cdr_header[] = { 0x52 , 0x49 , 0x46 , 0x46 };
	const int cdr_header_size = SIZEOF_ARRAY(cdr_header);

	const uint8_t tif_header[] = { 0x49 , 0x49 , 0x2A };
	const int tif_header_size = SIZEOF_ARRAY(tif_header);

	const uint8_t tif2_header[] = { 0x4D , 0x4D , 0x00 , 0x2A };
	const int tif2_header_size = SIZEOF_ARRAY(tif2_header);

	const uint8_t ai_header[] = { 0xC5 , 0xD0 , 0xD3 , 0xC6 , 0x20 , 0x00 , 0x00 , 0x00 };
	const int ai_header_size = SIZEOF_ARRAY(ai_header);

	const uint8_t office_2007_header[] = { 0x50 , 0x4B , 0x03 , 0x04 , 0x14 , 0x00 , 0x06 , 0x00 , 0x08 , 0x00 , 0x00 , 0x00 , 0x21 , 0x00 };
	const int office_2007_header_size = SIZEOF_ARRAY(office_2007_header);

	const uint8_t office_2003_header[] = { 0xD0 , 0xCF , 0x11 };
	const int office_2003_header_size = SIZEOF_ARRAY(office_2003_header);

	const uint8_t pdf_header[] = { 0x25 ,0x50 ,0x44 ,0x46 ,0x2D };
	const int pdf_header_size = SIZEOF_ARRAY(pdf_header);

	const uint8_t jpg_header[] = { 0xFF, 0xD8, 0xFF };
	const uint8_t jpg_header_size = SIZEOF_ARRAY(jpg_header);

	const uint8_t bad_sector_marker[] = { 0x62 , 0x61 , 0x64 , 0x20 , 0x73 , 0x65 , 0x63 , 0x74 , 0x6F , 0x72 };
	const uint32_t bad_sector_marker_size = SIZEOF_ARRAY(bad_sector_marker);

	const uint8_t zbk_header[] = { 0x0F , 0x00 , 0x0B , 0x67 , 0x63 , 0x66 , 0x67 , 0x50 , 0x72 , 0x67 , 0x50 , 0x6C , 0x75 , 0x73 };
	const uint32_t zbk_header_size = SIZEOF_ARRAY(zbk_header);
	const uint8_t zbk_footer[] = { 0x50 , 0x4B , 0x05 , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 };
	const uint32_t zbk_footer_size = SIZEOF_ARRAY(zbk_footer);

	using u8 = unsigned char;
	using u16 = unsigned short;
	struct zbh_header_t
	{
		u16 reserver0[8];
		u8 day;
		u8 month;
		u16 year;
		u8 hour;
		u8 seconds;
		u8 reserved1[5];

	};
	const uint8_t c_riff[] = { 0x52,0x49,0x46,0x46 };
	const uint8_t c_mpeg_header[] = { 0x00 , 0x00 , 0x01 , 0xBA , 0x44 , 0x00 , 0x04 , 0x00 };

	const uint8_t dbf_header_1 = 0x03;
	const uint8_t dbf_header_2[] = { 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 };
	const uint32_t dbf_header_2_offset = 0x10;



};

const int SECTOR_SIZE = 512;
const int default_sector_size = 512;
const int defalut_number_sectors = 128;
const int default_block_size = defalut_number_sectors * default_sector_size;
const int SECTOR_COUNT = 64;
const int BLOCK_SIZE = SECTOR_SIZE * SECTOR_COUNT;

namespace BS
{
const int KB = 1024;
const int MB = KB*KB;
const int GB = MB*KB;
};


#define  IDE_ATA_IDENTIFY    0xEC
#pragma pack (1)
// ATA8 specification
typedef struct _IDENTIFY_DISK_ATA					// Word(dec)
{
	USHORT		GeneralConfiguration;				// 000
	USHORT		CylindersCHS;// Obsolete			// 001
	USHORT		SpecificConfiguration;				// 002
	USHORT		HeadsCHS;// Obsolete				// 003
	USHORT		Retired1[2];						// 004-005
	USHORT		SectorsCHS;// Obsolete				// 006
	USHORT		ReservedCFA1[2];					// 007-008
	USHORT		Retired2;							// 009
	UCHAR		SerialNumber[20];					// 010-019
	USHORT		Retired3[2];						// 020-021
	USHORT		NumberBytesLong;// Obsolete			// 022
	UCHAR		FirmwareRevision[8];				// 023-026
	UCHAR		ModelNumber[40];					// 027-046
	USHORT		MaxNumSectorsForMultiple;			// 047
	USHORT		TrustedComputingSupported;			// 048
	ULONG		Capabilities;						// 049-050
	USHORT		SettingsTimePIO;// Obsolete			// 051
	USHORT		SettingsTimeDMA;// Obsolete			// 052
	USHORT		SupportedModes;						// 053
	USHORT		CylindersCurrent;// Obsolete		// 054
	USHORT		HeadsCurrent;// Obsolete			// 055
	USHORT		SectorsCurrent;// Obsolete			// 056
	ULONG		CapacityCHS;// Obsolete				// 057-058
	USHORT		NumSectorsForMultiple;				// 059
	ULONG		TotalNumberLBA28;					// 060-061
	USHORT		ModeDMA;// Obsolete					// 062
	USHORT		ModeMultiwordDMA;					// 063
	USHORT		ModePIO;							// 064
	USHORT		MinCycleTimeMultiwordDMA;			// 065
	USHORT		VendorCycleTimeMultiwordDMA;		// 066
	USHORT		MinCycleTimePIOwithoutIORDY;		// 067
	USHORT		MinCycleTimePIOwithIORDY;			// 068
	USHORT		Reserved1[2];						// 069-070
	USHORT		ReservedIdentifyPacketDevice[4];	// 071-074
	USHORT		QueueDepth;							// 075
	USHORT		CapabilitiesSATA;					// 076
	USHORT		ReservedSATA;						// 077
	USHORT		FeaturesSupportedSATA;				// 078
	USHORT		FeaturesEnabledSATA;				// 079
	USHORT		MajorVersionNumber;					// 080
	USHORT		MinorVersionNumber;					// 081
	USHORT		FeatureSetSupported1;				// 082
	USHORT		FeatureSetSupported2;				// 083
	USHORT		FeatureSetSupported3;				// 084
	USHORT		FeatureSetSupportedOrEnabled1;		// 085
	USHORT		FeatureSetSupportedOrEnabled2;		// 086
	USHORT		FeatureSetSupportedOrEnabled3;		// 087
	USHORT		ModeUltraDMA;						// 088
	USHORT		TimeSecuriteEraseNormal;			// 089
	USHORT		TimeSecuriteEraseEnhanced;			// 090
	USHORT		CurrentAdvancedPowerManagementLevel;// 091
	USHORT		MasterPasswordIdentifier;			// 092
	USHORT		HardwareResetResults;				// 093
	USHORT		FeatureSetAAM;						// 094
	USHORT		StreamMinimumRequestSize;			// 095
	USHORT		StreamTransferTimeDMA;				// 096
	USHORT		StreamAccessLatency;				// 097
	ULONG		StreamingPerformanceGranularity;	// 098-099
	ULONGLONG	TotalNumberLBA48;					// 100-103
	USHORT		StreamTransferTimePIO;				// 104
	USHORT		Reserved2;							// 105
	USHORT		PhysicalAndLogicalSectorSize;		// 106
	USHORT		InterSeekDelay;						// 107
	USHORT		WorldWideName[4];					// 108-111
	USHORT		ReservedWorldWideName[4];			// 112-115
	USHORT		ReservedTLC;						// 116
	ULONG		LogicalSectorSize;					// 117-118
	USHORT		FeatureSetSupported4;				// 119
	USHORT		FeatureSetSupportedOrEnabled4;		// 120
	USHORT		ReservedSupportedAndEnabledExp[6];	// 121-126
	USHORT		RemovableMediaSupported;// Obsolete	// 127
	USHORT		SecurityStatus;						// 128
	USHORT		VendorSpecific[31];					// 129-159
	USHORT		PowerModeCFA;						// 160
	USHORT		ReservedCFA2[7];					// 161-167
	USHORT		DeviceNominalFormFactor;			// 168
	USHORT		Reserved3[7];						// 169-175
	UCHAR		MediaSerialNumber[40];				// 176-195
	UCHAR		MediaManufacturer[20];				// 196-205
	USHORT		CommandTransportSCT;				// 206
	USHORT		ReservedCE_ATA1[2];					// 207-208
	USHORT		AlignmentLogicalToPhysical;			// 209
	ULONG		WriteReadVerifySectorCountMode3;	// 210-211
	ULONG		WriteReadVerifySectorCountMode2;	// 212-213
	USHORT		NonVolatileCacheCapabilities;		// 214
	ULONG		NonVolatileCacheSize;				// 215-216
	USHORT		NominalMediaRotationRate;			// 217
	USHORT		Reserved4;							// 218
	USHORT		NonVolatileCacheOptions;			// 219
	USHORT		WriteReadVerifyCurrentMode;			// 220
	USHORT		Reserved5;							// 221
	USHORT		TransportMajorVersion;				// 222
	USHORT		TransportMinorVersion;				// 223
	USHORT		ReservedCE_ATA2[10];				// 224-233
	USHORT		MinNumberSectorsDownloadMicrocode;	// 234
	USHORT		MaxNumberSectorsDownloadMicrocode;	// 235
	USHORT		Reserved6[19];						// 236-254
	USHORT		IntegrityWord;						// 255
} IDENTIFY_DISK_ATA, *PIDENTIFY_DISK_ATA;
#pragma pack ()
#endif