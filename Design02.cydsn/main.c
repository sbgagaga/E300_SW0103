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

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    CyDelay(200);//上电延迟
    ADC_SAR_Seq_1_Start();
    ADC_SAR_Seq_1_StartConvert();
    CapSense_1_Start();
    I2C_Slave_I2CSlaveInitReadBuf(I2CWriteBuf,I2C_LEN);
    I2C_Slave_I2CSlaveInitWriteBuf(I2CReadBuf,13);
    Timer_Init();
    I2C_Slave_Start();
    I2C_1_Start();  
    AF_Init();
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
        I2C_task();
        MasterOrder();
    }
}

void Timer_Init()
{
    isr_1_StartEx(TimeBase_OnInterrupt);
    Timer_1_Start();
}

/* [] END OF FILE */
