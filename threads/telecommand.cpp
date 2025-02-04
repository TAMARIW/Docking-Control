
#include <stdlib.h>
#include "fsm.h"
#include "led.h"
#include "magnet.h"
#include "telecommand.h"
#include "current_control.h"
#include "satellite_config.h"
#include "collision_control.h"

bool led_switch = false;

namespace RODOS
{
  extern HAL_UART uart_stdout;
}

#define TeleUART uart_stdout

uint8_t ReceiveState = 0;
uint8_t SignFlag = 0;
uint8_t  DotFlag = 0;
uint8_t DataIndex = 0;
char telecommand_id;
char ReceiveData[TELECOMMAND_MAX_LEN];

HAL_GPIO power_reset_pin(GPIO_058);

uint8_t decode_command(uint8_t rx_buffer)
{
  uint8_t success=0;

  switch (ReceiveState)
  {
  case 0:
    SignFlag=0;
    DotFlag=0;
    DataIndex=0;
    if (rx_buffer==TELECOMMAND_START)
    {
      ReceiveState=1;
    }
    break;

  case 1:
    SignFlag=0;
    DotFlag=0;
    DataIndex=0;
    if (rx_buffer==TELECOMMAND_START)
    {
      ReceiveState=1;
    }
    else {
      telecommand_id = rx_buffer;
      ReceiveState = 2;
    }
    break;

  case 2:
    if (rx_buffer=='+' || rx_buffer=='-')
    {
      if (SignFlag==0 && DataIndex==0)
        {
        SignFlag=1;
        ReceiveData[DataIndex]=rx_buffer;
        DataIndex++;
        ReceiveState = 2;
        }
      else {ReceiveState = 0;}
    }
    else if (rx_buffer=='.')
    {
      if (DotFlag==0)
        {
          DotFlag=1;
          ReceiveData[DataIndex]=rx_buffer;
          DataIndex++;
          ReceiveState = 2;
        }
      else {ReceiveState = 0;}
    }
    else if (rx_buffer>='0' && rx_buffer<='9')
    {
        ReceiveData[DataIndex]=rx_buffer;
        DataIndex++;
      if (DataIndex > TELECOMMAND_MAX_LEN) {ReceiveState = 0;}
      else {ReceiveState = 2;}
    }
    else if (rx_buffer==TELECOMMAND_START)
    {
      ReceiveState=1;
    }
    else if (rx_buffer==TELECOMMAND_STOP)
    {
      ReceiveData[DataIndex]= 0x00;
      success=execute_command(telecommand_id);
      ReceiveState=0;
    }
    else { ReceiveState=0;}
    break;
  default:
    ReceiveState=0;
    break;
  }
  return success;
}

uint8_t execute_command(uint8_t telecommand_id)
{
  switch (telecommand_id)
  {
  case ENABLE_CONTROL:
  {
    control_mode = true;

    if(int(atof(ReceiveData))== 1) // Idle mode
    {
      // Magnets off and disable magnet thread
      tamariw_current_control_thread.stop_control = true;
      tamariw_collision_control_thread.stop_thread = true;
      fsm::set_state(STANDBY);
    }
    else // Resume control thread
    {
      tamariw_current_control_thread.stop_control = false;
      tamariw_collision_control_thread.stop_thread = false;
      tamariw_collision_control_thread.resume();
      fsm::set_state(START_DOCKING);
    }
    break;
  }
  case TEST_MAGNETS:
  {
    control_mode = false;
    for(uint8_t i = 0; i < 4; i++)
    {
      dpid[i].reset_memory();
    }
    break;
  }
  case PI_POS_GAIN_KP:
  {
    float kp = float(atof(ReceiveData));
    dpid[0].kp = kp;
    dpid[1].kp = kp;
    dpid[2].kp = kp;
    dpid[3].kp = kp;
    break;
  }
  case PI_POS_GAIN_KI:
  {
    float ki = float(atof(ReceiveData));
    dpid[0].ki = ki;
    dpid[1].ki = ki;
    dpid[2].ki = ki;
    dpid[3].ki = ki;
    break;
  }
  case PI_VEL_GAIN_KP:
  {
    break;
  }
  case PI_VEL_GAIN_KI:
  {
    break;
  }
  case DISTANCE_SP:
  {
    dsp = float(atof(ReceiveData));
    break;
  }
  default:
  {
    return 0;
  }
  }

  return 1;
}

void telecommand_thread::init()
{
  magnet::init();
  led::init_far();
  led::init_near();
}

void telecommand_thread::run()
{
  char rx_buffer;

  while (1)
  {
    TeleUART.suspendUntilDataReady();
    TeleUART.read(&rx_buffer,1);
    decode_command(rx_buffer);
  }
}

telecommand_thread tamariw_telecommand_thread("telecommand_thread", THREAD_PRIO_TELECOMMAND);
