#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);
    connect(serial, SIGNAL(readyRead()), this, SLOT(readDataSlot()));

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
            ui->portBox->addItem(serialPortInfo.portName());
        }
    ui->sensorBox->addItem("1");
    ui->sensorBox->addItem("2");
    ui->sensorBox->addItem("3");
    ui->sensorBox->addItem("4");

    if (!QFile::exists("log.csv")){
        QFile log("log.csv");
        log.open(QIODevice::Append);
        log.write("Time;D1;D2;D3;D4\n");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readDataSlot()
{
    if (serial->bytesAvailable()==9) {
        data = serial->readAll();
        serial->clear();

        for (int i = 0; i < 9; i++){
            dataIn[i] = data[i] & 0xFF;
        }

            if (dataIn[0] == 3){
                int s1 = (dataIn[1]) * 256 + dataIn[2];
                int s2 = (dataIn[3]) * 256 + dataIn[4];
                int s3 = (dataIn[5]) * 256 + dataIn[6];
                int s4 = (dataIn[7]) * 256 + dataIn[8];

                QFile file("log.csv");
                file.open(QIODevice::Append);
                QString log = QTime::currentTime().toString("hh:mm:ss") + ";" + QString::number(s1) + ";" + QString::number(s2) + ";"
                        + QString::number(s3) + ";" + QString::number(s4) + "\n";
                log.replace(".", ",");
                file.write(log.toLocal8Bit());

                if (absolute) {
                    ui->sensor1DataLCD->display(s1);
                    ui->sensor2DataLCD->display(s2);
                    ui->sensor3DataLCD->display(s3);
                    ui->sensor4DataLCD->display(s4);
                } else {
                    ui->sensor1DataLCD->display(s1);
                    ui->sensor2DataLCD->display(s2);
                    ui->sensor3DataLCD->display(s3);
                    ui->sensor4DataLCD->display(s4);
                }
            } else if (dataIn[0] == 12){
                for (int i = 0; i < 8; i++){
                    koefSets[i] = dataIn[i+1];
                }
                QString k1 = QString::number((double(dataIn[1])*100 + double(dataIn[2]))/1000, 'f', 3);
                QString k2 = QString::number((double(dataIn[3])*100 + double(dataIn[4]))/1000, 'f', 3);
                QString k3 = QString::number((double(dataIn[5])*100 + double(dataIn[6]))/1000, 'f', 3);
                QString k4 = QString::number((double(dataIn[7])*100 + double(dataIn[8]))/1000, 'f', 3);
                ui->koef1Label->setText("K1: " + k1);
                ui->koef2Label->setText("K2: " + k2);
                ui->koef3Label->setText("K3: " + k3);
                ui->koef4Label->setText("K4: " + k4);
            }
        }
}


void MainWindow::on_connectButton_clicked()
{
    serial->setPortName(ui->portBox->currentText());

    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setParity(QSerialPort::EvenParity);

    if (!serial->open(QIODevice::ReadWrite)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось подключится к порту");
            return;
        }

    serial->clear();

    if (QFile::exists("settings.txt"))
    {
        QFile sets("settings.txt");
        sets.open(QIODevice::ReadOnly);
        sets.read((char*)koefSets, 8);
        dataOut[0] = 3;
        for (int i = 0; i < 4; i++ ) {
            dataOut[1] = i+1;
            dataOut[2] = koefSets[2*i];
            dataOut[3] = koefSets[2*i+1];
            serial->write((char*)dataOut, 4);
            serial->waitForBytesWritten();
            QThread::msleep(50);
        }

    }

    serial->clear();

    dataOut[0] = 48;
    dataOut[1] = 0;
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();

}


void MainWindow::on_refreshPortButton_clicked()
{
    ui->portBox->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
            ui->portBox->addItem(serialPortInfo.portName());
        }
}


void MainWindow::on_sendButton_clicked()
{
    dataOut[0] = 3;
    dataOut[1] = ui->sensorBox->currentText().toInt();
    uint16_t temp = ui->koefSettingLineEdit->text().toDouble()*1000;

    dataOut[2] = temp/100;
    dataOut[3] = temp%100;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();

    QThread::msleep(10);

    serial->clear();

    dataOut[0] = 48;
    dataOut[1] = 0;
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();
}


void MainWindow::on_sendDefaultButton_clicked()
{
    dataOut[0] = 12;
    dataOut[1] = ui->sensorBox->currentText().toInt();
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();

    QThread::msleep(10);

    serial->clear();

    dataOut[0] = 48;
    dataOut[1] = 0;
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();
}


void MainWindow::on_saveSetsButton_clicked()
{
    QFile sets("settings.txt");
    sets.open(QIODevice::WriteOnly);
    sets.write((char*)koefSets, 8);
}


void MainWindow::on_modeButton_clicked()
{
    absolute = !absolute;
    dataOut[0] = 96;
    if (absolute) {
        dataOut[1] = 240;
        ui->modeButton->setText("Абсолютный");
    }
    else {
        dataOut[1] = 16;
        ui->modeButton->setText("Обычный");
    }
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();
}


void MainWindow::on_constMeasButton_clicked()
{
    const_meas = !const_meas;
    dataOut[0] = 204;
    if (const_meas) {
        dataOut[1] = 240;
        ui->constMeasButton->setText("Постоянно");
        ui->getDataButton->setEnabled(false);
    }
    else {
        dataOut[1] = 16;
        ui->constMeasButton->setText("Однократно");
        ui->getDataButton->setEnabled(true);
    }
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();
}


void MainWindow::on_getDataButton_clicked()
{
    dataOut[0] = 204;
    dataOut[1] = 16;
    dataOut[2] = 0;
    dataOut[3] = 0;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();
}

