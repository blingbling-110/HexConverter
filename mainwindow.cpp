#include "mainwindow.h"
#include "ui_mainwindow.h"

std::vector<std::map<QString, QString>> crcArgs;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化
    initCrc();
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
    QDesktopServices::openUrl(QUrl("mailto:523497359@qq.com&subject=HexConverter使用问题"));
}

void MainWindow::on_action_document_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile("./README.md"));
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
        ui->lineEdit_crcName->setDisabled(true);
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
        ui->lineEdit_crcName->setEnabled(true);
        ui->lineEdit_crcWidth->setEnabled(true);
        ui->lineEdit_poly->setEnabled(true);
        ui->lineEdit_init->setEnabled(true);
        ui->comboBox_refin->setEnabled(true);
        ui->comboBox_refout->setEnabled(true);
        ui->lineEdit_xorout->setEnabled(true);
    }else {
        ui->lineEdit_crcName->setText(crcArgs[currentIndex - 1]["name"]);
        ui->lineEdit_crcWidth->setText(crcArgs[currentIndex - 1]["width"]);
        ui->lineEdit_poly->setText(crcArgs[currentIndex - 1]["poly"]);
        ui->lineEdit_init->setText(crcArgs[currentIndex - 1]["init"]);
        ui->comboBox_refin->setCurrentIndex(crcArgs[currentIndex - 1]["refin"].toInt());
        ui->comboBox_refout->setCurrentIndex(crcArgs[currentIndex - 1]["refout"].toInt());
        ui->lineEdit_xorout->setText(crcArgs[currentIndex - 1]["xorout"]);
        ui->lineEdit_crcName->setDisabled(true);
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

void MainWindow::initCrc()
{
    if (!QFileInfo::exists(CRC_INIT_FILE)) {
        QMessageBox::warning(this, "警告", QString("CRC配置文件%1不存在！").arg(CRC_INIT_FILE));
        return;
    }
    QFile file(CRC_INIT_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "警告", QString("读取CRC配置文件%1失败").arg(CRC_INIT_FILE));
        return;
    }
    QJsonParseError *error = new QJsonParseError;
    QJsonDocument jdc = QJsonDocument::fromJson(file.readAll(), error);
    if (error->error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "警告", QString("解析CRC配置文件%1失败：%2").arg(CRC_INIT_FILE, error->errorString()));
        return;
    }

    QJsonObject obj = jdc.object();//获取json对象
    QStringList list = obj.keys();
    bool abort = false;
    foreach (QString key, list) {
        std::map<QString, QString> map;
        map["name"] = key;
        QJsonObject subObj = obj[key].toObject();
        if (!(subObj.contains("width")
              && subObj.contains("poly")
              && subObj.contains("init")
              && subObj.contains("refin")
              && subObj.contains("refout")
              && subObj.contains("xorout"))) {
            abort = true;
            break;
        }
        map["width"] = subObj["width"].toString();
        map["poly"] = subObj["poly"].toString();
        map["init"] = subObj["init"].toString();
        map["refin"] = subObj["refin"].toString();
        map["refout"] = subObj["refout"].toString();
        map["xorout"] = subObj["xorout"].toString();
        crcArgs.push_back(map);
        ui->comboBox_crcType->addItem(key);
    }
    if (abort) {
        crcArgs.clear();
        ui->comboBox_crcType->blockSignals(true);//屏蔽currentIndexChanged信号
        ui->comboBox_crcType->clear();
        ui->comboBox_crcType->addItem("自定义CRC算法");
        ui->comboBox_crcType->blockSignals(false);
        QMessageBox::critical(this, "错误", QString("CRC配置文件%1格式有误").arg(CRC_INIT_FILE));
    }
//    QMessageBox::information(this, "通知", QString("CRC配置文件%1加载成功").arg(CRC_INIT_FILE));
    file.close();
}

void MainWindow::on_pushButton_addCrc_clicked()
{
    QString name = ui->lineEdit_crcName->text();
    QString width = ui->lineEdit_crcWidth->text();
    QString poly = ui->lineEdit_poly->text();
    QString init = ui->lineEdit_init->text();
    QString refin = QString::number(ui->comboBox_refin->currentIndex());
    QString refout = QString::number(ui->comboBox_refout->currentIndex());
    QString xorout = ui->lineEdit_xorout->text();
    if (name == ""
            || width == ""
            || poly == ""
            || init == ""
            || xorout == "") {
        QMessageBox::critical(this, "错误", "CRC参数不完整！");
        return;
    }
    foreach (auto el, crcArgs) {
        if (el["name"] == name) {
            QMessageBox::warning(this, "警告", "CRC算法已存在！");
            return;
        }
    }

    std::map<QString, QString> map;
    map["name"] = name;
    map["width"] = width;
    map["poly"] = poly;
    map["init"] = init;
    map["refin"] = refin;
    map["refout"] = refout;
    map["xorout"] = xorout;
    crcArgs.push_back(map);
    ui->comboBox_crcType->addItem(name);
    ui->comboBox_crcType->setCurrentIndex(static_cast<int>(crcArgs.size()));

    QFile file(CRC_INIT_FILE);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "错误", QString("读取CRC配置文件%1失败").arg(CRC_INIT_FILE));
        return;
    }
    QJsonDocument jdoc;
    QJsonObject obj, subObj;
    foreach (auto el, crcArgs) {
        subObj["width"] = el["width"];
        subObj["poly"] = el["poly"];
        subObj["init"] = el["init"];
        subObj["refin"] = el["refin"];
        subObj["refout"] = el["refout"];
        subObj["xorout"] = el["xorout"];
        obj[el["name"]] = subObj;
    }
    jdoc.setObject(obj);
    file.write(jdoc.toJson(QJsonDocument::Indented));//自动添加回车符
    file.close();
}
