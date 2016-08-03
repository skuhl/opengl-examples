
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include <stdint.h>

/* This driver reads raw data from the BNO055

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/


Adafruit_BNO055 bno = Adafruit_BNO055();


int binary = 1;


/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  
  if(!binary)
  {
    Serial.println("Orientation Sensor Raw Data Test"); 
    Serial.println("Calibration status values: 0=uncalibrated, 3=fully calibrated");
  }

  /* Initialise the sensor */
  while(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    if(!binary)
      Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    delay(1000);
  }

  if(!binary)
  {
    /* Display the current temperature */
    int8_t temp = bno.getTemp();
    Serial.print("Current Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
    Serial.println("");
  }
  
  bno.setExtCrystalUse(true);

}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void)
{
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - rad/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2

  
  if(binary)
  {
/*    
    Serial.write(255);
    for(char i=0; i<4*4-1; i++)
    {
       Serial.write(i);
    }
  */  
  
  /*
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    double sanity = 1.0;
    Serial.write((uint8_t*) &sanity, 4);
    Serial.write((uint8_t*) &(euler.x()), sizeof(double));
    Serial.write((uint8_t*) &(euler.y()), sizeof(double));
    Serial.write((uint8_t*) &(euler.z()), sizeof(double));
  */

    imu::Quaternion quat = bno.getQuat();
    double sanityCheck = 123.456;
    Serial.write((uint8_t*) &sanityCheck, 4);
    Serial.write((uint8_t*) &(quat.x()), sizeof(double));
    Serial.write((uint8_t*) &(quat.y()), sizeof(double));
    Serial.write((uint8_t*) &(quat.z()), sizeof(double));
    Serial.write((uint8_t*) &(quat.w()), sizeof(double));
    
    uint8_t system, gyro, accel, mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);
    //Serial.write(1);
    //Serial.write(2);
    //Serial.write(3);
    //Serial.write(4);
    Serial.write(system);
    Serial.write(gyro);
    Serial.write(accel);
    Serial.write(mag);
  }    
  else 
  {
    /* Display the floating point data */
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    Serial.print("X: ");
    Serial.print(euler.x());
    Serial.print(" Y: ");
    Serial.print(euler.y());
    Serial.print(" Z: ");
    Serial.print(euler.z());
    Serial.print("\t\t");

    /*
    // Quaternion data
    imu::Quaternion quat = bno.getQuat();
    Serial.print("qW: ");
    Serial.print(quat.w(), 4);
    Serial.print(" qX: ");
    Serial.print(quat.y(), 4);
    Serial.print(" qY: ");
    Serial.print(quat.x(), 4);
    Serial.print(" qZ: ");
    Serial.print(quat.z(), 4);
    Serial.print("\t\t");
    */
  
    /* Display calibration status for each sensor. */
    uint8_t system, gyro, accel, mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);
    Serial.print("CALIBRATION: Sys=");
    Serial.print(system, DEC);
    Serial.print(" Gyro=");
    Serial.print(gyro, DEC);
    Serial.print(" Accel=");
    Serial.print(accel, DEC);
    Serial.print(" Mag=");
    Serial.println(mag, DEC);
  }
  
  Serial.flush();

  // How many samples per second?  
  delay(10); // milliseconds
}
