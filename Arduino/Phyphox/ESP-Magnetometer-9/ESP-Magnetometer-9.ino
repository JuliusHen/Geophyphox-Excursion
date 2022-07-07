/*
  MLX90393 Magnetometer Example Code
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 6th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Read the mag fields on three XYZ axis
  Hardware Connections (Breakoutboard to Arduino):
  3.3V = 3.3V
  GND = GND
  SDA = A4
  SCL = A5
  Serial.print it out at 9600 baud to serial monitor.
*/

#include <Wire.h>
#include <MLX90393.h> //From https://github.com/tedyapo/arduino-MLX90393 by Theodore Yapo
#include <phyphoxBle.h>



MLX90393 mlx;
MLX90393::txyz data; //Create a structure, called data, of four floats (t, x, y, and z)


void setup()
{ 
  PhyphoxBLE::start("Magnetometer-9");
  Serial.begin(9600);


  Wire.begin();
  mlx.begin(); //Assumes I2C jumpers are GND. No DRDY pin used.
  mlx.reset();
  mlx.setGainSel(7);
  mlx.setResolution(0, 0, 0); //x, y, z
  mlx.setOverSampling(3);
  mlx.setDigitalFiltering(7);
 /*
  mlx.setTemperatureCompensation(3);
  mlx.setOffsets(32768,32768,32768);
 */
}
void loop()
{
  mlx.readData(data); //Read the values from the sensor


  float T_C[3][3]
  { {-5.74520421e-04, -3.26585112e-02,  5.72487892e-01},
    {    2.01285214e-03, -8.05596228e-01,  1.37815382e+01},
    { -7.22594149e-04, -2.55751057e-01,  5.30791334e+00 }
  };
  float Mxyz_final[3];

  float M_B[3]
  { -7.834588, -55.517695, 23.270871};

  float M_Ainv[3][3]
  { {  1.035012, -0.010076, 0.003399},
    { -0.010076 , 1.053415, -0.038374},
    { 0.003399, -0.038374, 1.060257}
  };

  float Mxyz[3], temp[3];
  float M_abs;
  float Mxyz_cal[3];
  float M_abs_final;
  float T;

  Mxyz[0] = data.x;
  Mxyz[1] = data.y;
  Mxyz[2] = data.z;
  T       = data.t;
  for (int i = 0; i < 3; i++) temp[i] = (Mxyz[i] - M_B[i]);
  Mxyz_cal[0] = M_Ainv[0][0] * temp[0] + M_Ainv[0][1] * temp[1] + M_Ainv[0][2] * temp[2];
  Mxyz_cal[1] = M_Ainv[1][0] * temp[0] + M_Ainv[1][1] * temp[1] + M_Ainv[1][2] * temp[2];
  Mxyz_cal[2] = M_Ainv[2][0] * temp[0] + M_Ainv[2][1] * temp[1] + M_Ainv[2][2] * temp[2];

  Mxyz_final[0] =   Mxyz_cal[0]     + ((T_C[0][0] * sq(T) + T_C[0][1] * T + T_C[0][2]));
  Mxyz_final[1] =   Mxyz_cal[1]     + ((T_C[1][0] * sq(T) + T_C[1][1] * T + T_C[1][2]));
  Mxyz_final[2] =   Mxyz_cal[2]     + ((T_C[2][0] * sq(T) + T_C[2][1] * T + T_C[2][2]));


  M_abs   = sqrt(sq(Mxyz[0]) + sq(Mxyz[1]) + sq(Mxyz[2]));
  M_abs_final   = sqrt(sq(Mxyz_final[0]) + sq(Mxyz_final[1]) + sq(Mxyz_final[2]));
  
  float heading = atan2(Mxyz_final[1],Mxyz_final[0]);
  float declinationAngle = 0.044907322; //  magnetic declination. 2°57’ east.
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
  
  Serial.print(headingDegrees);
  Serial.println();


  PhyphoxBLE::write(Mxyz_final[0],  Mxyz_final[1], Mxyz_final[2], M_abs_final, headingDegrees);
  delay(10);
}
