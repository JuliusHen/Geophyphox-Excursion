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
  ESP_BT.begin("Magnetometer-Calibration_14");

  float M_B[3]
  { -13.127087, -53.358581, 30.778381};

  float M_Ainv[3][3]
  { {  1.027754, -0.007257, -0.012053},
    { -0.007257 , 1.014241, -0.048628},
    { -0.012053, -0.048628, 1.042468}
  };

  float Mxyz[3], temp[3];
  float Mxyz_final[3];
  float Mxyz_cal[3];
  float M_abs;
  float T;

  Mxyz[0] = data.x;
  Mxyz[1] = data.y;
  Mxyz[2] = data.z;
  T       = data.t;

  for (int i = 0; i < 3; i++) temp[i] = (Mxyz[i] - M_B[i]);
  Mxyz_cal[0] = M_Ainv[0][0] * temp[0] + M_Ainv[0][1] * temp[1] + M_Ainv[0][2] * temp[2];
  Mxyz_cal[1] = M_Ainv[1][0] * temp[0] + M_Ainv[1][1] * temp[1] + M_Ainv[1][2] * temp[2];
  Mxyz_cal[2] = M_Ainv[2][0] * temp[0] + M_Ainv[2][1] * temp[1] + M_Ainv[2][2] * temp[2];

  ESP_BT.print(Mxyz_cal[0]);
  ESP_BT.print(" ");
  ESP_BT.print(Mxyz_cal[1]);
  ESP_BT.print(" ");
  ESP_BT.print(Mxyz_cal[2]); 
  ESP_BT.print(" ");
  ESP_BT.print(T); 
  ESP_BT.println();

  delay(10);
}
