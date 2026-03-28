/*
******************************************************************************
File:     main.c
Info:     2026-03-18

The MIT License (MIT)
Copyright (c) 2026 M.Cetin Atila

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/


// Data types
#include <stdint.h>

// For sprintf
#include <stdio.h>

// STM32 peripheral library
#include "stm32f10x.h"

// SSD1306 Oled display driver
#include "ssd1306_driver/ssd1306.h"

// LFC font library
#include "lfc_font_lib/lfc_font.h"

// Include fonts
#include "fonts/C8_fonts.h"

// i2c functions
#include "i2c/i2c.h"

// BMP180 sensor library
#include "bmp180/bmp180.h"



void I2C1_Init(uint32_t speed) {

	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	/* GPIOB peripheral clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// I2C1 and I2C2 peripheral clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	// Configure I2C1 pins: SCL and SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 =0x10;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = speed;

	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);

}



uint8_t Device_Display_1306(DISPLAY_CONTEXT * device_display){

	if(device_display==NULL){
		return 1;
	}

	// Set display properties (for ssd1306)
	device_display->width		    = 128;    // Display width
	device_display->height		    = 64;     // Display height
	device_display->rotation	    = 0;	             // Screen rotation (0-3), 0: 0, 1:90, 2:180, 3:270 degree
	device_display->set_pixel_func  = SSD1306_Set_Pixel; // Pixel write function, it defined in display library

	return 0;
}


// I2C functions for bmp180
uint8_t _read_byte(uint8_t address, uint8_t * buffer, uint8_t len){
	return ReadByte(I2C1,BMP180_I2C_ADDRESS,address,buffer,len);
}

uint8_t _write_byte(uint8_t address, uint8_t * buffer, uint8_t len){
	return WriteByte(I2C1,BMP180_I2C_ADDRESS,address,buffer,len);
}

void _delay_ms(uint32_t ms){
    volatile uint32_t nCount;
    while(ms--)
    {
        nCount = 4000;
        while(nCount--) __NOP();
    }
}




int main(void) {

	// I2C speed max:400000
	uint32_t i2c_speed=400000;

	// STM32 I2C1 initialize
	I2C1_Init(i2c_speed);

	// SSD oled initialize
	SSD1306_Init();

	// Clear display
	SSD1306_Clear();

	// Flush display buffer
	SSD1306_Flush();

	// Display context
	DISPLAY_CONTEXT main_display;

	// Set display
    Device_Display_1306(&main_display);


    // Set print form to Goldman_Regular_16 (include all ASCII characters)
	PRINT_FORM system_font;
	system_font.config=LFC_DEFAULT_CONFIG;
	system_font.display_context=&main_display;
	system_font.padding=0;
	system_font.spacing=0;
	system_font.font=Goldman_Regular_16;


	// Set print form to Goldman_Regular_40 (include only numbers and +,-,.)
	PRINT_FORM big_number;
	big_number.config=LFC_DEFAULT_CONFIG;
	big_number.display_context=&main_display;
	big_number.padding=0;
	big_number.spacing=0;
	big_number.font=Goldman_Regular_40;


	// Set print form to Goldman_Regular_24 (include only numbers and +,-,. and degree symbol)
	PRINT_FORM mid_number;
	mid_number.config=LFC_DEFAULT_CONFIG;
	mid_number.display_context=&main_display;
	mid_number.padding=0;
	mid_number.spacing=0;
	mid_number.font=Goldman_Regular_24;


	// Set print form to Goldman_Regular_24 (include only numbers and +,-,. and degree symbol)
	PRINT_FORM altitude_font;
	altitude_font.config=LFC_BOUNDING_BOX;
	altitude_font.display_context=&main_display;
	altitude_font.padding=4;
	altitude_font.spacing=0;
	altitude_font.font=Goldman_Regular_24;


	//Create BMP180 structure
	BMP180_INF bmp180_inf;
	bmp180_inf.ReadByte =_read_byte;
	bmp180_inf.WriteByte=_write_byte;
	bmp180_inf.Delay_ms=_delay_ms;

	// String buffer for print function
	uint8_t text[50];
	uint8_t error = 0;
	uint8_t resolution = 1; // 0-3

	// Sample count: 2^x (0–8)
	// Smaller values give faster responses
	// Larger values give more stable results
	uint8_t sample_bit = 4; // 2^4=16


	BMP180_init:

	// SSD oled initialize after bmp180 init
	SSD1306_Init();

	// Start initializing BMP180 sensor
	do{

		// Clear display
		SSD1306_Clear();

		// Start top line of the screen
		int16_t pos_y= main_display.height-18;

		sprintf(text,"Sensor init");
		LFC_Print(&system_font,text,0,pos_y);
		pos_y-=18; // next line

	    error=BMP180_Init(&bmp180_inf);
	    if(error){
			// Print error code
	    	uint8_t *message=Get_Error_Message(error);
			sprintf(text,"Err:%s",message);
			LFC_Print(&system_font,text,0,pos_y);
			pos_y-=18; // next line

	    }else{
	    	// Read sensor data
			error=BMP180_Read(&bmp180_inf,resolution);
			if(error){
		    	uint8_t *message=Get_Error_Message(error);
				sprintf(text,"Err:%s",message);
				LFC_Print(&system_font,text,0,pos_y);
			}
	    }

		// Flush display buffer
		SSD1306_Flush();
	}while(error);


	int32_t temp_mean     = bmp180_inf.temperature << sample_bit;
	int32_t pressure_mean = bmp180_inf.pressure    << sample_bit;
	int32_t altitude_mean = bmp180_inf.altitude    << sample_bit;

	uint16_t cn=0;

	// SSD oled initialize
	SSD1306_Init();

	while(1){
		// Clear display
		SSD1306_Clear();

		// Read BMP180 sensor
		error=BMP180_Read(&bmp180_inf,resolution);
		if(error){
			// Something wrong, start again
			goto BMP180_init;
		}

		// Get sensor data
		int32_t temp_raw     = bmp180_inf.temperature;
		int32_t pressure_raw = bmp180_inf.pressure;
		int32_t altitude_raw = bmp180_inf.altitude;


		// Calculate average temperature value
		temp_mean-=(temp_mean  >> sample_bit);
		temp_mean+=temp_raw;
		int32_t temp=temp_mean >> sample_bit;

		// Calculate average pressure value
		pressure_mean-=(pressure_mean  >> sample_bit);
		pressure_mean+=pressure_raw;
		int32_t pressure=pressure_mean >> sample_bit;

		// Calculate average altitude value
		altitude_mean-=(altitude_mean  >> sample_bit);
		altitude_mean+=altitude_raw;
		int32_t altitude=altitude_mean >> sample_bit;


		// Print temperature value on the screen
		sprintf(text,"%d.",temp/10);
		int16_t pos_x=LFC_Print(&big_number,text,0,40);

		int8_t temp_fractional_part=(temp%10);
		if(temp_fractional_part<0){
			temp_fractional_part*=-1;
		}

		sprintf(text,"%d",temp_fractional_part);
		pos_x=LFC_Print(&mid_number,text,pos_x,40);


		// Print degree symbol, 176 is UTF32 code for degree symbol
		pos_x=LFC_Print_Utf32_Chr(&mid_number,176, pos_x,40);
		sprintf(text,"C");
		LFC_Print(&mid_number,text,pos_x,40);


		// Print pressure as hPa
		sprintf(text,"%d.",pressure/100 );
		pos_x=LFC_Print(&system_font,text,0,26);

		int8_t pressure_fractional_part=((pressure/10)%10);
		if(pressure_fractional_part<0){
			pressure_fractional_part*=-1;
		}
		sprintf(text,"%d hPa",pressure_fractional_part);
		LFC_Print(&system_font,text,pos_x,26);


		// Print Altitude
		sprintf(text,"%dm",altitude/100);
		pos_x=LFC_Print(&altitude_font,text,0,0);


		// Blink altitude value
		if(cn%50<25){
			// Clear invert mode
			altitude_font.config &= ~LFC_INVERT;
		}else{
			// Set invert mode
			altitude_font.config |= LFC_INVERT;
		}
		cn++;

		// Flush display buffer
		SSD1306_Flush();

	}


}
