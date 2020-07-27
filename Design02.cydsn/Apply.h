/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef __APPLY__
#define __APPLY__
    
#include "project.h"
#include "event.h"
#include "I2C.h"
#include "AF_drive.h"

/*触摸按键*/
typedef struct
{
    uint8 Return:1;
    uint8 Up:1;
    uint8 Down:1;
    uint8 SeekReduce:1;
    uint8 OK:1;
    uint8 SeekPlus:1;
    uint8 reserve:2;
    
}STRUCT_TOUCH_KEY;    

typedef union
{
    uint8 Date;
    STRUCT_TOUCH_KEY Key;
    
}UNION_TOUCH_KEY;

/*触摸LIN*/
typedef struct
{
    uint8 Return;
    uint8 Source;
    uint8 Down;
    uint8 SeekReduce;
    uint8 OK;
    uint8 SeekPlus;
    
}STRUCT_TOUCH_LIN;

typedef union
{
    uint8 Date[6];
    STRUCT_TOUCH_LIN Lin;
    
}UNION_TOUCH_LIN;
    
/*机械LIN*/
typedef struct
{
    uint8 VolPlus;
    uint8 Mute;
    uint8 VolReduce;
    
}STRUCT_MECH_LIN;

typedef union
{
    uint8 Date[3];
    STRUCT_MECH_LIN Lin;
    
}UNION_MECH_LIN;


#define short_press     2
#define half_press      1
#define no_press        0
#define I2C_Slave_dress 0x08
#define I2C_LEN         37
#define Vol_17          2040
#define Vol_16P5        1994 
#define Vol_16          1930
#define Vol_9           1089
#define Vol_8P5         1029
#define Vol_6P5         786
#define Vol_6           725
#define ADC_index       9
#define RestReg         0x0F
#define Rest            0x01
#define KeyNum          6
#define error           3

extern uint8 I2CWriteBuf[I2C_LEN];
extern uint8 I2CReadBuf[13];
extern uint16 KeyThreshold[KeyNum];
extern uint8 EEPROM_buf[13];

void TouchKeyScan();
void MechKeyScan();
uint8 KeyNumCheck();
void Timer_Init();
void Apply();       
void I2C_task();
void I2C_Date_Pro();
void ADC_Check();
void Lin_Init();
void TouchKeyPro(uint8);
void MechKeyPro(uint8,uint16 *,uint8 *,uint8 *,uint8);
void MasterOrder();
uint32 CapSense_IsWidgetActive_bit(uint32 widgetId);
void KEY_Clean();
void Key_Free();
uint8 SignalCheck();
void I2C_Detect();

    
#endif
/* [] END OF FILE */
