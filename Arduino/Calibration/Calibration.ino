/*
  Modified from:
  MLX90393 Magnetometer Example Code
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 6th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Read the mag fields on three XYZ axis
  Hardware Connections (Breakoutboard to ESP32):
  3.3V = 3.3V
  GND = GND
  SDA = D21
  SCL = D22 
  Serial.print it out at 9600 baud to serial monitor.
*/
#include <Wire.h>
#include <MLX90393.h> //From https://github.com/tedyapo/arduino-MLX90393 by Theodore Yapo
#include "BluetoothSerial.h" 

MLX90393 mlx;
MLX90393::txyz data; //Create a structure, called data, of four floats (t, x, y, and z)
BluetoothSerial ESP_BT; 


void setup()
{ 
  Serial.begin(9600);
  Wire.begin();
  mlx.begin(); //Assumes I2C jumpers are GND. No DRDY pin used.
  mlx.reset();
  mlx.setGainSel(7);
  mlx.setResolution(0, 0, 0); //x, y, z
  mlx.setOverSampling(3);
  mlx.setDigitalFiltering(7);

}
void loop()
{
  mlx.readData(data); //Read the values from the sensor
  ESP_BT.begin("Magnetometer-Calibration_X"); //Bluetooth

  float Mxyz[3];
  float M_abs;
  float T;

  Mxyz[0] = data.x;
  Mxyz[1] = data.y;
  Mxyz[2] = data.z;
  T       = data.t;
// send values to Bluetooth device
  ESP_BT.print(Mxyz[0]);
  ESP_BT.print(" ");
  ESP_BT.print(Mxyz[1]);
  ESP_BT.print(" ");
  ESP_BT.print(Mxyz[2]); 
  ESP_BT.print(" ");
  ESP_BT.print(T); 
  ESP_BT.println(); 

  delay(10);
}
