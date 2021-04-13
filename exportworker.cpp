#include "exportworker.h"

ExportWorker::ExportWorker(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<std::vector<std::vector<QString>>>("std::vector<std::vector<QString>>");//注册类型
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
        const QString endAddr)
{
//    qDebug() << exportPath;
    //取消导出
    if (exportPath.isEmpty()) {
        emit enable();
        return;
    }

    FileModel *file = new FileModel;
    bool ok;

    //解析文件
    for (size_t i = 0; i < importFiles.size(); ++i) {
        QString startAddress = importFiles[i][0];
        QString importPath = importFiles[i][1];
//        qDebug() << importPath;
        if (importPath.endsWith(".bin") || importPath.endsWith(".BIN")) {
            file->parseBin(importPath.toLocal8Bit().data(), startAddress.toUInt(&ok, 16));
        }else if (importPath.endsWith(".hex") || importPath.endsWith(".HEX")) {
            file->parseHex(importPath.toLocal8Bit().data());
        }
    }

    //对所有段进行过滤
    file->filter(startAddr.toUInt(&ok, 16), endAddr.toUInt(&ok, 16));

    //打印输入文件
    if (print) {
        this->printFile(file);
    }

    //导出文件
    if (exportPath.endsWith(".bin") || exportPath.endsWith(".BIN")) {
        file->generateBin(exportPath.toLocal8Bit().data(), padding, paddingValue.toUInt(&ok, 16));
    }else if (exportPath.endsWith(".hex") || exportPath.endsWith(".HEX")) {
        file->generateHex(exportPath.toLocal8Bit().data(), padding, paddingValue.toUInt(&ok, 16));
    }
    emit message("转换已完成！");
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
