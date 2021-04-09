#include "fileoffsetwriter.h"
#include "io/file.h"


int main(int argc, TCHAR **argv)
{
    IO::path_string targetFilename = LR"(g:\!!!TARGET\TARGET.dd)";
    auto targetFile = IO::makeFilePtr(targetFilename);

	FileOffsetWriter foWriter(targetFile);

    IO::path_string source_folder = LR"(g:\img\)";
    foWriter.writeImageFiles(source_folder);

    return 0;
}