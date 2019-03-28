/*
\file   main.c

\brief  Main source file.

(c) 2018 Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software and any
derivatives exclusively with Microchip products. It is your responsibility to comply with third party
license terms applicable to your use of third party software (including open source software) that
may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
FOR A PARTICULAR PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mcc_generated_files/application_manager.h"
#include "mcc_generated_files/led.h"
#include "mcc_generated_files/sensors_handling.h"
#include "mcc_generated_files/cloud/cloud_service.h"
#include "mcc_generated_files/debug_print.h"
#include "mcc_generated_files/drivers/i2c_simple_master.h"
#include "util/delay.h"
// IR Thermo Click(tm) constants
#define IR_THERMO_ADDRESS   0x5A
#define AMB_TEMP            0x06
#define OBJ_TEMP            0x07

//bool IR_SensorRead(uint8_t reg, float * pTemp)
//{
//    int16_t data;
//    I2C2_MESSAGE_STATUS status = I2C2_MESSAGE_PENDING;
//    static I2C2_TRANSACTION_REQUEST_BLOCK trb[2];
//
//    I2C2_MasterWriteTRBBuild(&trb[0], &reg, 1, IR_THERMO_ADDRESS);
//    I2C2_MasterReadTRBBuild(&trb[1], (uint8_t*)&data, 2, IR_THERMO_ADDRESS);
//    I2C2_MasterTRBInsert(2, &trb[0], &status);
//
//    while(status == I2C2_MESSAGE_PENDING);         // blocking
//    *pTemp = ((float)(data) *  0.02) - 273.15;     // convert to deg C
//
//    return (status == I2C2_MESSAGE_COMPLETE);
//}

//This handles messages published from the MQTT server when subscribed
void receivedFromCloud(uint8_t *topic, uint8_t *payload)
{
    char *toggleToken = "\"toggle\":";
    char *subString;

    if ((subString = strstr((char*)payload, toggleToken)))
    {
        LED_holdYellowOn( subString[strlen(toggleToken)] == '1' );
    }

    debug_printer(SEVERITY_NONE, LEVEL_NORMAL, "topic: %s", topic);
    debug_printer(SEVERITY_NONE, LEVEL_NORMAL, "payload: %s", payload);
}

// This will get called every CFG_SEND_INTERVAL only while we have a valid Cloud connection
void sendToCloud(void)
{
   static char json[70];

   int light = SENSORS_getLightValue();
    uint16_t temp = i2c_read2ByteRegister(IR_THERMO_ADDRESS, OBJ_TEMP);
    temp = (temp >> 8) | (temp << 8); // LSB/MSB swap
    float ftemp = ((float)(temp) *  0.02) - 273.15;     // convert to deg C

   int len = sprintf(json, "{\"Light\":%d,\"Temp\":%d.%02d}", light, (int)ftemp, (int)(ftemp*100));

   if (len >0) {
      CLOUD_publishData((uint8_t*)json, len);
      LED_flashYellow();
   }
}


int main(void)
{
   application_init();

   while (1)
   {
      runScheduler();
   }

   return 0;
}
