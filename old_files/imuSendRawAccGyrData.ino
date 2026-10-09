#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>

unsigned long startTime;




#define BNO055_SAMPLERATE_DELAY_MS (100)
#define Interrupt_Pin 2 

volatile bool state = LOW;


Adafruit_BNO055 myImu = Adafruit_BNO055();


void setup() {
  Serial.begin(115200);
  startTime=millis();
  myImu.begin();
  delay(1000);
  int8_t temp = myImu.getTemp();
  myImu.setExtCrystalUse(true);
  

}

void loop() {
  unsigned long elapsedTime = millis() - startTime;

  //calibrating sensor
  uint8_t system, gyro, accel, mg = 0;
  myImu.getCalibration(&system, &gyro, &accel, &mg);
  


  //acc in ms^-2
  imu::Vector<3> acc = myImu.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  //imu::Vector<3> gyr = myImu.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Vector<3> mag = myImu.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
  imu::Quaternion quaternion = myImu.getQuat();

if (system < 3) {
  Serial.println("System not fully calibrated");
} else {
  Serial.println("Fully calibrated!");
}
  
  Serial.print(acc.x());
  Serial.print(",");
  Serial.print(acc.y());
  Serial.print(",");
  Serial.print(acc.z());
  Serial.print(",");

  

  Serial.print(quaternion.w());
  Serial.print(",");
  Serial.print(quaternion.x());
  Serial.print(",");
  Serial.print(quaternion.y());
  Serial.print(",");
  Serial.print(quaternion.z());
  Serial.print(",");
  


  Serial.print(mag.x());
  Serial.print(",");
  Serial.print(mag.y());
  Serial.print(",");
  Serial.print(mag.z());
  Serial.print(",");


  Serial.print(accel);
  Serial.print(",");
  Serial.print(gyro);
  Serial.print(",");
  Serial.print(mg);
  Serial.print(",");
  Serial.print(system);
  Serial.print(",");
  Serial.println(elapsedTime);

  delay(10);
  //Add a data interrupt pin and use to create loop that only uses fresh data when pin assignments are known.
  //Nvm replace with constant time polling.



  



}
