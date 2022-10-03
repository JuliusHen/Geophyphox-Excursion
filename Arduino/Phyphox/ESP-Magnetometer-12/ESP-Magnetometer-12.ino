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
  PhyphoxBLE::start("Magnetometer-12");
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
  { { -8.86682680e-04, -1.90060920e-03,  1.40623204e-02},
    {    -6.69500441e-04, -3.13905005e-01,  7.32413990e+00},
    { -4.60597071e-04, -2.65830694e-01,  5.66977923e+00 }
  };
  float Mxyz_final[3];

  float M_B[3]
  { -11.928011, -30.601441, 34.248609};

  float M_Ainv[3][3]
  { {  1.036952, -0.007439, -0.016170},
    { -0.007439 , 1.025198, -0.040178},
    { -0.016170, -0.040178, 1.035611}
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
