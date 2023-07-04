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

    QString filename;

    uint8_t dataIn[9];
    uint8_t dataOut[4];
    uint16_t adc1, adc2, adc3, adc4;
    uint16_t targetCountOfMeasures, currentCountOfMeasures;
    uint16_t measures[65635];
    double volt1, volt2, volt3, volt4;
    double mm1, mm2, mm3, mm4;
    double diameter;

    double sens1Offset, sens2Offset, sens3Offset;

private slots:
    void readDataSlot();

    void on_connectButton_clicked();

    void on_refreshPortButton_clicked();

    void on_startCountOfMeasuresButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
