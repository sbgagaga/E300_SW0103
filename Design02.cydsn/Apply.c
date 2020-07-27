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

#include "Apply.h"

UNION_TOUCH_KEY touch_key;
uint8 VolPlus,Mute,VolReduce;
UNION_TOUCH_LIN touch_lin;
UNION_MECH_LIN mech_lin;

uint16 TouchKeyCount,TouchKeyBuf,TouchKeyLock;
uint16 VolPlus_cnt,Mute_cnt,VolReduce_cnt;
uint8 VolPlus_lock,Mute_lock,VolReduce_lock;

uint8 TouchKey_State[6][6]={0};
uint8 MechKey_State[3][3]={0};

uint8 I2CWriteBuf[I2C_LEN]={0,0,0,0,0,0,0,0,0,0x01,0,0,0,0,0};
uint8 I2CReadBuf[13];
uint8 EEPROM_buf[13];

uint16 Force_Value;
int32 ADC_Value,ADC_Time;
uint8 RepPreFlag;

uint8 flagSystemON,PowerMode;
uint16 varPowerUpCount;

CapSense_1_RAM_WD_BUTTON_STRUCT* CapSense_WD_Pointer=(CapSense_1_RAM_WD_BUTTON_STRUCT*)&CapSense_1_dsRam.wdgtList;
CapSense_1_RAM_SNS_STRUCT*       CapSense_SNS_Pointer=(CapSense_1_RAM_SNS_STRUCT*)&CapSense_1_dsRam.snsList;


//Return、Up、Down、rr、OK、SeekPlus、3N
//uint16 KeyThreshold[KeyNum]={129,65,110,91,196,187};

//Return、Up、Down、SeekReduce、OK、SeekPlus、2N
//uint16 KeyThreshold[KeyNum]={86,43,73,61,130,125};

//Return、Up、Down、SeekReduce、OK、SeekPlus、3N
//uint16 KeyThreshold[KeyNum]={230,78,69,128,191,283};

//Return、Up、Down、SeekReduce、OK、SeekPlus、弧面2N、平面3N
uint16 KeyThreshold[KeyNum]={129,43,73,61,196,187};


uint16 SDA_cnt;

uint8 ADC_Lock=0;

uint8 MechAllFreeFlag=0;
uint8 MechAllFreeCnt=0;

void Apply()
{
    if(DEF_TICK_2mS == 1)
    {	 
        DEF_TICK_2mS = 0;
        if(flagSystemON==0&&varPowerUpCount<250)
        {
            varPowerUpCount++;
            KEY_Clean();
            CapSense_1_ProcessAllWidgets();
            CapSense_1_ScanAllWidgets();
            CapSense_1_InitializeAllBaselines();
        }
        else if(varPowerUpCount>=250)
        {
            flagSystemON=1;
            varPowerUpCount=0;
            CapSense_1_InitializeAllBaselines();
        }
        if(flagSystemON && PowerMode == 0)
        {
            TouchKeyScan();
            MechKeyScan();
            if(KeyNumCheck())
            {
                TouchKeyCount=0;
                VolPlus_cnt=0;
                Mute_cnt=0;
                VolReduce_cnt=0;
                //CapSense_1_InitializeAllBaselines();
                //KEY_Clean();
            }
            Key_Free();
        }
        I2C_Detect();
    }

    if(DEF_TICK_5mS == 1)
    {
        DEF_TICK_5mS = 0; 
    }

    if(DEF_TICK_10mS == 1)
    {
        DEF_TICK_10mS = 0;
        ADC_Check();
    }
    
    if(DEF_TICK_15mS == 1)
    {
        DEF_TICK_15mS = 0;
    }
    
    if(DEF_TICK_20mS == 1)
    {
        DEF_TICK_20mS = 0;
    }

    if(DEF_TICK_50mS == 1)
    {
        DEF_TICK_50mS = 0;
    }

    if(DEF_TICK_100mS == 1)
    {
        DEF_TICK_100mS = 0;
   
    }	    
}

uint32 CapSense_IsWidgetActive_bit(uint32 widgetId)
{
    return (CapSense_1_IsWidgetActive(widgetId)>>widgetId);
}

void TouchKeyScan()
{
    if(CapSense_1_NOT_BUSY == CapSense_1_IsBusy())//检查传感器是否忙碌
    {
        CapSense_1_ProcessAllWidgets();       //处理所有传感器
        
        touch_key.Key.Return=CapSense_IsWidgetActive_bit(CapSense_1_RET_WDGT_ID);
        touch_key.Key.Up=CapSense_IsWidgetActive_bit(CapSense_1_UP_WDGT_ID);
        touch_key.Key.Down=CapSense_IsWidgetActive_bit(CapSense_1_DOWN_WDGT_ID);
        touch_key.Key.SeekReduce=CapSense_IsWidgetActive_bit(CapSense_1_SEEKREDUCE_WDGT_ID);
        touch_key.Key.OK=CapSense_IsWidgetActive_bit(CapSense_1_OK_WDGT_ID);
        touch_key.Key.SeekPlus=CapSense_IsWidgetActive_bit(CapSense_1_SEEKPLUS_WDGT_ID);
        
        if(SignalCheck())
        {
            CapSense_1_InitializeAllBaselines();
            return;
        }
        
        if(touch_key.Date!=0&&touch_key.Date==TouchKeyBuf&&!TouchKeyLock&&!RepPreFlag)
        {
            TouchKeyCount++;
            if(TouchKeyCount>=25)
            {
                TouchKeyCount=25;
                for(uint8 index=0,mask=0x01; mask!=0x40; mask<<=1,index++)
                {
                    if(mask&touch_key.Date)
                    {
                        TouchKeyPro(index);
                        break;
                    }
                }
            }
        }
        else if(touch_key.Date!=0&&touch_key.Date==TouchKeyBuf&&TouchKeyLock&&!RepPreFlag)
        {
            TouchKeyCount++;
            if(TouchKeyCount>=10000)
            {
                TouchKeyCount=10000;
                for(int8 i=0;i<6;i++)
                {
                    if(touch_lin.Date[i]==2)
                    {
                        touch_lin.Date[i]=error;
                        break;
                    }
                }
            }
        }
        else if(touch_key.Date==0||touch_key.Date!=TouchKeyBuf||RepPreFlag)
        {
            if(TouchKeyLock)
            {
                for(int8 i=0;i<6;i++)
                {
                    if(touch_lin.Date[i]!=3)
                    {
                        if(touch_lin.Date[i]==2)
                        {
                            touch_lin.Date[i]=short_press;
                            TouchKey_State[i][0]=1;
                            break;
                        }
                    }
                    else
                    {
                        touch_lin.Date[i]=0;
                        TouchKeyLock=0;
                        break;
                    }
                    
                }
            }
            TouchKeyCount=0;
        }
        TouchKeyBuf=touch_key.Date;
        CapSense_1_ScanAllWidgets();    //扫描所有的传感器
    }
}

void TouchKeyPro(uint8 index)
{
    Force_Value=AF_sampling();
    if(Force_Value>=KeyThreshold[index]&&!TouchKeyLock)
    {
        TouchKeyLock=1;
        touch_lin.Date[index]=short_press;
    }
}

void MechKeyScan()
{
    VolPlus=VolPlus_in_Read();
    Mute=Mute_in_Read();
    VolReduce=VolReduce_in_Read();

    if(MechAllFreeFlag)
    {
        MechKeyPro(VolPlus,&VolPlus_cnt,&VolPlus_lock,&mech_lin.Lin.VolPlus,0);
        MechKeyPro(Mute,&Mute_cnt,&Mute_lock,&mech_lin.Lin.Mute,1);
        MechKeyPro(VolReduce,&VolReduce_cnt,&VolReduce_lock,&mech_lin.Lin.VolReduce,2);
    }
    else
    {
        if(VolPlus&&Mute&&VolReduce)
        {
            MechAllFreeFlag=1;
        }
    }
}

void MechKeyPro(uint8 KeyType,uint16 *KeyCnt,uint8 *KeyLock,uint8 *KeyLIN,uint8 index)
{
    if(KeyType==0&&!*KeyLock&&!RepPreFlag)
    {
        (*KeyCnt)++;
        if(*KeyCnt>=25)
        {
            *KeyLock=1;
            *KeyLIN=short_press;
        }
    }
    else if(KeyType==0&&*KeyLock&&!RepPreFlag)
    {
        (*KeyCnt)++;
        if(*KeyCnt>=10000)
        {
            *KeyCnt=10000;
            *KeyLIN=error;
        }
    }
    else if(KeyType||RepPreFlag)
    {
        if(*KeyLock)
        {
            if(*KeyLIN!=error)
            {
                *KeyLIN=short_press;
                MechKey_State[index][0]=1;
            }
            else
            {
                *KeyLIN=0;
                *KeyLock=0;
            }
        }
        *KeyCnt=0;
    }
}

uint8 SignalCheck()
{
    uint32 cnt;
    CapSense_1_GetParam(CapSense_1_RET_SNS0_DIFF_PARAM_ID,&cnt);
    if(cnt>250) return 1;
    CapSense_1_GetParam(CapSense_1_UP_SNS0_DIFF_PARAM_ID ,&cnt);
    if(cnt>250) return 1;
    CapSense_1_GetParam(CapSense_1_DOWN_SNS0_DIFF_PARAM_ID,&cnt);
    if(cnt>250) return 1;
    CapSense_1_GetParam(CapSense_1_SEEKREDUCE_SNS0_DIFF_PARAM_ID,&cnt);
    if(cnt>250) return 1;
    CapSense_1_GetParam(CapSense_1_OK_SNS0_DIFF_PARAM_ID,&cnt);
    if(cnt>250) return 1;
    CapSense_1_GetParam(CapSense_1_SEEKPLUS_SNS0_DIFF_PARAM_ID,&cnt);
    if(cnt>250) return 1;
    return 0;
}

uint8 KeyNumCheck()
{
    uint8 KeySum=0;
    for(uint8 mask=0x20; mask!=0; mask>>=1)
    {
        if(mask&touch_key.Date)
        {
            KeySum++;
        }
    }
    if(!VolPlus)
    {
        KeySum++;
    }
    if(!Mute)
    {
        KeySum++;
    }
    if(!VolReduce)
    {
        KeySum++;
    }
    if(KeySum>=2)
    {
        RepPreFlag=1;
        return 1;
    }
    if(KeySum==0)
    {
        RepPreFlag=0;
    }
    return 0;
}

void Key_Free()
{
    for(int i=0;i<6;i++)
    {
        if(TouchKey_State[i][0]==1)
        {
            TouchKey_State[i][1]++;
            if(TouchKey_State[i][1]>25&&TouchKey_State[i][1]<100)
            {
                touch_lin.Date[i]=half_press;
            }
            else if(TouchKey_State[i][1]>=100)
            {
                TouchKey_State[i][0]=0;
                TouchKey_State[i][1]=0;
                TouchKeyLock=0;
                touch_lin.Date[i]=no_press;
            }
        }
    }
    for(int i=0;i<3;i++)
    {
        if(MechKey_State[i][0]==1)
        {
            MechKey_State[i][1]++;
            if(MechKey_State[i][1]>25&&MechKey_State[i][1]<100)
            {
                mech_lin.Date[i]=half_press;
            }
            else if(MechKey_State[i][1]>=100)
            {
                mech_lin.Date[i]=no_press;
                MechKey_State[i][0]=0;
                MechKey_State[i][1]=0;
                switch(i)
                {
                    case 0:VolPlus_lock=0;break;
                    case 1:Mute_lock=0;break;
                    case 2:VolReduce_lock=0;break;
                }
            }
        }
    }
}

void KEY_Clean()
{
    TouchKeyCount=0;
    TouchKeyLock=0;
    VolPlus_cnt=0;
    Mute_cnt=0;
    VolReduce_cnt=0;
    VolPlus_lock=0;
    Mute_lock=0;
    VolReduce_lock=0;
    RepPreFlag=0;
    for(int i=0;i<6;i++)
    {
        for(int j=0;j<2;j++)
        {
            TouchKey_State[i][j]=0;
        }
    }
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<2;j++)
        {
            MechKey_State[i][j]=0;
        }
    }
    for(int i=0;i<6;i++)
    {
        touch_lin.Date[i]=0;
    }
    for(int i=0;i<3;i++)
    {
        mech_lin.Date[i]=0;
    }
}

void I2C_Date_Pro()
{
    uint8 CheckSum=0;
    int i=0;
    for(i=0;i<6;i++)
    {
        I2CWriteBuf[i]=touch_lin.Date[i];
    }
    for(;i<9;i++)
    {
        I2CWriteBuf[i]=mech_lin.Date[i-6];
    }
    for(int i=0;i<10;i++)
    {
        CheckSum+=I2CWriteBuf[i];
    }
    I2CWriteBuf[10]=CheckSum^0xFF;
    for(i=0;i<6;i++)
    {
        I2CWriteBuf[i*2+11]=CapSense_SNS_Pointer[i].bsln[0]>>8;
        I2CWriteBuf[i*2+1+11]=(uint8)(CapSense_SNS_Pointer[i].bsln[0]);
    }
    for(i=0;i<6;i++)
    {
        I2CWriteBuf[i*2+23]=CapSense_SNS_Pointer[i].diff>>8;
        I2CWriteBuf[i*2+1+23]=(uint8)CapSense_SNS_Pointer[i].diff;
    }
    while(ReadCommand(addr,0x00)==addr)
    {
        addr++;
        if(addr>=0x48)
        {
            addr=0x44;
        }
    }
    I2CWriteBuf[35]=ReadCommand(addr,0x03);
    I2CWriteBuf[36]=ReadCommand(addr,0x04);
}

void I2C_task()
{
    if (0u != (I2C_Slave_I2CSlaveStatus() & I2C_Slave_I2C_SSTAT_RD_CMPLT))
    {
        /* Clear slave read buffer and status */
        I2C_Slave_I2CSlaveClearReadBuf();
        (void) I2C_Slave_I2CSlaveClearReadStatus();      
    }
    if (0u != (I2C_Slave_I2CSlaveStatus() & I2C_Slave_I2C_SSTAT_WR_CMPLT))
    {
        /* Clear slave write buffer and status */
        I2C_Slave_I2CSlaveClearWriteBuf();
        (void)I2C_Slave_I2CSlaveClearWriteStatus();
    }
}

void ADC_Check()
{
    if(ADC_SAR_Seq_1_IsEndConversion(ADC_SAR_Seq_1_WAIT_FOR_RESULT)==1)
    {
        ADC_Value=ADC_SAR_Seq_1_GetResult16(0);
       
        if(ADC_Value>=Vol_17)
        {
            PowerMode = 1;
        }
        else if(ADC_Value>=Vol_16P5&&ADC_Value<Vol_17)
        {
            PowerMode = 2;
        }
        else if(ADC_Value>=Vol_16&&ADC_Value<Vol_16P5)
        {
            if(PowerMode==0)
            {
                PowerMode=0;
            }
            else
            {
                PowerMode=2;
            }
        }
        else if(ADC_Value>=Vol_9&&ADC_Value<Vol_16)
        {
            PowerMode = 0;
        }
        else if(ADC_Value>=Vol_8P5&&ADC_Value<Vol_9)
        {
            if(PowerMode==0)
            {
                PowerMode=0;
            }
            else
            {
                PowerMode=3;
            }
        }
        else if(ADC_Value>=Vol_6P5&&ADC_Value<Vol_8P5)
        {
            PowerMode = 3;
        }
        else if(ADC_Value>=Vol_6&&ADC_Value<Vol_6P5)
        {
            if(PowerMode==1)
            {
                PowerMode=1;
            }
            else
            {
                PowerMode=3;
            }
        }
        else if(ADC_Value<Vol_6)
        {
            PowerMode = 1;
        }
        switch(PowerMode)
        {
            case 0:
            I2CWriteBuf[ADC_index]=0;
            break;
            case 1:
            I2CWriteBuf[ADC_index]=1;
            break;
            case 2:
            I2CWriteBuf[ADC_index]=2;
            break;
            case 3:
            I2CWriteBuf[ADC_index]=3;
            break;
        }
        if(PowerMode!=0)
        {
            flagSystemON=0;
            varPowerUpCount=0;
            MechAllFreeFlag=0;
        }
    }
}

void MasterOrder()
{
    switch(I2CReadBuf[0])
    {
        case 1:
        I2CReadBuf[0]=0;
        I2C_Slave_Sleep();
        //ADC_SAR_Seq_1_Stop();
        //AF_stop();
        CapSense_1_Stop();
        //I2C_1_Stop();
        CySysWdtClearInterrupt();
        CySysWdtDisable();
        
        CySysPmDeepSleep();
        I2C_Slave_Wakeup();
        CySoftwareReset();
        CySysWdtEnable();
        CapSense_1_Start();
        //ADC_SAR_Seq_1_Start();
        //ADC_SAR_Seq_1_StartConvert();
        //I2C_1_Start();
        
        break;
        
        case 2:
        CySoftwareReset();
        break;
        
        case 0x24:
        for(int i=0;i<12;i++)
        {
            EEPROM_buf[i+1]=I2CReadBuf[i+1];
        }
        EEPROM_buf[0]=0x24;
        Em_EEPROM_1_Write(0,EEPROM_buf,13); 
        for(int i=0;i<6;i++)
        {
            KeyThreshold[i]=(uint16)I2CReadBuf[i*2+1]<<8|I2CReadBuf[i*2+2];
        }
        break;
    }
}

void I2C_Detect()
{
    uint8 SDA_State=0;
    SDA_State=I2C_Slave_sda_Read();
    if(SDA_State==0)
    {
        SDA_cnt++;
        if(SDA_cnt==125)
        {
            CySoftwareReset();
        }
    }
    else
    {
        SDA_cnt=0;
    }
}








/* [] END OF FILE */
