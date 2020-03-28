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
#include "project.h"
#include "Apply.h"

uint8 EEPROM_buf[13];

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    CyDelay(10);//上电延迟
    PWM_BEEP_Start();
    //PWM_BEEP_WriteCompare(250);
    Timer_Init();
    Lin_Init();
    CapSense_1_Start();
    CySysWdtEnable();
    WdtIsr_ClearPending();
    WdtIsr_StartEx(wdt_isr);
//    Em_EEPROM_1_Init(0x7800);
//    Em_EEPROM_1_Read(0, EEPROM_buf, 13);
//    if(EEPROM_buf[0]==0x24)
//    {
//        for(int i=0;i<6;i++)
//        {
//            KeyThreshold[i]=(uint16)EEPROM_buf[i*2+1]<<8|EEPROM_buf[i*2+2];
//        }
//    }
    I2C_1_Start(); 
    AF_Init();
    LowPowerFlag=1;
    //BEEP_Flag=1;
    for(;;)
    {
        /* Place your application code here. */
        Apply();       
        if(LowPowerFlag==1)
        {
            LIN_1_Stop();
            //LIN_1_ISR_Disable();
            PowerLock=1;
        }
        else if(LowPowerFlag==0&&PowerLock)
        {
            PowerLock=0;
            LIN_1_Start();
            //Lin_Init();
            //LIN_1_ISR_Disable();
        }
        
        LIN_task();
        CySysWdtClearInterrupt();
    }
}

void Timer_Init()
{
    isr_1_StartEx(TimeBase_OnInterrupt);
    Timer_1_Start();
}

void Lin_Init()
{
    unsigned int delay=250;
    if(0u != l_sys_init())
    {
        //initialize failed
    }
    if(0u != l_ifc_init(LIN_1_IFC_HANDLE))
    {
        //initialize failed
    }
    ld_init(LIN_1_IFC_HANDLE);
    LIN_EN_Write(0);
    while(delay--);
    LIN_EN_Write(1);
}

void wdt_isr()
{
    CySysWdtClearInterrupt();
}

/* [] END OF FILE */
