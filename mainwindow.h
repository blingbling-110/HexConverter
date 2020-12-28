#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_import_clicked();

    void on_checkBox_addCrc_stateChanged(int arg1);

    void on_checkBox_padding_stateChanged(int arg1);

    void on_pushButton_export_clicked();

    void on_action_support_triggered();

    void on_action_document_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
