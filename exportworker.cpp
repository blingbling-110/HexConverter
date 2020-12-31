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

void ExportWorker::realWorker(const QString &path, const QString &startAddress)
{
    FileModel *file = new FileModel;
    bool ok;
    file->parseBin(path.toLocal8Bit().data(), startAddress.toUInt(&ok, 16));
    Segment segment;
    for (auto iter = file->segments.begin(); iter != file->segments.end(); ++iter) {
        segment = iter->second;
        printSegment(segment);
    }
    emit enable();
}

void ExportWorker::printSegment(Segment segment)
{
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
