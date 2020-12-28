#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qfiledialog.h"
#include <QUrl>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化
    ui->comboBox_crcType->setDisabled(true);
    ui->lineEdit_crcWidth->setDisabled(true);
    ui->lineEdit_poly->setDisabled(true);
    ui->lineEdit_init->setDisabled(true);
    ui->comboBox_refin->setDisabled(true);
    ui->comboBox_refout->setDisabled(true);
    ui->lineEdit_xorout->setDisabled(true);
    ui->lineEdit_padding->setDisabled(true);

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
                "(*.hex *.s19 *.bin)");
    ui->lineEdit_import->setText(fileName);
}

void MainWindow::on_checkBox_addCrc_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->comboBox_crcType->setEnabled(true);
        ui->lineEdit_crcWidth->setEnabled(true);
        ui->lineEdit_poly->setEnabled(true);
        ui->lineEdit_init->setEnabled(true);
        ui->comboBox_refin->setEnabled(true);
        ui->comboBox_refout->setEnabled(true);
        ui->lineEdit_xorout->setEnabled(true);
    }else if (state == Qt::Unchecked) {
        ui->comboBox_crcType->setDisabled(true);
        ui->lineEdit_crcWidth->setDisabled(true);
        ui->lineEdit_poly->setDisabled(true);
        ui->lineEdit_init->setDisabled(true);
        ui->comboBox_refin->setDisabled(true);
        ui->comboBox_refout->setDisabled(true);
        ui->lineEdit_xorout->setDisabled(true);
    }
}

void MainWindow::on_checkBox_padding_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->lineEdit_padding->setEnabled(true);
    }else if (state == Qt::Unchecked) {
        ui->lineEdit_padding->setDisabled(true);
    }
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
    QDesktopServices::openUrl(QUrl::fromLocalFile("./readme.doc"));
}
