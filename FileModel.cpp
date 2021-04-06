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
                string folderPath = path;
                folderPath = folderPath.substr(0, folderPath.rfind('/'));
                char outputPath[256];
                sprintf_s(outputPath, "%s/%zX.bin", folderPath.c_str(), segment.getStartAddress());
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
    string line, lineAddr, lineData, extLinAddr;
    int lineDataLen, lineType, lineCheckSum, i;
    size_t absAddr, lastAbsAddr;
    Segment temp;
    lastAbsAddr = 0;
    while (getline(ifs, line)) {
//        printf((line + "\n").c_str());
        lineDataLen = std::strtoul(line.substr(1, 2).c_str(), NULL, 16);
        lineAddr = line.substr(3, 4);
        lineType = std::strtoul(line.substr(7, 2).c_str(), NULL, 16);
        lineData = line.substr(9, lineDataLen * 2);
        lineCheckSum = std::strtoul(line.substr(9 + lineDataLen * 2, 2).c_str(), NULL, 16);
//        printf("%d %s %d %s %d\n",
//               lineDataLen, lineAddr.c_str(), lineType, lineData.c_str(), lineCheckSum);

        switch (lineType) {
        case 0://数据记录
            absAddr = std::strtoul((extLinAddr + lineAddr).c_str(), NULL, 16);
            if (absAddr != lastAbsAddr) {
//                printf("absAddr:%u lastAbsAddr:%u\n", absAddr, lastAbsAddr);
                if (lastAbsAddr != 0) {
                    this->append(temp);
                }
                temp.clear();
                temp.setStartAddress(absAddr);
            }
            i = 0;
            while(i < lineDataLen)
            {
                temp.append(static_cast<unsigned char>(std::strtoul(lineData.substr(i * 2, 2).c_str(), NULL, 16)));
                i++;
            }
            lastAbsAddr = absAddr + lineDataLen;
            break;
        case 1://文件结束记录
            this->append(temp);
            temp.clear();
            break;
        case 4://扩展线性地址记录
            extLinAddr = lineData;
            break;
        default:
            break;
        }
    }
    ifs.close();
}

void FileModel::generateHex(const char * path, const bool padding, unsigned char paddingValue)
{
//    for (auto iter = this->segments.begin(); iter != this->segments.end(); ++iter) {
}
