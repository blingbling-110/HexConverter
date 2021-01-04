#include "FileModel.h"

bool FileModel::append(Segment segment)
{
	return this->segments.insert(std::pair<rsize_t, Segment>(segment.getStartAddress(), segment)).second;
}

void FileModel::parseBin(const char * path, size_t startAddress)
{
    ifstream ifs(path, ios::in | ios::binary);
    Segment temp;
    temp.setStartAddress(startAddress);
    char buffer[READ_EACH_TIME];
    while (!ifs.eof()) {
        ifs.read(buffer, READ_EACH_TIME);
        temp.append(reinterpret_cast<unsigned char*>(buffer), ifs.gcount());
    }
    ifs.close();
    this->append(temp);
}

void FileModel::generateBin(const char* path, const bool padding, unsigned char paddingValue)
{
    Segment allSegments;
    for (auto iter = this->segments.begin(); iter != this->segments.end(); ++iter) {
        size_t startAddress = iter->first;
        Segment segment = iter->second;
        if (padding) {//需填充
            if (allSegments.getLength() == 0) {
                allSegments.setStartAddress(startAddress);
            }else {
                size_t paddingSize = segment.getStartAddress() - allSegments.getStartAddress() - allSegments.getLength();
                for (int i = 0; i < paddingSize; ++i) {
                    allSegments.append(paddingValue);//填充
                }
            }
            allSegments.append(segment.getFrontPointer(), segment.getLength());
        }else {//不需填充
            if (this->segments.size() == 1) {
                ofstream ofs(path, ios::out | ios::binary);
                ofs.write(reinterpret_cast<char*>(segment.getFrontPointer()), segment.getLength());
                ofs.close();
            }else if (this->segments.size() > 1) {
                char outputPath[256];
                sprintf(outputPath, "%zX.bin", segment.getStartAddress());
                ofstream ofs(outputPath, ios::out | ios::binary);
                ofs.write(reinterpret_cast<char*>(segment.getFrontPointer()), segment.getLength());
                ofs.close();
            }
        }
    }
    if (allSegments.getLength() != 0) {
        ofstream ofs(path, ios::out | ios::binary);
        ofs.write(reinterpret_cast<char*>(allSegments.getFrontPointer()), allSegments.getLength());
        ofs.close();
    }
}

void FileModel::parseHex(const char * path)
{
    ifstream ifs(path, ios::in);
    string line;
    while (getline(ifs, line)) {
        Segment temp;
//        temp.setStartAddress(startAddress);
//        temp.append(reinterpret_cast<unsigned char*>(buffer), ifs.gcount());
        this->append(temp);
    }
    ifs.close();
}
