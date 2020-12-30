#include "FileModel.h"

bool FileModel::append(Segment segment)
{
	return this->segments.insert(std::pair<rsize_t, Segment>(segment.getStartAddress(), segment)).second;
}

void FileModel::parseBin(const char * path, size_t startAddress)
{
    ifstream ifs(path, ios::in | ios::binary);
    Segment temp;
    char buffer[READ_BIN_EACH_TIME];
    while (!ifs.eof()) {
        ifs.read(buffer, READ_BIN_EACH_TIME);
        temp.append(reinterpret_cast<unsigned char*>(buffer), ifs.gcount());
    }
    temp.setStartAddress(startAddress);
    append(temp);
}
