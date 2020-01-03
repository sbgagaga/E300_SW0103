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
    CyDelay(200);//上电延迟
    PWM_BEEP_Start();
    I2C_1_Start(); 
    Timer_Init();
    Lin_Init();
    AF_Init();//需要修改
    CapSense_1_Start();
    Em_EEPROM_1_Init(0x7800);
    Em_EEPROM_1_Read(0, EEPROM_buf, 13);
    if(EEPROM_buf[0]==0x24)
    {
        for(int i=0;i<6;i++)
        {
            KeyThreshold[i]=(uint16)EEPROM_buf[i*2+1]<<8|EEPROM_buf[i*2+2];
        }
    }
    for(;;)
    {
        /* Place your application code here. */
        Apply();
        LIN_task();
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

/* [] END OF FILE */
