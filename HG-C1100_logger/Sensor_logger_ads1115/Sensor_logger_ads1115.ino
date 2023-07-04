#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

uint64_t timer;
uint8_t dataIn[4] = {0};
uint8_t dataOut[9] = {0};

uint16_t targetCountOfMeasures, currentCountOfMeasures;

bool const_meas = false;

void sendData() {
  currentCountOfMeasures++;
  int16_t adc0, adc1, adc2, adc3;
  adc0 = ads.readADC_SingleEnded(0);/*
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);*/

  dataOut[0] = 3;

  dataOut[1] = adc0 >> 8;
  dataOut[2] = adc0;
  /*
    dataOut[3] = adc1 >> 8;
    dataOut[4] = adc1;

    dataOut[5] = adc2 >> 8;
    dataOut[6] = adc2;

    dataOut[7] = adc3 >> 8;
    dataOut[8] = adc3;
  */
  dataOut[3] = 0;
  dataOut[4] = 0;

  dataOut[5] = 0;
  dataOut[6] = 0;

  dataOut[7] = 0;
  dataOut[8] = 0;

  Serial.write(dataOut, 9);
}

void setup(void)
{
  Serial.begin(115200, SERIAL_8E1);
  ads.setGain(GAIN_TWOTHIRDS); //0.1875mV
  ads.setDataRate(RATE_ADS1115_128SPS);//8, 16, 32, 64, 128, 250, 475, 860
  ads.begin();
  Wire.setClock(400000);
}

void loop(void)
{
  if (Serial.available())
  {
    delay(10);
    Serial.readBytes(dataIn, 4);
    if (dataIn[0] == 16)
    {
      targetCountOfMeasures = dataIn[1] * 256 + dataIn[2];
      currentCountOfMeasures = 0;
    }
  }
  if ((currentCountOfMeasures < targetCountOfMeasures) && (millis() - timer > 20)) {
    sendData();
    timer = millis();
  }
}
