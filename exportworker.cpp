#include "exportworker.h"
#include "FileModel.h"

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
    file->parseBin(path.toLocal8Bit().data(), startAddress.toInt(&ok, 16));
    Segment segment;
    for (auto iter = file->segments.begin(); iter != file->segments.end(); ++iter) {
        segment = iter->second;
        emit message(QString::number(segment.getStartAddress(), 16).toUpper());
        auto pData = segment.getFrontPointer();
        size_t len = segment.getLength();
        while(len-- > 0)
        {
            emit message(QString::number(*pData++, 16).toUpper());
        }
    }
    emit enable();
}
