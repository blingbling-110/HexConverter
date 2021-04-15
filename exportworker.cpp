#include "exportworker.h"

ExportWorker::ExportWorker(QObject *parent) : QObject(parent)
{
    //注册类型
    qRegisterMetaType<std::vector<std::vector<QString>>>("std::vector<std::vector<QString>>");
    qRegisterMetaType<CrcParams>("CrcParams");
}

void ExportWorker::testWorker()
{
    for(unsigned int i = 0; i <= 0xffffffff; i++)
    {
        QThread::usleep(300);
        emit message(QString::number(i, 16) + " FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF");
    }
}

void ExportWorker::realWorker(
        const std::vector<std::vector<QString>> importFiles,
        const QString &exportPath,
        const bool padding,
        const QString paddingValue,
        const bool print,
        const QString startAddr,
        const QString endAddr,
        const bool addCrc,
        const CrcParams crcParams)
{
//    qDebug() << exportPath;
    //取消导出
    if (exportPath.isEmpty()) {
        emit enable();
        return;
    }

    FileModel *file = new FileModel;
    bool ok, res = false;

    //解析文件
    for (size_t i = 0; i < importFiles.size(); ++i) {
        QString startAddress = importFiles[i][0];
        QString importPath = importFiles[i][1];
        //判断文件是否存在
        if (!QFileInfo::exists(importPath)) {
            delete(file);
            emit message("文件不存在：" + importPath);
            emit enable();
            return;
        }

        emit message("正在解析 " + importPath);
        if (importPath.endsWith(".bin") || importPath.endsWith(".BIN")) {
            file->parseBin(importPath.toLocal8Bit().data(), startAddress.toUInt(&ok, 16));
        }else if (importPath.endsWith(".hex") || importPath.endsWith(".HEX")) {
            file->parseHex(importPath.toLocal8Bit().data());
        }else if (importPath.endsWith(".s19") || importPath.endsWith(".S19")) {
            file->parseS19(importPath.toLocal8Bit().data());
        }
        emit message("解析完毕");
    }

    //将不连续的段填充为连续
    if (padding) {
        if (paddingValue == "") {
            emit message("将不连续的段填充为连续的段，填充值：0x00");
        }else {
            emit message("将不连续的段填充为连续的段，填充值：0x" + paddingValue);
        }
        file->padding(paddingValue.toUInt(&ok, 16));
    }

    //对所有段进行过滤
    file->filter(startAddr.toUInt(&ok, 16), endAddr.toUInt(&ok, 16), padding, paddingValue.toUInt(&ok, 16));

    //计算并添加各段CRC
    if (addCrc) {
        if (crcParams.width == ""
                || crcParams.poly == ""
                || crcParams.init == ""
                || crcParams.xorout == "") {
            emit message("CRC参数不完整！\n");
            emit enable();
            return;
        }
        emit message("正在计算各段CRC");
        file->addCrc(crcParams.width.toUInt(&ok, 10),
                     crcParams.poly.toULongLong(&ok, 16),
                     crcParams.init.toULongLong(&ok, 16),
                     crcParams.refin,
                     crcParams.refout,
                     crcParams.xorout.toULongLong(&ok, 16));
    }

    //打印输出段
    if (print) {
        emit message("打印各输出段的起始地址和数据：");
        this->printFile(file);
        emit message("打印完毕");
    }

    //导出文件
    emit message("正在导出文件");
    if (exportPath.endsWith(".bin") || exportPath.endsWith(".BIN")) {
        res = file->generateBin(exportPath.toLocal8Bit().data());
    }else if (exportPath.endsWith(".hex") || exportPath.endsWith(".HEX")) {
        res = file->generateHex(exportPath.toLocal8Bit().data());
    }else if (exportPath.endsWith(".s19") || exportPath.endsWith(".S19")) {
        res = file->generateS19(exportPath.toLocal8Bit().data());
    }

    if (res) {
        emit message("转换已完成！\n");
    }else {
        emit message("输出范围内无数据，转换失败！\n");
    }
    delete(file);
    emit enable();
}

void ExportWorker::printFile(FileModel* file)
{
    for (auto iter = file->segments.begin(); iter != file->segments.end(); ++iter) {
        Segment segment = iter->second;
        emit message(QString::number(segment.getStartAddress(), 16).toUpper());
        auto pData = segment.getFrontPointer();
        size_t len = segment.getLength();
        unsigned int i = 0;
        QString output;
        while(len-- > 0)
        {
            if (i++ < 0x20) {
                output += QString("%1 ").arg(*pData++, 2, 16, QLatin1Char('0')).toUpper();
                if (len == 0) {
                    emit message(output);
                }
            }else {
                QThread::usleep(1);
                emit message(output);
                output = QString("%1 ").arg(*pData++, 2, 16, QLatin1Char('0')).toUpper();
                i = 1;
            }
        }
    }
}
