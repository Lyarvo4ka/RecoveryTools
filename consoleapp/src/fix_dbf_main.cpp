
#include "io/dbf.h"
#include "io/finder.h"


void fixAllDbfFiles(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.add_extension(L".dbf");
	finder.FindFiles(folder);

	for (auto filename : finder.getFiles())
	{
		IO::fixDBF(filename);
	}
}

void createManyExcelFiles()
{
	IO::path_string src_excel = LR"(d:\PaboTa\tmp\23.¬≈—Õ¿ 2021  ¿—¿.xls)";
	IO::path_string target_folder = LR"(f:\xls\)";
	IO::path_string img_file = LR"(y:\49655\49655.img)";

	IO::File img(img_file);
	img.OpenRead();
	IO::File src(src_excel);
	src.OpenRead();

	const uint32_t write_pos = 20480;
	const uint32_t mod_count = 10000;

	const uint32_t cluster_size = 4096;

	uint64_t offset = 0;

	IO::DataArray excel_data(src.Size());
	src.ReadData(excel_data);

	IO::DataArray cluster(cluster_size);

	uint64_t conter = 0;

	uint64_t folder_counter = 0;


	while (offset < img.Size())
	{
		img.ReadData(cluster);
		memcpy(excel_data.data() + write_pos, cluster.data(), cluster.size());
		if (IO::isNot00orFF(cluster.data(), cluster.size()))
		{
			if (conter % mod_count == 0)
			{
				folder_counter++;
				IO::path_string new_folder = target_folder + std::to_wstring(folder_counter);
				fs::create_directories(new_folder);
			}

			IO::path_string target_filename = target_folder + std::to_wstring(folder_counter) + L"\\" + std::to_wstring(conter++) + L".xls";
			IO::File target(target_filename);
			target.OpenCreate();
			target.WriteData(excel_data.data(), excel_data.size());
			target.Close();
		}
		offset += cluster_size;
	}







}

int wmain(int argc, wchar_t* argv[])
{
	IO::path_string foldername = LR"(f:\50134\to_fix\)";
	//createManyExcelFiles();

	fixAllDbfFiles(foldername);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
