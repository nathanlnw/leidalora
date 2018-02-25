/**************************************************************************
 *
 *   sed1520.c
 *   LCD display controller interface routines for graphics modules
 *   with onboard SED1520 controller(s) in "write-only" setup
 *
 *   Version 1.02 (20051031)
 *
 *   For Atmel AVR controllers with avr-gcc/avr-libc
 *   Copyright (c) 2005
 *     Martin Thomas, Kaiserslautern, Germany
 *     <eversmith@heizung-thomas.de>
 *     http://www.siwawi.arubi.uni-kl.de/avr_projects
 *
 *   Permission to use in NON-COMMERCIAL projects is herbey granted. For
 *   a commercial license contact the author.
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *   partly based on code published by:
 *   Michael J. Karas and Fabian "ape" Thiele
 *
 *
 ***************************************************************************/

/*
   An Emerging Display EW12A03LY 122x32 Graphics module has been used
   for testing. This module only supports "write". There is no option
   to read data from the SED1520 RAM. The SED1520 R/W line on the
   module is bound to GND according to the datasheet. Because of this
   Read-Modify-Write using the LCD-RAM is not possible with the 12A03
   LCD-Module. So this library uses a "framebuffer" which needs
   ca. 500 bytes of the AVR's SRAM. The libray can of cause be used
   with read/write modules too.
*/

/* tab-width: 4 */

//#include <LPC213x.H>
//#include <includes.h>

#include <stdint.h>
#include "LCD_Driver.h"
#include "bmp.h"
#include "stm32f2xx.h"

#include "Lcd.h"
#include "Menu_Include.h"

#define b_CardEdge   0x0001

#define IO_Group       GPIOD
#define IO_Group_NUM   GPIO_Pin_5

//unsigned int  ICcardCheckDelayNum=0;
//unsigned char ICcardInsert_PowerOn=1;//设备上电时IC卡已经插入
//unsigned char ICcardCheck_pc7_LH=0;//PC7为低时=1，PC7变为高时=2，初始化IC卡=3
//unsigned int R_Flag=0;
void Init_lcdkey(void)
{

    GPIO_InitTypeDef  GPIO_InitStructure;

    //按键	  PE1:down	  PB7:up	PB8:ok	   PB9:mune
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

    GPIO_InitStructure.GPIO_Pin = KEY_UP_PIN;
    GPIO_Init(KEY_UP_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY_DOWN_PIN;
    GPIO_Init(KEY_DOWN_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY_OK_PIN;
    GPIO_Init(KEY_OK_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY_MENU_PIN;
    GPIO_Init(KEY_MENU_PORT, &GPIO_InitStructure);

	 GPIO_InitStructure.GPIO_Pin = KEY_RTN_BACK_PIN;
    GPIO_Init(KEY_RTN_BACK_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = KEY_BAKUSE_PIN;
    GPIO_Init(KEY_BAKUSE_PORT, &GPIO_InitStructure);


   // 显示屏管脚初始化

    // INPUT
    GPIO_InitStructure.GPIO_Pin = LCD_BUSY_PIN;
    GPIO_Init(LCD_BUSY_PORT, &GPIO_InitStructure);
   
    //OUT   DB7-DB1
    GPIO_InitStructure.GPIO_Pin = LCD_DB7_PIN| LCD_DB6_PIN | LCD_DB5_PIN | LCD_DB4_PIN|LCD_DB3_PIN|LCD_DB2_PIN|LCD_DB1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LCD_DB7_PORT, &GPIO_InitStructure);

    // OUT  DB0  
	GPIO_InitStructure.GPIO_Pin = LCD_DB0_PIN|LCD_REQ_PIN|LCD_RES_PIN;
    GPIO_Init(LCD_DB0_PORT, &GPIO_InitStructure);

   //  OUT  LED
	 GPIO_InitStructure.GPIO_Pin = LCD_LED_PIN;
    GPIO_Init(LCD_LED_PORT, &GPIO_InitStructure);


    LCD_RstLow(); // 
    delay_ms(5);
    LCD_LED_ON;

}


void KeyCheckFun(void)
{
    if(!GPIO_ReadInputDataBit(KEY_UP_PORT, KEY_UP_PIN))
    {
        KeyCheck_Flag[0]++;
        if(KeyCheck_Flag[0] == 1)
            KeyValue = 1;
    }
    else
        KeyCheck_Flag[0] = 0;

    if(!GPIO_ReadInputDataBit(KEY_MENU_PORT, KEY_MENU_PIN))
    {
        KeyCheck_Flag[1]++;
            if(KeyCheck_Flag[1] == 1)
              {
                 KeyValue = 2;
               }	 
    }
    else
        KeyCheck_Flag[1] = 0;

    if(!GPIO_ReadInputDataBit(KEY_OK_PORT, KEY_OK_PIN))
    {
        KeyCheck_Flag[2]++;
        if(KeyCheck_Flag[2] == 1)
            KeyValue = 3;
        else if((KeyCheck_Flag[2] > 10) && (KeyCheck_Flag[2] % 2 == 0))
            KeyValue = 3;
    }
    else
        KeyCheck_Flag[2] = 0;

    if(!GPIO_ReadInputDataBit(KEY_DOWN_PORT, KEY_DOWN_PIN))
    {
        KeyCheck_Flag[3]++;

        if(KeyCheck_Flag[3] == 1)
            KeyValue = 4;
        else if((KeyCheck_Flag[3] > 10) && (KeyCheck_Flag[3] % 2 == 0))
            KeyValue = 4;
    }
    else
        KeyCheck_Flag[3] = 0;

    //======  below   便携台--------------------
    if(!GPIO_ReadInputDataBit(KEY_RTN_BACK_PORT, KEY_RTN_BACK_PIN))    // 退出键
    {
        KeyCheck_Flag[4]++;

        if(KeyCheck_Flag[4] == 1)
            KeyValue = 5;
        else if((KeyCheck_Flag[4] > 10) && (KeyCheck_Flag[4] % 2 == 0))
            KeyValue = 5;
    }
    else
        KeyCheck_Flag[4] = 0;

	if(!GPIO_ReadInputDataBit(KEY_BAKUSE_PORT, KEY_BAKUSE_PIN))      // 备用键
    {
        KeyCheck_Flag[5]++;

        if(KeyCheck_Flag[5] == 1)
            KeyValue = 6;
        else if((KeyCheck_Flag[5] > 10) && (KeyCheck_Flag[5] % 2 == 0))
            KeyValue = 6;
    }
    else
        KeyCheck_Flag[5] = 0;


	//rt_kprintf("\r\n Keyvalue=%d  \r\n",KeyValue);  
}


void lcd_light(u8 value) 
{
    if(value)
      { 
         LCD_LED_ON;
		 rt_kprintf("\r\n 背光开启\r\n");
      }		 
	else
	 {
	     LCD_LED_OFF;
         rt_kprintf("\r\n 背光关闭\r\n");
	 }  
}
FINSH_FUNCTION_EXPORT(lcd_light, lcd_light(1|0)) ;  	
