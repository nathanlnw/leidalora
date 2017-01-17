/*
    Device CAN
*/
#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "Device_CAN.h"
#include <finsh.h>


TestStatus TestRx;
u8  CAN_initOver = 0;

CanRxMsg RxMessageData;
/**********************/
u8  CANBuffer[CANBufSize];
u32 CANBuffer_wr = 0;
u32 CANBuffer_rd = 0;
//u32 iout=0;
u32 iCANRX = 0;
/**********************/

//--------- Debug ----------
u32  Can_RXnum = 0;
u32  Can_sdnum = 0;
u32  Can_same = 0;
u32  Can_loudiao = 0;
u32  Can_notsame = 0;

void CANGPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* CAN GPIOs configuration **************************************************/
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Connect CAN pins to AF9 */
    GPIO_PinAFConfig( GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
    GPIO_PinAFConfig( GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);

    /* Configure CAN RX and TX pins */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    CAN_initOver = 1;
}
void CAN_App_Init(void)
{
    CAN_InitTypeDef 	   CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;



    CANGPIO_Configuration();

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    /* CAN register init */
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    /* CAN cell init */
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = DISABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;

    //----- Debug use---
#if      1           //  生产测试用 
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
    CAN_InitStructure.CAN_Prescaler = 100; //20K	        42MHZ     42/(1+bs1+bs2)/prescaler

#else
    // ----- 北斗扩展信息 ----------
    switch(BD_EXT.CAN_1_Mode & 0x60000000)  //mode
    {
    case 0x20000000:
        CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;
        break;
    case 0x40000000:
        CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
        break;
    case 0x60000000:
        CAN_InitStructure.CAN_Mode = CAN_Mode_Silent;
        break;
    }
    switch(BD_EXT.CAN_1_Mode & 0x0000FFFF)  // CAN Baud
    {
        /* 5k   10k  20k 50k 125k 250k 500k    --default 20k       42MHZ max */
    case 5:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 400; //5K                  42/(1+bs1+bs2)/prescaler
        break;
    case 10:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 200; //10K                  42/(1+bs1+bs2)/prescaler
        break;
    case 20:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 100; //20K                  42/(1+bs1+bs2)/prescaler
        break;
    case 50:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 40; //50K                  42/(1+bs1+bs2)/prescaler
        break;
    case 125:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 16; //125K                  42/(1+bs1+bs2)/prescaler
        break;
    case 250:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 8; //250K                  42/(1+bs1+bs2)/prescaler
        break;
    case 500:
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
        CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
        CAN_InitStructure.CAN_Prescaler = 4; //500K                  42/(1+bs1+bs2)/prescaler
        break;
    default:
        return;
    }
#endif
    CAN_Init(CAN1, &CAN_InitStructure);

    /* CAN filter init */
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);


    //-----------------------------
    /* Enable CAN1 RX0 interrupt IRQ channel */
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}
TestStatus CAN_TX(void)
{
    CanTxMsg TxMessage;
    u8 TransmitMailbox;
    u32 iCANTX = 0, iDelay = 0;

    /* transmit */
    TxMessage.StdId = 0x12;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_ID_STD; //BD_EXT.CAN_1_ID;
    TxMessage.DLC = 8;
    TxMessage.Data[0] = 0xCA;
    TxMessage.Data[1] = 0xAE;
    TxMessage.Data[2] = 0xAA;
    TxMessage.Data[3] = 0xBB;
    TxMessage.Data[4] = 0xCC;
    TxMessage.Data[5] = 0xDD;
    TxMessage.Data[6] = 0xEE;
    TxMessage.Data[7] = 0xFF;

    TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);
    iCANTX = 0;
    while((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (iCANTX != 0xFF))
    {
        iCANTX++;
        for(iDelay = 0; iDelay < 500; iDelay++);
    }
    if(iCANTX == 0xFF)
        return FAILED;
    else
        return PASSED;

}
TestStatus CAN_RX(void)
{
    u32 iCANRX = 0;

    iCANRX = 0;
    while((CAN_MessagePending(CAN1, CAN_FIFO0) < 1) && (iCANRX != 0xFF))
    {
        iCANRX++;
    }
    if(iCANRX != 0xFF)
    {
        /* receive */
        RxMessageData.Data[0] = 0x00;
        RxMessageData.Data[1] = 0x00;
        RxMessageData.Data[2] = 0x00;
        RxMessageData.Data[3] = 0x00;
        RxMessageData.Data[4] = 0x00;
        RxMessageData.Data[5] = 0x00;
        RxMessageData.Data[6] = 0x00;
        RxMessageData.Data[7] = 0x00;
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessageData);
        return PASSED;
    }
    else
        return FAILED;
}

u8  CAN1_Rx_Process(void)
{

    return true;
}

u8 TransmitMailbox = 0;;
u8 CANTXData(u8 *Instr, u8 len) //len=0~8
{
    CanTxMsg TxMessage;
    //u8 TransmitMailbox;
    u32 iCANTX = 0, iDelay = 0;
    /* transmit */
    TxMessage.StdId = 0x12;
    TxMessage.RTR = CAN_RTR_DATA;
    /*if(BD_EXT.CAN_1_Type==0)
          TxMessage.IDE=CAN_ID_EXT; ///  --- 至少支持扩展帧
    else
          TxMessage.IDE=CAN_ID_STD;
    */
    TxMessage.IDE = CAN_ID_EXT;
    TxMessage.DLC = len;
    if(GB19056.workstate == 0)
    {
        rt_kprintf("\r\nTxMessage=");
        for(iCANTX = 0; iCANTX < len; iCANTX++)
        {
            TxMessage.Data[iCANTX] = *Instr++;
            rt_kprintf(" %X", TxMessage.Data[iCANTX]);
        }
    }
    TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);
    TransmitMailbox = 1;

    iCANTX = 0;
    while((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (iCANTX != 0xFF))
    {
        iCANTX++;
        for(iDelay = 0; iDelay < 900; iDelay++);
    }
    if(iCANTX == 0xFF)
    {
        return FAILED;
    }
    else
    {
        return PASSED;
    }
}
u8 CANTXStr(u8 *Instr, u32 len)
{
    u32 iPacket = 0, iTXStr = 0, iTX2 = 0;
    u8 txdata[8];
    if((len % 8) == 0)
    {
        iPacket = len / 8;
        for(iTXStr = 0; iTXStr < iPacket; iTXStr++)
        {
            for(iTX2 = 0; iTX2 < 8; iTX2++)
                txdata[iTX2] = *Instr++;
            CANTXData(txdata, 8);
            for(iTX2 = 0; iTX2 < 800000; iTX2++);
        }
    }
    else
    {
        iPacket = (len / 8) + 1;
        for(iTXStr = 0; iTXStr < iPacket; iTXStr++)
        {
            if(iTXStr == (iPacket - 1))
            {
                for(iTX2 = 0; iTX2 < (len % 8); iTX2++)
                    txdata[iTX2] = *Instr++;
                CANTXData(txdata, (len % 8));
                for(iTX2 = 0; iTX2 < 800000; iTX2++);
            }
            else
            {
                for(iTX2 = 0; iTX2 < 8; iTX2++)
                    txdata[iTX2] = *Instr++;
                CANTXData(txdata, 8);
                for(iTX2 = 0; iTX2 < 800000; iTX2++);
            }
        }
    }
    return true;
}




