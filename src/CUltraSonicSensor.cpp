#include "CUltraSonicSensor.h"
#include "Arduino.h"


int CUltraSonicSensor::S_TriggerPin = 0;
int CUltraSonicSensor::S_EchoPin = 0;

CUltraSonicSensor::CUltraSonicSensor()
{

}

CUltraSonicSensor::~CUltraSonicSensor()
{

}


int CUltraSonicSensor::getDistanceInCM()
{
  // Clear the trigPin by setting it LOW:
  digitalWrite(S_TriggerPin, LOW);
  
  delayMicroseconds(5);
 // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(S_TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(S_TriggerPin, LOW);
  
  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  long duration = pulseIn(S_EchoPin, HIGH);
  
  // Calculate the distance:
  int distance = duration * 0.034 / 2;
  return distance;
}

int CUltraSonicSensor::initUltraSonicSensorPins(int _triggerPin, int _echoPin)
{
  S_EchoPin = _echoPin;
  S_TriggerPin = _triggerPin;
  pinMode(S_TriggerPin, OUTPUT);
  pinMode(S_EchoPin, INPUT);

  
}