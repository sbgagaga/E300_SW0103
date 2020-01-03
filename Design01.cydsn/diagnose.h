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
#ifndef __DIAGNOSE__
#define __DIAGNOSE__

#include "Apply.h"    

#define DiagNAD 0x21
    
typedef struct
{
    uint8 length;
    uint8 pointer;
    uint8 * buffer;
}BUFFTER_TYPE;

void DiagPro();
void DiagRev();
void FillInBuffer(BUFFTER_TYPE * buffer,uint8 * date,uint8 len);
void LIN_SID22(uint16 DID);
void LIN_SID23();
void LIN_SID24();
void LIN_SID25();
void LIN_SID26();
void Send_I2C_Slave(uint8,uint8 *,uint8);
    
#endif
/* [] END OF FILE */
