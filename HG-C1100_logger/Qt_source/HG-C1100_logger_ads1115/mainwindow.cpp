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
            currentCountOfMeasures++;

            adc1 = dataIn[1] * 256 + dataIn[2];
            adc2 = dataIn[3] * 256 + dataIn[4];
            adc3 = dataIn[5] * 256 + dataIn[6];
            adc4 = dataIn[7] * 256 + dataIn[8];

            volt1 = adc1 * 0.1875 / 1000;
            volt2 = adc2 * 0.1875 / 1000;
            volt3 = adc3 * 0.1875 / 1000;
            volt4 = adc4 * 0.1875 / 1000;

            mm1 = 65 + (5-volt1) * 14;
            mm2 = 65 + (5-volt2) * 14;
            mm3 = 65 + (5-volt3) * 14;
            mm4 = 30 + (5-volt4) * 2;

            double r1 = 140.5 - mm1;
            double r2 = 140.5 - mm2;
            double r3 = 140.5 - mm3;

            diameter = (r1+r2+r3)*1.5; //------------------------------------заменить формулу

            if (volt1 > 5.1) {
                ui->sensor1DataLCD->display("----");
                diameter = 0;
            } else ui->sensor1DataLCD->display(mm1);
            if (volt2 > 5.1) {
                ui->sensor2DataLCD->display("----");
                diameter = 0;
            } else ui->sensor2DataLCD->display(mm2);
            if (volt3 > 5.1) {
                ui->sensor3DataLCD->display("----");
                diameter = 0;
            } else ui->sensor3DataLCD->display(mm3);
            /*if (volt4 > 5.1) {
                ui->sensor4DataLCD->display("----");
            } else ui->sensor4DataLCD->display(mm4);*/

            if (diameter == 0) ui->diameterDataLCD->display("----");

            ui->countOfMeasuresProgressBar->setValue((currentCountOfMeasures*100/targetCountOfMeasures));

            QFile file(filename + ".csv");
            file.open(QIODevice::Append);
            QString log = QTime::currentTime().toString("hh:mm:ss") + ";" + QString::number(currentCountOfMeasures) + ";" +
                    QString::number(adc1) + ";" + QString::number(volt1) + ";" + QString::number(mm1) + ";" +
                    QString::number(adc2) + ";" + QString::number(volt2) + ";" + QString::number(mm2) + ";" +
                    QString::number(adc3) + ";" + QString::number(volt3) + ";" + QString::number(mm3) + ";" +
                    QString::number(adc4) + ";" + QString::number(volt4) + ";" + QString::number(mm4) + QString::number(diameter) + "\n";
            log.replace(".", ",");
            file.write(log.toLocal8Bit());
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
}


void MainWindow::on_refreshPortButton_clicked()
{
    ui->portBox->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->portBox->addItem(serialPortInfo.portName());
    }
}

void MainWindow::on_startCountOfMeasuresButton_clicked()
{
    targetCountOfMeasures = ui->countOfMesasuresLineEdit->text().toInt();
    currentCountOfMeasures = 0;
    QString text = QString::number(targetCountOfMeasures) + " measures";

    filename = ui->fileNameLineEdit->text();
    if (filename == "") filename = "log";

    QFile log(filename + ".csv");
    log.open(QIODevice::Append);
    log.write("/---;---;---;---;---;---/;" + text.toLocal8Bit() + ";/---;---;---;---;---;---;---/;\n");
    log.write("Time;Number;D1-adc;D1-volt;D1-mm;D2-adc;D2-volt;D2-mm;D3-adc;D3-volt;D3-mm;D4-adc;D4-volt;D4-mm;Diameter;\n");

    dataOut[0] = 16;
    dataOut[1] = targetCountOfMeasures >> 8;
    dataOut[2] = targetCountOfMeasures;

    serial->write((char*)dataOut, 4);
    serial->waitForBytesWritten();
}

