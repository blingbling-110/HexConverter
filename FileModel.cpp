#include "FileModel.h"

void FileModel::append(Segment segment)
{
    size_t startAddr = segment.getStartAddress();
    size_t endAddr = startAddr + segment.getLength();

    //替换掉重复的段
    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t oriStartAddr = iter->first;
        Segment oriSegment = iter->second;
        size_t oriEndAddr = oriStartAddr + oriSegment.getLength();

        if (oriStartAddr <= startAddr && oriEndAddr >= endAddr) {
            //整个新段替换掉旧段
            oriSegment.replace(startAddr, segment.getLength(), segment.getFrontPointer());
            segments[oriStartAddr] = oriSegment;
            return;
        }else if (oriStartAddr > startAddr && oriStartAddr < endAddr && oriEndAddr >= endAddr) {
            //新段的靠下部分替换掉旧段，靠上部分继续新增
            oriSegment.replace(oriStartAddr, endAddr - oriStartAddr,
                               segment.getFrontPointer() + (oriStartAddr - startAddr));
            segments[oriStartAddr] = oriSegment;
            segment.remove(oriStartAddr, endAddr - oriStartAddr);
            this->append(segment);
            return;
        }else if (oriEndAddr > startAddr && oriEndAddr < endAddr && oriStartAddr <= startAddr) {
            //新段的靠上部分替换掉旧段，靠下部分继续新增
            oriSegment.replace(startAddr, oriEndAddr - startAddr, segment.getFrontPointer());
            segments[oriStartAddr] = oriSegment;
            segment.remove(startAddr, oriEndAddr - startAddr);
            segment.setStartAddress(oriEndAddr);
            this->append(segment);
            return;
        }else if (oriStartAddr > startAddr && oriEndAddr < endAddr) {
            //新段的中间部分替换掉旧段，上下部分继续新增
            oriSegment.replace(oriStartAddr, oriSegment.getLength(),
                               segment.getFrontPointer() + (oriStartAddr - startAddr));
            segments[oriStartAddr] = oriSegment;
            Segment segmentEnd;
            segmentEnd.setStartAddress(oriEndAddr);
            segmentEnd.append(segment.getFrontPointer() + (oriEndAddr - startAddr), endAddr - oriEndAddr);
            this->append(segmentEnd);
            segment.remove(oriStartAddr, endAddr - oriStartAddr);
            this->append(segment);
            return;
        }
    }

    //合并连续段
    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t oriStartAddr = iter->first;
        Segment oriSegment = iter->second;
        size_t oriEndAddr = oriStartAddr + oriSegment.getLength();

        if (oriEndAddr == startAddr) {//旧段在新段前且连续
            oriSegment.append(segment.getFrontPointer(), segment.getLength());
            segments[oriStartAddr] = oriSegment;
            if (++iter == segments.end()) {
                return;
            }
            size_t nextStartAddr = iter->first;
            Segment nextSegment = iter->second;
            if (nextStartAddr == endAddr) {//下一段仍然连续
                oriSegment.append(nextSegment.getFrontPointer(), nextSegment.getLength());
                segments[oriStartAddr] = oriSegment;
                segments.erase(iter);
            }
            return;
        }else if (oriStartAddr == endAddr) {//新段在旧段前且连续
            segment.append(oriSegment.getFrontPointer(), oriSegment.getLength());
            segments.erase(iter);
            break;
        }
    }

    segments.insert(std::pair<rsize_t, Segment>(segment.getStartAddress(), segment));
}

void FileModel::parseBin(const char * path, size_t startAddr)
{
    ifstream ifs(path, ios::in | ios::binary);
    Segment temp;
    temp.setStartAddress(startAddr);
    char buffer[READ_EACH_TIME];
    while (!ifs.eof()) {
        ifs.read(buffer, READ_EACH_TIME);
        temp.append(reinterpret_cast<unsigned char*>(buffer), ifs.gcount());
    }
    ifs.close();
    this->append(temp);
}

bool FileModel::generateBin(const char* path)
{
    if (segments.empty()) {//生成必要性检测
        return false;
    }

    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t startAddr = iter->first;
        Segment segment = iter->second;

        if (segments.size() == 1) {//就一段则按原文件名输出
            ofstream ofs(path, ios::out | ios::binary);
            ofs.write(reinterpret_cast<char*>(segment.getFrontPointer()), segment.getLength());
            ofs.close();
        }else if (segments.size() > 1) {//多段则按每段起始地址为文件名输出
            string folderPath = path;//字符数组转字符串
            folderPath = folderPath.substr(0, folderPath.rfind('/'));
            char outputPath[256];
            sprintf_s(outputPath, "%s/%zX.bin", folderPath.c_str(), startAddr);
            ofstream ofs(outputPath, ios::out | ios::binary);
            ofs.write(reinterpret_cast<char*>(segment.getFrontPointer()), segment.getLength());
            ofs.close();
        }
    }

    return true;
}

void FileModel::parseHex(const char * path)
{
    ifstream ifs(path, ios::in);
    string line, lineAddr, lineData, extLinAddr;
    int lineDataLen, lineType, i;
    size_t absAddr, lastAbsAddr;
    Segment temp;
    lastAbsAddr = 0;
    while (getline(ifs, line)) {
//        printf((line + "\n").c_str());
        lineDataLen = std::strtoul(line.substr(1, 2).c_str(), NULL, 16);
        lineAddr = line.substr(3, 4);
        lineType = std::strtoul(line.substr(7, 2).c_str(), NULL, 16);
        lineData = line.substr(9, lineDataLen * 2);
//        int lineCheckSum = std::strtoul(line.substr(9 + lineDataLen * 2, 2).c_str(), NULL, 16);
//        printf("%d %s %d %s %d\n",
//               lineDataLen, lineAddr.c_str(), lineType, lineData.c_str(), lineCheckSum);

        switch (lineType) {
        case 0://数据记录
            absAddr = std::strtoul((extLinAddr + lineAddr).c_str(), NULL, 16);
            if (absAddr != lastAbsAddr) {
//                printf("absAddr:%llX lastAbsAddr:%llX\n", absAddr, lastAbsAddr);
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

bool FileModel::generateHex(const char * path)
{
    if (segments.empty()) {//生成必要性检测
        return false;
    }

    ofstream ofs(path, ios::out);
    int lineDataLen = 0x20;

    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t startAddr = iter->first;
        Segment segment = iter->second;
        size_t segLen = segment.getLength();
        auto pData = segment.getFrontPointer();
        int lineDataWrited = 0, checksum = 0, length = 0;
        bool insertELA = true;
        std::stringstream strStream;

        for(size_t i = 0; i < segLen; i++)
        {
            size_t dataAddr = startAddr + i;
            strStream << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << dataAddr;
            string strDataAddr = strStream.str();
            strStream.str("");
            int data = static_cast<int>(*pData++);
            strStream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << data;
            string strData = strStream.str();
            strStream.str("");
            size_t remainLen = segLen - i;

            if(insertELA) {  // 插入扩展线性地址记录
                ofs << ":02000004" << strDataAddr.substr(0, 4);
                checksum += 0x02 + 0x04 + ((dataAddr & 0xFF000000) >> 24) + ((dataAddr & 0xFF0000) >> 16);
                ofs << std::setfill('0') << std::setw(2) << std::uppercase << std::hex
                    << (-checksum & 0xFF) << '\n';
                insertELA = false;
                checksum = 0;
            }

            if(lineDataWrited == 0) {
                ofs << ":";
                if(remainLen >= static_cast<size_t>(lineDataLen)) {
                    length = lineDataLen;
                }else {
                    length = static_cast<int>(remainLen);
                }
                ofs << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << length;
                checksum += length;
                ofs << strDataAddr.substr(4, 4);
                checksum += ((dataAddr >> 8) & 0xFF) + dataAddr & 0xFF;
                ofs << "00";
            }

            ofs << strData;
            checksum += data;
            lineDataWrited++;
            if(strDataAddr.substr(4, 4).compare("FFFF") == 0) {
                insertELA = true;
            }

            if(lineDataWrited >= length) {
                ofs << std::setfill('0') << std::setw(2) << std::uppercase << std::hex
                    << (-checksum & 0xFF) << '\n';
                lineDataWrited = 0;
                checksum = 0;
            }
        }
    }
    ofs << ":00000001FF\n";
    ofs.close();

    return true;
}

void FileModel::parseS19(const char * path)
{
    ifstream ifs(path, ios::in);
    string line, lineData;
    int lineAddrLen, lineDataLen, lineType, i;
    size_t lineAddr, lastAbsAddr;
    Segment temp;
    lastAbsAddr = 0;
    while (getline(ifs, line)) {
//        printf((line + "\n").c_str());
        lineType = std::strtoul(line.substr(1, 1).c_str(), NULL, 16);
        switch (lineType) {
        case 1://S1记录
        case 2://S2记录
        case 3://S3记录
            lineAddrLen = lineType + 1;
            break;
        default:
            continue;
        }
        lineDataLen = std::strtoul(line.substr(2, 2).c_str(), NULL, 16) - lineAddrLen - 1;
        lineAddr = std::strtoul(line.substr(4, lineAddrLen * 2).c_str(), NULL, 16);
        lineData = line.substr(4 + lineAddrLen * 2, lineDataLen * 2);
//        int lineCheckSum = std::strtoul(line.substr(4 + (lineAddrLen + lineDataLen) * 2, 2).c_str(), NULL, 16);
//        printf("%d %d %s %s %d\n",
//               lineType, lineDataLen, lineAddr.c_str(), lineData.c_str(), lineCheckSum);

        if (lineAddr != lastAbsAddr) {
//            printf("lineAddr:%llX lastAbsAddr:%llX\n", lineAddr, lastAbsAddr);
            if (lastAbsAddr != 0) {
                this->append(temp);
            }
            temp.clear();
            temp.setStartAddress(lineAddr);
        }
        i = 0;
        while(i < lineDataLen)
        {
            temp.append(static_cast<unsigned char>(std::strtoul(lineData.substr(i * 2, 2).c_str(), NULL, 16)));
            i++;
        }
        lastAbsAddr = lineAddr + lineDataLen;
    }
    if (temp.getLength() != 0) {
        this->append(temp);
        temp.clear();
    }
    ifs.close();
}

bool FileModel::generateS19(const char * path)
{
    if (segments.empty()) {//生成必要性检测
        return false;
    }

    ofstream ofs(path, ios::out);
    int lineDataLen = 20;
    ofs << "S01E00006F75747075742E73313931304D6164652062792051696E5A696A756EFA\n";

    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t startAddr = iter->first;
        Segment segment = iter->second;
        size_t segLen = segment.getLength();
        auto pData = segment.getFrontPointer();
        int lineDataWrited = 0, checksum = 0, dataLength = 0, lineLength;
        std::stringstream strStream;

        for(size_t i = 0; i < segLen; i++)
        {
            size_t dataAddr = startAddr + i;
            strStream << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << dataAddr;
            string strDataAddr = strStream.str();
            strStream.str("");
            int data = static_cast<int>(*pData++);
            strStream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << data;
            string strData = strStream.str();
            strStream.str("");
            size_t remainLen = segLen - i;

            if(lineDataWrited == 0) {
                ofs << "S3";
                if(remainLen >= static_cast<size_t>(lineDataLen)) {
                    dataLength = lineDataLen;
                }else {
                    dataLength = static_cast<int>(remainLen);
                }
                lineLength = dataLength + 4 + 1;//数据长度+地址长度+校验和占位
                ofs << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << lineLength;
                checksum += lineLength;
                ofs << strDataAddr;
                checksum += ((dataAddr >> 24) & 0xFF) + ((dataAddr >> 16) & 0xFF) + ((dataAddr >> 8) & 0xFF) + dataAddr & 0xFF;
            }

            ofs << strData;
            checksum += data;
            lineDataWrited++;

            if(lineDataWrited >= dataLength) {
                ofs << std::setfill('0') << std::setw(2) << std::uppercase << std::hex
                    << (0xFF - (checksum & 0xFF)) << '\n';
                lineDataWrited = 0;
                checksum = 0;
            }
        }
    }
    ofs.close();

    return true;
}

void FileModel::padding(unsigned char paddingValue)
{
    if (segments.size() < 2) {//填充必要性检测
        return;
    }

    Segment allSegments;
    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t startAddr = iter->first;
        Segment segment = iter->second;

        if (allSegments.getLength() == 0) {
            allSegments.setStartAddress(startAddr);
        }else {
            size_t paddingSize = startAddr - allSegments.getStartAddress() - allSegments.getLength();
            for (size_t i = 0; i < paddingSize; ++i) {
                allSegments.append(paddingValue);//填充
            }
        }
        allSegments.append(segment.getFrontPointer(), segment.getLength());
    }
    segments.clear();
    this->append(allSegments);
}

void FileModel::filter(size_t startAddr, size_t endAddr, bool padding, unsigned char paddingValue)
{
    //有效性检查
    if (startAddr > endAddr || (startAddr == 0 && endAddr == 0)) {
        return;
    }

    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t sgmStartAddr = iter->first;
        Segment segment = iter->second;
        size_t sgmEndAddr = sgmStartAddr + segment.getLength() - 1;

        if (startAddr > sgmEndAddr || endAddr < sgmStartAddr) {//段完全不在输出范围之内
            segments.erase(iter);
        }else if (startAddr <= sgmStartAddr && endAddr >= sgmStartAddr && endAddr < sgmEndAddr) {
            //段的靠上部分在输出范围之内
            segment.remove(endAddr + 1, sgmEndAddr - endAddr);
            segments[sgmStartAddr] = segment;
        }else if (startAddr > sgmStartAddr && startAddr <= sgmEndAddr && endAddr >= sgmEndAddr) {
            //段的靠下部分在输出范围之内
            segment.remove(sgmStartAddr, startAddr - sgmStartAddr);
            segment.setStartAddress(startAddr);
            segments[startAddr] = segment;
            segments.erase(iter);
        }else if (startAddr > sgmStartAddr && endAddr < sgmEndAddr) {
            //段的中间部分在输出范围之内
            segment.remove(endAddr + 1, sgmEndAddr - endAddr);
            segment.remove(sgmStartAddr, startAddr - sgmStartAddr);
            segment.setStartAddress(startAddr);
            segments[startAddr] = segment;
            segments.erase(iter);
        }
    }

    if (!padding || segments.empty()) {//检查是否填充以及段是否为空
        return;
    }
    this->padding(paddingValue);//填充
    size_t sgmStartAddr = segments.begin()->first;
    Segment segment = segments.begin()->second;
    size_t sgmEndAddr = sgmStartAddr + segment.getLength() - 1;
    if (sgmStartAddr > startAddr) {
        //补全段头
        Segment paddedSegment;
        paddedSegment.setStartAddress(startAddr);
        for (size_t i = 0; i < sgmStartAddr - startAddr; i++) {
            paddedSegment.append(paddingValue);
        }
        paddedSegment.append(segment.getFrontPointer(), segment.getLength());
        segments.clear();
        segments[startAddr] = paddedSegment;
    }
    if (sgmEndAddr < endAddr) {
        //补全段尾
        segment = segments[startAddr];
        for (size_t i = 0; i < endAddr - sgmEndAddr; i++) {
            segment.append(paddingValue);
        }
        segments[startAddr] = segment;
    }
}

void FileModel::addCrc(
        rsize_t width,
        unsigned long long poly,
        unsigned long long init,
        bool refin,
        bool refout,
        unsigned long long xorout)
{
    for (auto iter = segments.begin(); iter != segments.end(); ++iter) {
        size_t startAddr = iter->first;
        Segment segment = iter->second;

        segment.calculateSegmentCrc(width, poly, init, refin, refout, xorout);
        auto crc = segment.getSegmentCrc();
        int crcLen = ceil(width / 8.0);
        while (crcLen > 0) {
            segment.append((crc >> (crcLen-- - 1) * 8) & 0xFF);
        }
        segments[startAddr] = segment;
    }
}
