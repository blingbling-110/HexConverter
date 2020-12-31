#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include <QObject>
#include <QThread>
#include "FileModel.h"

class ExportWorker : public QObject
{
    Q_OBJECT

    void printSegment(Segment segment);

public:
    explicit ExportWorker(QObject *parent = nullptr);

signals:
    void message(const QString &str);
    void enable();

public slots:
    void testWorker();
    void realWorker(const QString &path, const QString &startAddress);
};

#endif // EXPORTWORKER_H
