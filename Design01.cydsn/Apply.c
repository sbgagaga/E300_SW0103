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
uint8 RESPlus,Crusie,SETReduce;
UNION_TOUCH_LIN touch_lin;
UNION_MECH_LIN mech_lin;  

uint8 TouchKey_State[6][6]={0};
uint8 MechKey_State[3][3]={0};

uint8 baseline[12];
uint8 signline[12];
uint8 SlavePressValue[2];
uint8 SlavePressThrValue[12];

uint16 TouchKeyCount,TouchKeyBuf,TouchKeyLock;
uint16 RESPlus_cnt,Crusie_cnt,SETReduce_cnt;
uint8 RESPlus_lock,Crusie_lock,SETReduce_lock;

uint8 I2CReadBuf[I2C_LEN]={0};//顺序：9个按键、1个AD、12个基线、12个信号、2个压力值、12个压力阀值
uint8 I2CWriteBuf[1]={0};
uint8 SlaveBuf[I2C_LEN]={0},LastReadBuf[I2C_LEN]={0};

int16 Force_Value;
uint8 BEEP_Flag;

//Menu、Mode、ADAS、Answer、Speech、DIST、3N
//uint16 KeyThreshold[KeyNum]={80,230,250,200,194,147};

//Menu、Mode、ADAS、Answer、Speech、DIST、2N
//uint16 KeyThreshold[KeyNum]={53,153,167,133,130,98};

//Menu、Mode、ADAS、Answer、Speech、DIST、弧面2N、平面3N
uint16 KeyThreshold[KeyNum]={53,230,250,200,130,98};

uint8 sleep_flag;
uint16 sleep_time;

uint8 flagSystemON;
uint16 varPowerUpCount;

void Apply()
{
    if(DEF_TICK_2mS == 1)
    {	 
        DEF_TICK_2mS = 0;
        if(flagSystemON==0&&varPowerUpCount<250)
        {
            varPowerUpCount++;
        }
        else if(varPowerUpCount>=250)
        {
            flagSystemON=1;
            varPowerUpCount=0;
            CapSense_1_InitializeAllBaselines();
        }
        if(flagSystemON)
        {
            TouchKeyScan();
            MechKeyScan();
            Key_Free();
        }
    }

    if(DEF_TICK_5mS == 1)
    {
        DEF_TICK_5mS = 0;        
    }

    if(DEF_TICK_10mS == 1)
    {
        DEF_TICK_10mS = 0;

        SleepCheck();
        I2C_task();
    }
    
    if(DEF_TICK_15mS == 1)
    {
        DEF_TICK_15mS = 0;
    }
    
    if(DEF_TICK_20mS == 1)
    {
        DEF_TICK_20mS = 0;
        //TouchKeyScan();
        //MechKeyScan();
    }

    if(DEF_TICK_50mS == 1)
    {
        DEF_TICK_50mS = 0;
        Beep_EN();
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
        
        touch_key.Key.Menu=CapSense_IsWidgetActive_bit(CapSense_1_MENU_WDGT_ID);
        touch_key.Key.Mode=CapSense_IsWidgetActive_bit(CapSense_1_MODE_WDGT_ID);
        touch_key.Key.ADAS=CapSense_IsWidgetActive_bit(CapSense_1_ADAS_WDGT_ID);
        touch_key.Key.Answer=CapSense_IsWidgetActive_bit(CapSense_1_ANSWER_WDGT_ID);
        touch_key.Key.Speech=CapSense_IsWidgetActive_bit(CapSense_1_SPEECH_WDGT_ID);
        touch_key.Key.DIST=CapSense_IsWidgetActive_bit(CapSense_1_DIST_WDGT_ID);
        
        if(KeyNumCheck())
        {
            touch_key.Date=0;
            TouchKeyLock=0;
            TouchKeyCount=0;
            KEY_Clean();
            CapSense_1_ScanAllWidgets();    //扫描所有的传感器
            return;
        }
        
        if(touch_key.Date!=0&&touch_key.Date==TouchKeyBuf&&!TouchKeyLock)
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
        else if(touch_key.Date!=0&&touch_key.Date==TouchKeyBuf&&TouchKeyLock)
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
        else if(touch_key.Date==0||touch_key.Date!=TouchKeyBuf)
        {
            if(TouchKeyLock)
            {
                for(int8 i=0;i<6;i++)
                {
                    if(touch_lin.Date[i]!=3)
                    {
                        if(touch_lin.Date[i]==2)
                        {
                            touch_lin.Date[i]=half_press;
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
    if(Force_Value>=KeyThreshold[index])
    {
        TouchKeyLock=1;
        BEEP_Flag=1;
        touch_lin.Date[index]=short_press;
    }
}

void MechKeyScan()
{  
    RESPlus=RESPlus_in_Read();
    Crusie=Crusie_in_Read();
    SETReduce=SETReduce_in_Read();
    if(KeyNumCheck())
    {
        RESPlus_cnt=0;
        Crusie_cnt=0;
        SETReduce_cnt=0;
        RESPlus_lock=0;
        Crusie_lock=0;
        SETReduce_lock=0;
        KEY_Clean();
        return;
    }
    MechKeyPro(RESPlus,&RESPlus_cnt,&RESPlus_lock,&mech_lin.Lin.RESPlus,0);
    MechKeyPro(Crusie,&Crusie_cnt,&Crusie_lock,&mech_lin.Lin.Crusie,1);
    MechKeyPro(SETReduce,&SETReduce_cnt,&SETReduce_lock,&mech_lin.Lin.SETReduce,2);
}

void MechKeyPro(uint8 KeyType,uint16 *KeyCnt,uint8 *KeyLock,uint8 *KeyLIN,uint8 index)
{
    if(KeyType==0&&!*KeyLock)
    {
        (*KeyCnt)++;
        if(*KeyCnt>=25)
        {
            BEEP_Flag=1;
            *KeyLock=1;
            *KeyLIN=short_press;
        }
    }
    else if(KeyType==0&&*KeyLock)
    {
        (*KeyCnt)++;
        if(*KeyCnt>=10000)
        {
            *KeyCnt=10000;
            *KeyLIN=error;
        }
    }
    else if(KeyType)
    {
        if(*KeyLock)
        {
            if(*KeyLIN!=error)
            {
                *KeyLIN=half_press;
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

uint8 KeyNumCheck()
{
    uint8 KeySum=0;
    for(uint8 mask=0x20; mask!=0; mask>>=1)
    {
        if(mask&touch_key.Date)
        {
            KeySum++;
            if(KeySum>=2) 
            {
                return 1;
            }
        }
    }
    if(!RESPlus)
    {
        KeySum++;
    }
    if(!Crusie)
    {
        KeySum++;
    }
    if(!SETReduce)
    {
        KeySum++;
    }
    if(KeySum>=2) 
    {
        return 1;
    }
    return 0; 
}

void KEY_Clean()
{
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

void Key_Free()
{
    for(int i=0;i<6;i++)
    {
        if(TouchKey_State[i][0]==1)
        {
            TouchKey_State[i][1]++;
            if(TouchKey_State[i][1]>75)
            {
                TouchKey_State[i][0]=0;
                TouchKey_State[i][1]=0;
                touch_lin.Date[i]=no_press;
                TouchKeyLock=0;
            }
        }
    }
    for(int i=0;i<3;i++)
    {
        if(MechKey_State[i][0]==1)
        {
            MechKey_State[i][1]++;
            if(MechKey_State[i][1]>75)
            {
                MechKey_State[i][0]=0;
                MechKey_State[i][1]=0;
                mech_lin.Date[i]=no_press;
                switch(i)
                {
                    case 0:RESPlus_lock=0;break;
                    case 1:Crusie_lock=0;break;
                    case 2:SETReduce_lock=0;break;
                }
            }
        }
    }
}

void I2C_task()
{
    uint8 I2C_temp,KeySum=0; 
    static uint8 same_flag,beep_flag;
    
    I2C_1_I2CMasterClearStatus();
    if(I2C_1_I2C_MSTR_NO_ERROR==I2C_1_I2CMasterReadBuf(I2C_Slave_dress, I2CReadBuf, I2C_LEN,
                                        I2C_1_I2C_MODE_COMPLETE_XFER))
    {
        while (0u == (I2C_1_I2CMasterStatus() & I2C_1_I2C_MSTAT_RD_CMPLT))
        {
            
        }
    }
    I2C_temp=I2C_1_I2CMasterStatus();
    I2C_1_I2CMasterClearStatus();
    if(I2C_temp&I2C_1_I2C_MSTAT_ERR_XFER)
    {
        return;
    }
    else
    {
        if(I2CReadBuf[0]!=0xFF)
        {
            for(uint8 i=0;i<10;i++)
            {
                if(I2CReadBuf[i]!=LastReadBuf[i])
                {
                    same_flag=0;
                    beep_flag=0;
                    break;
                }
                same_flag=1;
            }
            if(same_flag)
            {
                for(uint8 i=0;i<10;i++)
                {
                    SlaveBuf[i]=LastReadBuf[i];
                }
                for(uint8 i=0;i<9;i++)
                {
                    if(SlaveBuf[i]==2)
                    {
                        KeySum++;
                    }
                }
                if(KeySum!=0&&!beep_flag)
                {
                    BEEP_Flag=1;
                    beep_flag=1;
                }
                if(SlaveBuf[9]==2||SlaveBuf[9]==3)//ADC Control
                {
                    flagSystemON=0;
                    varPowerUpCount=0;
                }
                else if(SlaveBuf[9]==1)
                {
                    SlaveReset();
                    CySoftwareReset();
                }
                for(int i=0;i<12;i++)//基线值
                {
                    baseline[i]=I2CReadBuf[10+i];
                }
                for(int i=0;i<12;i++)//信号值
                {
                    signline[i]=I2CReadBuf[22+i];
                }
                for(int i=0;i<2;i++)//压力值
                {
                    SlavePressValue[i]=I2CReadBuf[34+i];
                }
                for(int i=0;i<12;i++)
                {
                    SlavePressThrValue[i]=I2CReadBuf[36+i];
                }
            }
            for(uint8 i=0;i<10;i++)
            {
                LastReadBuf[i]=I2CReadBuf[i];
            }
        }
    }
}

void LIN_task()
{
    l_u8_wr_MFS_Menu_switch_signal(touch_lin.Date[0]);
    l_u8_wr_MFS_Mode_switch_signal(touch_lin.Date[1]);
    l_u8_wr_MFS_ADAS_switch_signal(touch_lin.Date[2]);
    l_u8_wr_MFS_Answer_switch_signal(touch_lin.Date[3]);
    l_u8_wr_MFS_Speech_switch_signal(touch_lin.Date[4]);
    l_u8_wr_MFS_DIST_switch_signal(touch_lin.Date[5]);
    l_u8_wr_MFS_RESPlus_switch_signal(mech_lin.Date[0]);
    l_u8_wr_MFS_Crusie_switch_signal(mech_lin.Date[1]);
    l_u8_wr_MFS_SETReduce_switch_signal(mech_lin.Date[2]);
    l_u8_wr_MFS_Return_switch_signal(SlaveBuf[0]);
    l_u8_wr_MFS_Up_switch_signal(SlaveBuf[1]);
    l_u8_wr_MFS_Down_switch_signal(SlaveBuf[2]);
    l_u8_wr_MFS_SeekReduce_switch_signal(SlaveBuf[3]);
    l_u8_wr_MFS_OK_switch_signal(SlaveBuf[4]);
    l_u8_wr_MFS_SeekPlus_switch_signal(SlaveBuf[5]);
    l_u8_wr_MFS_VolPlus_switch_signal(SlaveBuf[6]);
    l_u8_wr_MFS_Mute_switch_signal(SlaveBuf[7]);
    l_u8_wr_MFS_VolReduce_switch_signal(SlaveBuf[8]);
    l_u8_wr_MFS_DiagInfoSW(SlaveBuf[9]);
    DiagRev();
    DiagPro();
}

void Beep_EN()
{
    static uint8 BEEP_cnt;
    if(BEEP_Flag)
    {
        PWM_BEEP_WriteCompare(250);
        BEEP_cnt++;
        if(BEEP_cnt>=3)
        {
            BEEP_cnt=0;
            BEEP_Flag=0;
            PWM_BEEP_WriteCompare(0);
        }
    }
}

void SleepCheck()
{
    uint8 state;
    state=l_ifc_read_status_LIN_1();
    if((state&0x02)&&(state&0x08)==0)
    {
        sleep_time=0;
        sleep_flag=0;
        SlaveGoSleep(sleep_flag);
    }
    else if(state&0x08)
    {
        sleep_flag=1;
        SlaveGoSleep(sleep_flag);
        SleepMode();
    }
    else if((state&0x02)==0)
    {
        sleep_time++;
        if(sleep_time>=1000)
        {
            sleep_flag=1;
            SlaveGoSleep(sleep_flag);
            SleepMode();
        }
    }
}

void SleepMode()
{
    if(sleep_flag)
    {
        

        AF_stop();
        PWM_BEEP_Stop();
        I2C_1_Stop();
        CapSense_1_Sleep();
        
        LIN_EN_Write(0);
        LIN_1_SCB_rx_Sleep();
        LIN_1_SCB_tx_Sleep();
        LIN_1_SCB_rx_SetDriveMode(LIN_1_SCB_rx_DM_RES_UP);//LIN_1_SCB_rx_DM_RES_UP
        CyIntSetVector(0, &LINRx);
        LIN_1_SCB_rx_SetInterruptMode(LIN_1_SCB_rx_0_INTR, LIN_1_SCB_rx_INTR_FALLING);
        CyIntEnable(0);
        LIN_1_SCB_rx_ClearInterrupt();
        
        CySysPmDeepSleep();
        CySoftwareReset();
        //while(1);
    }
}

void LINRx()
{
    LIN_1_SCB_rx_ClearInterrupt();
    LIN_1_SCB_tx_ClearInterrupt();
}

void SlaveGoSleep(uint8 state)
{
    uint8 I2C_temp;
    I2CWriteBuf[0]=state;
    do
    {
        I2C_temp = I2C_1_I2CMasterWriteBuf(I2C_Slave_dress, I2CWriteBuf, 
										1, I2C_1_I2C_MODE_COMPLETE_XFER);
    }
    while(I2C_temp != I2C_1_I2C_MSTR_NO_ERROR);
    while(I2C_1_I2CMasterStatus() & I2C_1_I2C_MSTAT_XFER_INP);
    I2C_temp = I2C_1_I2CMasterClearStatus();
    CyDelay(1);
}

void SlaveReset()
{
    uint8 I2C_temp;
    I2CWriteBuf[0]=2;
    do
    {
        I2C_temp = I2C_1_I2CMasterWriteBuf(I2C_Slave_dress, I2CWriteBuf, 
										1, I2C_1_I2C_MODE_COMPLETE_XFER);
    }
    while(I2C_temp != I2C_1_I2C_MSTR_NO_ERROR);
    while(I2C_1_I2CMasterStatus() & I2C_1_I2C_MSTAT_XFER_INP);
    I2C_temp = I2C_1_I2CMasterClearStatus();
    CyDelay(1);
}



/* [] END OF FILE */
