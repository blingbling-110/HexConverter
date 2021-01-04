#include "exportworker.h"

ExportWorker::ExportWorker(QObject *parent) : QObject(parent)
{

}

void ExportWorker::testWorker()
{
    for(unsigned int i = 0; i <= 0xffffffffffffffff; i++)
    {
        QThread::usleep(300);
        emit message(QString::number(i, 16) + " FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF");
    }
}

void ExportWorker::realWorker(
        const QString &importPath,
        const QString &startAddress,
        const QString &exportPath,
        const bool padding,
        const QString paddingValue)
{
    FileModel *file = new FileModel;

    //解析文件
    if (importPath.endsWith(".bin")) {
        bool ok;
        file->parseBin(importPath.toLocal8Bit().data(), startAddress.toUInt(&ok, 16));
    }
//    this->printFile(file);

    //导出文件
    if (exportPath.endsWith(".bin")) {
        file->generateBin(exportPath.toLocal8Bit().data(), padding, *(paddingValue.toLocal8Bit().data()));
    }
    emit message("已生成：" + exportPath);

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
            if (i++ < 0x1F) {
                output += QString("%1 ").arg(*pData++, 2, 16, QLatin1Char('0')).toUpper();
                if (len == 0) {
                    emit message(output);
                }
            }else {
                QThread::usleep(1);
                emit message(output);
                output = QString("%1 ").arg(*pData++, 2, 16, QLatin1Char('0')).toUpper();
                i = 0;
            }
        }
    }
}
