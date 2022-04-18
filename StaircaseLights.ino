
#include "MovingAverage.h"

class Sensor {
  public:
    Sensor(int aPin);
    int pin;
    volatile byte dataIsAvailable;
    int negs;
    volatile uint32_t riseTime;
    volatile uint32_t fallTime;
    int getPin();
    int getPinInterrupt();
};

Sensor::Sensor(int aPin) {
  this->pin = aPin;
  this->dataIsAvailable = 0;
  this->negs = 0;
  this->riseTime = 0;
  this->fallTime = 0;
}

Sensor::getPin() {
  return pin;
}
Sensor::getPinInterrupt() {
  return pin - 2;
}

Sensor sensorA(2);
Sensor sensorB(3);

MovingAverage <uint8_t, 16> filterA;
MovingAverage <uint8_t, 16> filterB;

volatile bool inProgress = false;
int startPin = 4;

void setup() {
  pinMode(LED_BUILTIN_TX, INPUT);
  pinMode(LED_BUILTIN_RX, INPUT);
//  Serial.begin(115200);
//  while (!Serial) delay(1);
  pinMode(sensorA.getPin(), INPUT_PULLUP);
  attachInterrupt(sensorA.getPinInterrupt(), risingEdgeA, RISING);
  pinMode(sensorB.getPin(), INPUT_PULLUP);
  attachInterrupt(sensorB.getPinInterrupt(), risingEdgeB, RISING);

  for (int i = 0; i < 6; i++) {
    pinMode(startPin + i, OUTPUT);
    digitalWrite(startPin + i, LOW);
  }
}

void fallingEdgeA() {
  if (inProgress) {
    sensorA.dataIsAvailable  = 0;
    sensorB.dataIsAvailable  = 0;
    return;
  }
  sensorA.fallTime = micros();
  sensorA.dataIsAvailable  = 1;
  attachInterrupt(sensorA.getPinInterrupt(), risingEdgeA, RISING);
}

void risingEdgeA() {
  if (inProgress) {
    sensorA.dataIsAvailable  = 0;
    sensorB.dataIsAvailable  = 0;
    return;
  }
  sensorA.riseTime = micros();
  attachInterrupt(sensorA.getPinInterrupt(), fallingEdgeA, FALLING);
}

void fallingEdgeB() {
  if (inProgress) {
    sensorA.dataIsAvailable  = 0;
    sensorB.dataIsAvailable  = 0;
    return;
  }
  sensorB.fallTime = micros();
  sensorB.dataIsAvailable  = 1;
  attachInterrupt(sensorB.getPinInterrupt(), risingEdgeB, RISING);
}

void risingEdgeB() {
  if (inProgress) {
    sensorA.dataIsAvailable  = 0;
    sensorB.dataIsAvailable  = 0;
    return;
  }
  sensorB.riseTime = micros();
  attachInterrupt(sensorB.getPinInterrupt(), fallingEdgeB, FALLING);
}


void loop() {
  if (sensorA.dataIsAvailable) {
    int pulseDur = sensorA.fallTime - sensorA.riseTime;

    filterA.add(pulseDur);
//    Serial.print("A ");
//    Serial.println(filterA.get());
    if (filterA.get() > 238 && filterB.get() < 160) {
      lightsA();
//      Serial.print("Value after A:");
//      Serial.println(filterA.get());
    }
    sensorA.dataIsAvailable = 0;
    //    sensorA.riseTime = 0;
    //    sensorA.fallTime = 0;
  }
  if (sensorB.dataIsAvailable) {
    int pulseDur = sensorB.fallTime - sensorB.riseTime;
    filterB.add(pulseDur);
//    Serial.print("B ");
//    Serial.println(filterB.get());
    if (filterB.get() > 160 && filterA.get() < 238) {
      lightsB();

//      Serial.print("Value after B:");
//      Serial.println(filterB.get());
    }
    sensorB.dataIsAvailable = 0;
    //    sensorB.riseTime = 0;
    //    sensorB.fallTime = 0;
  }
}


void lightsA() {
  inProgress = true;
  for (int i = 0; i < 6; i++) {
    digitalWrite(startPin + i, HIGH);
    delay(500);
  }
  delay(9000);
  for (int i = 0; i < 10; i++) {
    filterA.add(0);
    filterB.add(0);
  }
  for (int i = 0; i < 6; i++) {
    digitalWrite(startPin + i, LOW);
    delay(500);
  }
  inProgress = false;
  attachInterrupt(sensorA.getPinInterrupt(), risingEdgeA, RISING);
  attachInterrupt(sensorB.getPinInterrupt(), risingEdgeB, RISING);
}
void lightsB() {
  inProgress = true;
  for (int i = 0; i < 6; i++) {
    digitalWrite((startPin + 5) - i, HIGH);
    delay(500);
  }
  delay(9000);
  for (int i = 0; i < 10; i++) {
    filterA.add(0);
    filterB.add(0);
  }
  for (int i = 0; i < 6; i++) {
    digitalWrite((startPin + 5) - i, LOW);
    delay(500);
  }
  inProgress = false;
  attachInterrupt(sensorA.getPinInterrupt(), risingEdgeA, RISING);
  attachInterrupt(sensorB.getPinInterrupt(), risingEdgeB, RISING);
}
