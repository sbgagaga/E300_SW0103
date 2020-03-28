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
#include "diagnose.h"

/*触摸按键*/
typedef struct
{
    uint8 Menu:1;
    uint8 Mode:1;
    uint8 ADAS:1;
    uint8 Answer:1;
    uint8 Speech:1;
    uint8 DIST:1;
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
    uint8 Menu;
    uint8 Mode;
    uint8 ADAS;
    uint8 Answer;
    uint8 Speech;
    uint8 DIST;
    
}STRUCT_TOUCH_LIN;

typedef union
{
    uint8 Date[6];
    STRUCT_TOUCH_LIN Lin;
    
}UNION_TOUCH_LIN;
    
/*机械LIN*/
typedef struct
{
    uint8 RESPlus;
    uint8 Crusie;
    uint8 SETReduce;
    
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
#define I2C_LEN         48

#define RestReg         0x0F
#define Rest            0x01
#define KeyNum          6
#define error           3

extern uint8 baseline[12];
extern uint8 signline[12];
extern uint8 SlavePressValue[2];
extern uint16 KeyThreshold[KeyNum];
extern uint8 SlavePressThrValue[12];
extern uint8 LowPowerFlag;
extern uint8 PowerLock;
extern uint16 DeepSleep_time;
extern uint8 BEEP_Flag;


void TouchKeyScan();
void MechKeyScan();
uint8 KeyNumCheck();
void Timer_Init();
void Apply();       
void Beep_EN();
void LIN_task();
void I2C_task();
void Lin_Init();
void TouchKeyPro(uint8);
void MechKeyPro(uint8,uint16 *,uint8 *,uint8 *,uint8);
void LINRx();
void SleepCheck();
void SleepMode();
void SlaveGoSleep(uint8);
void KEY_Clean();
void SlaveReset();
void Key_Free();
uint8 SignalCheck();
void LIN_Check();
void wdt_isr();

uint32 CapSense_IsWidgetActive_bit(uint32 widgetId);

    
#endif
/* [] END OF FILE */
