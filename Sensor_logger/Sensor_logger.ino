//Sensor Pin
#define SENS1_PIN 0
#define SENS2_PIN 2
#define SENS3_PIN 4
#define SENS4_PIN 6

//Voltage Divider Multiplier
#define SENS1_K 1.683
#define SENS2_K 1.688
#define SENS3_K 1.683
#define SENS4_K 1.623

//Conversion factor from voltage to mm
#define SENS_SIDE_RES 14 // 70/5 
#define SENS_BACK_RES 2  // 10/5

//Sampling value for the average arithmetic mean
#define NUM_SAMPLES 1

uint64_t timer;

uint8_t dataIn[4] = {0};
uint8_t dataOut[9] = {0};

bool absolute = false;
bool const_meas = false;
bool adc = false;

//A class containing parameters and intermediate variables of each connected sensor
class Sensor
{
  public:
    Sensor(uint8_t pin, float k, uint8_t res)
    {
      _pin = pin;
      koef = k;
      resolution = res;
      pinMode(pin, INPUT);
    }

    //A method that adds one measurement to the buffer
    void readSensor()
    {
      buf += analogRead(_pin);
    };

    //A method that calculates the average value
    //over the buffer and translates the values into mm if required
    void calculateData()
    {
      raw = buf / NUM_SAMPLES + 5;
      buf = 0;
      if (!adc) {
        volts = (raw * 3.3 / 4095.0) * koef;
        if (absolute) {
          if (volts > 5) mils = 80;
          else mm = abs(5.0 - volts) * resolution;
        } else {
          mm = abs(2.5 - volts) * resolution;
        }
      }
    }

    //The method that sets the voltage divider multiplier
    void setKoef(uint8_t HB, uint8_t LB) {
      koef = float(HB * 100 + LB) / 1000;
    }

//The method that sets the default voltage divider multiplier
    void setDefaultKoef(float def) {
      koef = def;
    }

    uint8_t _pin = 0; //Sensor pin
    uint16_t raw = 0; //ADC value after averaging
    float volts = 0;  //Voltage from sensor
    float mm = 0;     //Millimetres
    float koef = 0;   //Voltage Divider Multiplier
    uint8_t resolution = 0; //Conversion factor from voltage to mm
    uint32_t buf = 0; //Buffer for averaging
};

//Defining instances of the sensor class
Sensor sens1(SENS1_PIN, SENS1_K, SENS_SIDE_RES);
Sensor sens2(SENS2_PIN, SENS2_K, SENS_SIDE_RES);
Sensor sens3(SENS3_PIN, SENS3_K, SENS_SIDE_RES);
Sensor sens4(SENS4_PIN, SENS4_K, SENS_BACK_RES);

//A function that measures and sends sensor readings
void sendData() {
  for (int i = 0; i < NUM_SAMPLES; i++)
  {
    sens1.readSensor();
    sens2.readSensor();
    sens3.readSensor();
    sens4.readSensor();
  }
  sens1.calculateData();
  sens2.calculateData();
  sens3.calculateData();
  sens4.calculateData();

  dataOut[0] = 3;

  if (!adc) {
    uint16_t temp = sens1.mm * 100;
    dataOut[1] = temp / 100; // m1h
    dataOut[2] = temp % 100; // m1l

    temp = sens2.mm * 100;
    dataOut[3] = temp / 100; // m2h
    dataOut[4] = temp % 100; // m2l

    temp = sens3.mm * 100;
    dataOut[5] = temp / 100; // m3h
    dataOut[6] = temp % 100; // m3l

    temp = sens4.mm * 1000;
    dataOut[7] = temp / 100; // m4h
    dataOut[8] = temp % 100; // m4l

  } else {

    dataOut[1] = sens1.raw >> 8;
    dataOut[2] = sens1.raw;

    dataOut[3] = sens2.raw >> 8;
    dataOut[4] = sens2.raw;

    dataOut[5] = sens3.raw >> 8;
    dataOut[6] = sens3.raw;

    dataOut[7] = sens4.raw >> 8;
    dataOut[8] = sens4.raw;
  }
  Serial.write(dataOut, 9);
}

//A function that sends voltage divider multiplier
void sendKoefs() {
  uint16_t temp = sens1.koef * 1000;
  dataOut[0] = 12;
  dataOut[1] = temp / 100;
  dataOut[2] = temp % 100;

  temp = sens2.koef * 1000;
  dataOut[3] = temp / 100;
  dataOut[4] = temp % 100;

  temp = sens3.koef * 1000;
  dataOut[5] = temp / 100;
  dataOut[6] = temp % 100;

  temp = sens4.koef * 1000;
  dataOut[7] = temp / 100;
  dataOut[8] = temp % 100;
  Serial.write(dataOut, 9);
}

void setup()
{
  Serial.begin(115200, SERIAL_8E1);
}

void loop()
{
  if (Serial.available())
  {
    delay(10);
    Serial.read(dataIn, 4);
    if (dataIn[0] == 3)
    {
      switch (dataIn[1])
      {
        case 1:
          sens1.setKoef(dataIn[2], dataIn[3]);
          break;

        case 2:
          sens2.setKoef(dataIn[2], dataIn[3]);
          break;

        case 3:
          sens3.setKoef(dataIn[2], dataIn[3]);
          break;

        case 4:
          sens4.setKoef(dataIn[2], dataIn[3]);
          break;
      }
    }
    else if (dataIn[0] == 12)
    {
      switch (dataIn[1])
      {
        case 1:
          sens1.setDefaultKoef(SENS1_K);
          break;

        case 2:
          sens2.setDefaultKoef(SENS1_K);
          break;

        case 3:
          sens3.setDefaultKoef(SENS1_K);
          break;

        case 4:
          sens4.setDefaultKoef(SENS1_K);
          break;
      }
    }
    else if (dataIn[0] == 48)
    {
      sendKoefs();
    }
    else if (dataIn[0] == 96)
    {
      if (dataIn[1] == 16) absolute = false;
      else if (dataIn[1] == 240) absolute = true;
    }
    else if (dataIn[0] == 204)
    {
      if (dataIn[1] == 16)
      {
        const_meas = false;
        delay(10);
        sendData();
      }
      else if (dataIn[1] == 240) const_meas = true;
    }
    delay(10);
  }
  if (const_meas) {
    if (millis() - timer > 20)
    {
      sendData();
      timer = millis();
    }
  }
}
