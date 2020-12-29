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

    //输入框限制
    ui->lineEdit_crcWidth->setValidator(new QIntValidator(3, 64, this));
    QRegExp regHex("[a-fA-F0-9]*");
    ui->lineEdit_poly->setValidator(new QRegExpValidator(regHex, this));
    ui->lineEdit_init->setValidator(new QRegExpValidator(regHex, this));
    ui->lineEdit_xorout->setValidator(new QRegExpValidator(regHex, this));
    QRegExp regPadding("[a-fA-F0-9]{2}");
    ui->lineEdit_padding->setValidator(new QRegExpValidator(regPadding, this));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_import_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                "导入文件",
                "",
                "支持的文件\(*.hex *.s19 *.bin)");
    ui->lineEdit_import->setText(fileName);
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

std::map<std::string, unsigned int> crcArgs[] = {
    {
        {"width", 16},
        {"poly", 0x1021},
        {"init", 0xffff},
        {"refin", 0},
        {"refout", 0},
        {"xorout", 0x0000}
    },
    {
        {"width", 32},
        {"poly", 0xedb88320},
        {"init", 0xffffffff},
        {"refin", 0},
        {"refout", 0},
        {"xorout", 0xffffffff}
    }
};
