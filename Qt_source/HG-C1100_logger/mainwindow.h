#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QSerialPort* serial;

    QByteArray data;
    uint8_t koefSets[8];
    uint8_t dataIn[9];
    uint8_t dataOut[4];
    bool absolute = false;
    bool const_meas = false;

private slots:
    void readDataSlot();

    void on_connectButton_clicked();

    void on_refreshPortButton_clicked();

    void on_sendButton_clicked();

    void on_sendDefaultButton_clicked();

    void on_saveSetsButton_clicked();

    void on_modeButton_clicked();

    void on_constMeasButton_clicked();

    void on_getDataButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
