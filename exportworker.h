#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "FileModel.h"

class ExportWorker : public QObject
{
    Q_OBJECT

    void printFile(FileModel* file);

public:
    explicit ExportWorker(QObject *parent = nullptr);

signals:
    void message(const QString &str);
    void enable();

public slots:
    void testWorker();
    void realWorker(
            const QString &importPath,
            const QString &startAddress,
            const QString &exportPath,
            const bool padding,
            const QString paddingValue,
            const bool print);
};

#endif // EXPORTWORKER_H
