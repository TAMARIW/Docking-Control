/*****************************************************************
topics.h

Original Created by: Atheel Redah @ University of W�rzburg
Original Creation Date: March 8, 2015

Development environment specifics:
  Software Platform: Rodos (Realtime Onboard Dependable Operating System).
  Hardware Platform: STM32F4 + W�rzburg Uni Informatik 8 Discovery AddOn Board Version 2.0.
*****************************************************************/

#ifndef __TOPICS_H__
#define __TOPICS_H_

#include "rodos.h"

struct sTelecommandData
{
  bool Telemetry;
  float DistanceUWB;
};

struct sLidarData{
  int16_t lidar1, lidar2, lidar3, lidar4; //Filtered Distance from Lidar1, Lidar2, lidar3, Lidar4#
  float vel1, vel2, vel3, vel4;
  float yaw;
  double deltaTime;
};

struct sCurrentData
{
  float i0;
  float i1;
  float i2;
  float i3;
};

struct sCalculation
{
  float Velocity;
  float Position;
  float acceleration;
};

struct Orientation{
  float yaw, pitch;
};

extern Topic<sTelecommandData> TelecommandDataTopic;
extern Topic<sLidarData> LidarDataTopic;
extern Topic<sCurrentData> CurrentDataTopic;

#endif // telecommand.h

