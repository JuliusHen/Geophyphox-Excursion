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
#include <phyphoxBle.h>



MLX90393 mlx;
MLX90393::txyz data; //Create a structure, called data, of four floats (t, x, y, and z)


void setup()
{ 
  PhyphoxBLE::start("Magnetometer-1");
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

  // Temperature correction coefficients
  float T_C[3][3]
  { {   -1.10318387e-03,  1.65040045e-01, -3.37251127e+00},
    {    4.26082971e-04, -4.75875583e-01,  1.18696128e+01},
    {   -1.45040322e-03, -1.52003606e-01,  4.44652956e+00}
  };
  
  // Offset correction
  float M_B[3]
  { 16.117864, -49.947619, 37.924458};

  // Hard iron, soft iron, misaligment etc corrections
  float M_Ainv[3][3]
  { {  1.034092, -0.004798, 0.004807},
    { -0.004798 , 1.024437, -0.034644},
    { 0.004807, -0.034644, 1.028860}
  };

  // Initialize buffers
  float Mxyz_final[3];
  float Mxyz[3], temp[3];
  float M_abs;
  float Mxyz_cal[3];
  float M_abs_final;
  float T;

  Mxyz[0] = data.x;
  Mxyz[1] = data.y;
  Mxyz[2] = data.z;
  T       = data.t;

  // Calibration of raw data
  for (int i = 0; i < 3; i++) temp[i] = (Mxyz[i] - M_B[i]);
  Mxyz_cal[0] = M_Ainv[0][0] * temp[0] + M_Ainv[0][1] * temp[1] + M_Ainv[0][2] * temp[2];
  Mxyz_cal[1] = M_Ainv[1][0] * temp[0] + M_Ainv[1][1] * temp[1] + M_Ainv[1][2] * temp[2];
  Mxyz_cal[2] = M_Ainv[2][0] * temp[0] + M_Ainv[2][1] * temp[1] + M_Ainv[2][2] * temp[2];
  
  // Temperature correction with polynomial fit 
  Mxyz_final[0] =   Mxyz_cal[0]     + ((T_C[0][0] * sq(T) + T_C[0][1] * T + T_C[0][2]));
  Mxyz_final[1] =   Mxyz_cal[1]     + ((T_C[1][0] * sq(T) + T_C[1][1] * T + T_C[1][2]));
  Mxyz_final[2] =   Mxyz_cal[2]     + ((T_C[2][0] * sq(T) + T_C[2][1] * T + T_C[2][2]));

  // Calculate absolute magnetic field
  M_abs_final   = sqrt(sq(Mxyz_final[0]) + sq(Mxyz_final[1]) + sq(Mxyz_final[2]));

  // Compass   
  float heading = atan2(Mxyz_final[1],Mxyz_final[0]);
  float declinationAngle = 0.044907322; //  magnetic declination. 2°57’ east at Duerwiss Landfill
  heading += declinationAngle;
  // Correct for when signs are reversed.
  if (heading < 0) {
    heading += 2*PI;
  }
  // Check for wrap due to addition of declination.
  if (heading > 2*PI) {
    heading -= 2*PI;
  }
  float headingDegrees = heading * 180/M_PI; // Convert radians to degrees.

  // Send data to connected phyphox device
  PhyphoxBLE::write(Mxyz_final[0],  Mxyz_final[1], Mxyz_final[2], M_abs_final, headingDegrees);
  delay(10);
}
