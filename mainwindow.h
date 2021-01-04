#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qfiledialog.h"
#include <QUrl>
#include <QDesktopServices>
#include "aboutdialog.h"
#include "exportworker.h"
#include <QThread>
#include <vector>
#include <QTimer>
#include <QMessageBox>

#define CONSOLE_MAX_LINE 2020
#define CONSOLE_FLASH_TIME 10

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QThread exportThread;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_import_clicked();

    void on_checkBox_addCrc_stateChanged(int state);

    void on_checkBox_padding_stateChanged(int state);

    void on_pushButton_export_clicked();

    void on_action_support_triggered();

    void on_action_document_triggered();

    void on_action_about_triggered();

    void on_comboBox_crcType_currentIndexChanged(int index);

    void receiveMessage(const QString &str);

    void printStr();

    void enableExportButton();

    void on_pushButton_clear_clicked();

private:
    Ui::MainWindow *ui;
    AboutDialog *aboutDialog;

    void check_crc_setting(bool addCrcIsChecked);

    void select_crc_setting(int currentIndex);

    void check_padding_setting(bool paddingIsChecked);

    std::vector<QString> consoleBuffer;

    unsigned int bufferIndex;

    QTimer *consoleTimer;

signals:
    void startExport(
            const QString &importPath,
            const QString &startAddress,
            const QString &exportPath,
            const bool padding,
            const QString paddingValue);
};

std::map<std::string, unsigned int> crcArgs[];

#endif // MAINWINDOW_H
