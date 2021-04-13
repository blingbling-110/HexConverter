#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化
    check_crc_setting(ui->checkBox_addCrc->isChecked());
    check_padding_setting(ui->checkBox_padding->isChecked());
    ui->textEdit_console->document()->setMaximumBlockCount(CONSOLE_MAX_LINE);//设置最大显示行数

    //输入框限制
    ui->lineEdit_crcWidth->setValidator(new QIntValidator(3, 64, this));
    QRegExp regHex("[a-fA-F0-9]*");
    ui->lineEdit_poly->setValidator(new QRegExpValidator(regHex, this));
    ui->lineEdit_init->setValidator(new QRegExpValidator(regHex, this));
    ui->lineEdit_xorout->setValidator(new QRegExpValidator(regHex, this));
    ui->lineEdit_startAddr->setValidator(new QRegExpValidator(regHex, this));
    ui->lineEdit_endAddr->setValidator(new QRegExpValidator(regHex, this));
    QRegExp regPadding("[a-fA-F0-9]{2}");
    ui->lineEdit_padding->setValidator(new QRegExpValidator(regPadding, this));

    //表格设置
    ReadOnlyDelegate *readOnlyDelegate = new ReadOnlyDelegate(this);
    ui->tableWidget_import->setItemDelegateForColumn(1, readOnlyDelegate);//设置列只读
    ValidateDelegate *validateDelegate = new ValidateDelegate(this);
    ui->tableWidget_import->setItemDelegateForColumn(0, validateDelegate);//设置列的验证器

    //导出线程
    ExportWorker *exportWorker = new ExportWorker;
    exportWorker->moveToThread(&exportThread);
    connect(&exportThread, &QThread::finished, exportWorker, &QObject::deleteLater);
//    connect(this, &MainWindow::startExport, exportWorker, &ExportWorker::testWorker);
    connect(this, &MainWindow::startExport, exportWorker, &ExportWorker::realWorker);
    connect(exportWorker, &ExportWorker::message, this, &MainWindow::receiveMessage);
    connect(exportWorker, &ExportWorker::enable, this, &MainWindow::enableExportButton);
    exportThread.start();

    //初始化控制台刷新定时器
    bufferIndex = 0;
    QTimer *consoleTimer = new QTimer(this);
    connect(consoleTimer, SIGNAL(timeout()), this, SLOT(printStr()));
    consoleTimer->start(CONSOLE_FLASH_TIME);
}

MainWindow::~MainWindow()
{
    delete ui;
    exportThread.quit();
    exportThread.wait();
}

std::map<std::string, unsigned int> crcArgs[] = {
    {
        {"width", 16},
        {"poly", 0x1021},
        {"init", 0xFFFF},
        {"refin", 0},
        {"refout", 0},
        {"xorout", 0x0000}
    },
    {
        {"width", 32},
        {"poly", 0xEDB88320},
        {"init", 0xFFFFFFFF},
        {"refin", 0},
        {"refout", 0},
        {"xorout", 0xFFFFFFFF}
    }
};

void MainWindow::on_pushButton_import_clicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle("导入文件");
    fileDialog.setNameFilter("All(*.hex *.s19 *.bin);;hex(*.hex);;s19(*.s19);;bin(*.bin)");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    if (fileDialog.exec() == QDialog::Accepted)//弹出对话框
    {
        QStringList importPaths = fileDialog.selectedFiles();
        foreach (QString importPath, importPaths) {
            int rowNums = ui->tableWidget_import->rowCount();
            ui->tableWidget_import->insertRow(rowNums);
            QTableWidgetItem *cellItem = new QTableWidgetItem(importPath);
            ui->tableWidget_import->setItem(rowNums, 1, cellItem);
        }
    }
}

void MainWindow::on_checkBox_addCrc_stateChanged(int state)
{
    check_crc_setting(state == Qt::Checked);
}

void MainWindow::on_checkBox_padding_stateChanged(int state)
{
    check_padding_setting(state == Qt::Checked);
}

void MainWindow::on_pushButton_export_clicked()
{
    int rowNum = ui->tableWidget_import->rowCount();
    if (rowNum < 1) {
        QMessageBox::critical(this, "错误", QString("请先导入文件"));
        return;
    }
    std::vector<std::vector<QString>> importFiles;
    for (int i = 0; i < rowNum; ++i) {
        QTableWidgetItem* startAddressItem = ui->tableWidget_import->item(i, 0);
        QTableWidgetItem* importPathItem = ui->tableWidget_import->item(i, 1);
        QString startAddress = "";
        if (startAddressItem != NULL) {
            startAddress = startAddressItem->text().trimmed();
        }
        QString importPath = importPathItem->text().trimmed();
        if (importPath == ""
                || (!importPath.endsWith(".bin")
                    && !importPath.endsWith(".hex")
                    && !importPath.endsWith(".s19")
                    && !importPath.endsWith(".BIN")
                    && !importPath.endsWith(".HEX")
                    && !importPath.endsWith(".S19"))) {
            QMessageBox::critical(this, "错误", QString("在第%1行导入了非法文件路径").arg(i + 1));
            return;
        }
        else if (importPath.endsWith(".bin") && startAddress == "") {
            QMessageBox::critical(this, "错误", QString("在第%1行导入了bin文件，请输入起始地址").arg(i + 1));
            return;
        }
        importFiles.push_back({startAddress, importPath});
    }

    const bool padding = ui->checkBox_padding->isChecked();
    QString paddingValue = ui->lineEdit_padding->text();
    const bool print = ui->checkBox_print->isChecked();
    QString startAddr = ui->lineEdit_startAddr->text();
    QString endAddr = ui->lineEdit_endAddr->text();

    CrcParams crcParams;
    crcParams.width = ui->lineEdit_crcWidth->text();
    crcParams.poly = ui->lineEdit_poly->text();
    crcParams.init = ui->lineEdit_init->text();
    crcParams.refin = ui->comboBox_refin->currentIndex() == 0? false: true;
    crcParams.refout = ui->comboBox_refout->currentIndex() == 0? false: true;
    crcParams.xorout = ui->lineEdit_xorout->text();

    ui->pushButton_export->setDisabled(true);
    QString exportPath = QFileDialog::getSaveFileName(
                this,
                "导出文件",
                "output",
                "hex(*.hex);;s19(*.s19);;bin(*.bin)");
    emit startExport(
                importFiles,
                exportPath,
                padding,
                paddingValue,
                print,
                startAddr,
                endAddr,
                ui->checkBox_addCrc->isChecked(),
                crcParams);
}

void MainWindow::on_action_support_triggered()
{
    QDesktopServices::openUrl(QUrl("mailto:qinzijun@dias.com.cn&subject=HexConverter使用问题"));
}

void MainWindow::on_action_document_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile("./readme.md"));
}

void MainWindow::on_action_about_triggered()
{
    aboutDialog = new AboutDialog(this);
    aboutDialog->exec();
}

void MainWindow::on_comboBox_crcType_currentIndexChanged(int index)
{
    select_crc_setting(index);
}

void MainWindow::check_crc_setting(bool addCrcIsChecked)
{
    if (addCrcIsChecked) {
        ui->comboBox_crcType->setEnabled(true);
        select_crc_setting(ui->comboBox_crcType->currentIndex());
    }else {
        ui->comboBox_crcType->setDisabled(true);
        ui->lineEdit_crcWidth->setDisabled(true);
        ui->lineEdit_poly->setDisabled(true);
        ui->lineEdit_init->setDisabled(true);
        ui->comboBox_refin->setDisabled(true);
        ui->comboBox_refout->setDisabled(true);
        ui->lineEdit_xorout->setDisabled(true);
    }
}

void MainWindow::select_crc_setting(int currentIndex)
{
    if (currentIndex == 0) {
        ui->lineEdit_crcWidth->setEnabled(true);
        ui->lineEdit_poly->setEnabled(true);
        ui->lineEdit_init->setEnabled(true);
        ui->comboBox_refin->setEnabled(true);
        ui->comboBox_refout->setEnabled(true);
        ui->lineEdit_xorout->setEnabled(true);
    }else {
        ui->lineEdit_crcWidth->setText(QString::number(crcArgs[currentIndex - 1]["width"]));
        ui->lineEdit_poly->setText(QString::number(crcArgs[currentIndex - 1]["poly"], 16));
        ui->lineEdit_init->setText(QString::number(crcArgs[currentIndex - 1]["init"], 16));
        ui->comboBox_refin->setCurrentIndex(crcArgs[currentIndex - 1]["refin"]);
        ui->comboBox_refout->setCurrentIndex(crcArgs[currentIndex - 1]["refout"]);
        ui->lineEdit_xorout->setText(QString::number(crcArgs[currentIndex - 1]["xorout"], 16));
        ui->lineEdit_crcWidth->setDisabled(true);
        ui->lineEdit_poly->setDisabled(true);
        ui->lineEdit_init->setDisabled(true);
        ui->comboBox_refin->setDisabled(true);
        ui->comboBox_refout->setDisabled(true);
        ui->lineEdit_xorout->setDisabled(true);
    }
}

void MainWindow::check_padding_setting(bool paddingIsChecked)
{
    if (paddingIsChecked) {
        ui->lineEdit_padding->setEnabled(true);
    }else {
        ui->lineEdit_padding->setDisabled(true);
    }
}

void MainWindow::receiveMessage(const QString &str)
{
    consoleBuffer.push_back(str);
}

void MainWindow::printStr()
{
    if (bufferIndex != consoleBuffer.size()) {
        QString str;
        for (; bufferIndex < consoleBuffer.size(); ++bufferIndex) {
            if (bufferIndex != consoleBuffer.size() - 1) {
                str += consoleBuffer[bufferIndex] + "\n";
            }else {
                str += consoleBuffer[bufferIndex];
            }
        }
        ui->textEdit_console->append(str);

        //清空缓冲
        if (consoleBuffer.size() > CONSOLE_MAX_LINE) {
            consoleBuffer.clear();
            bufferIndex = 0;
        }
    }
}

void MainWindow::enableExportButton()
{
    ui->pushButton_export->setEnabled(true);
}

void MainWindow::on_pushButton_clearConsole_clicked()
{
    ui->textEdit_console->clear();
}

void MainWindow::on_pushButton_openFolder_clicked()
{
    QTableWidgetItem *curItem = ui->tableWidget_import->currentItem();
    if (curItem != NULL) {
        QString importPath = curItem->text().trimmed();
        QProcess::startDetached("explorer /select," + importPath.replace("/", "\\"));
    }
}

void MainWindow::on_pushButton_delete_clicked()
{
    auto selItems = ui->tableWidget_import->selectedItems();
    for(int i = 0; i < selItems.count(); i++)
    {
        int curRow = ui->tableWidget_import->row(selItems.at(i));
        ui->tableWidget_import->removeRow(curRow);
    }
}

void MainWindow::on_pushButton_clearList_clicked()
{
    ui->tableWidget_import->setRowCount(0);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto urlList = event->mimeData()->urls();
    for (int i = 0; i < urlList.length(); ++i) {
        QString importPath = urlList[i].toString().replace("file:///", "");
        if (importPath == ""
                || (!importPath.endsWith(".bin")
                    && !importPath.endsWith(".hex")
                    && !importPath.endsWith(".s19")
                    && !importPath.endsWith(".BIN")
                    && !importPath.endsWith(".HEX")
                    && !importPath.endsWith(".S19"))) {
            continue;
        }
        int rowNums = ui->tableWidget_import->rowCount();
        ui->tableWidget_import->insertRow(rowNums);
        QTableWidgetItem *cellItem = new QTableWidgetItem(importPath);
        ui->tableWidget_import->setItem(rowNums, 1, cellItem);
    }
}
