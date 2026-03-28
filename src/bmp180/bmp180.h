/*
******************************************************************************
File:     bmp180.h.c
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

#ifndef BMP180_H_
#define BMP180_H_

#include <stdint.h>
#include <stddef.h>

#define BMP180_I2C_ADDRESS 0xEE //0xEE



// i2c functions they must defined in main file

typedef uint8_t ( *bmp180_read_byte) (uint8_t address, uint8_t * buffer, uint8_t len );
typedef uint8_t ( *bmp180_write_byte)(uint8_t address, uint8_t * buffer, uint8_t len );

typedef void ( *delay_ms)(uint32_t );


// BMP180 register addresses
#define R_AC1 0xAA
#define R_AC2 0xAC
#define R_AC3 0xAE
#define R_AC4 0xB0
#define R_AC5 0xB2
#define R_AC6 0xB4

#define R_B1  0xB6
#define R_B2  0xB8

#define R_MB  0xBA
#define R_MC  0xBC
#define R_MD  0xBE


typedef enum {
    BMP180_OK = 0,              /*!< Success */
	BMP180_ERR_I2C_READ  = 1,
    BMP180_ERR_I2C_WRITE = 2,
    BMP180_ERR_CALIB_AC1 = 10,  /*!< AC1 data read error*/
    BMP180_ERR_CALIB_AC2 = 11,  /*!< AC2 data read error */
    BMP180_ERR_CALIB_AC3 = 12,  /*!< AC3 data read error */
    BMP180_ERR_CALIB_AC4 = 13,  /*!< AC4 data read error */
    BMP180_ERR_CALIB_AC5 = 14,  /*!< AC5 data read error */
    BMP180_ERR_CALIB_AC6 = 15,  /*!< AC6 data read error */
    BMP180_ERR_CALIB_B1  = 16,  /*!< B1 data read error */
    BMP180_ERR_CALIB_B2  = 17,  /*!< B2 data read error */
    BMP180_ERR_CALIB_MB  = 18,  /*!< MB data read error */
    BMP180_ERR_CALIB_MC  = 19,  /*!< MC data read error */
    BMP180_ERR_CALIB_MD  = 20,  /*!< MD data read error */
    BMP180_ERR_CHIP_ID   = 30,  /*!< Chip ID error (sensor connections error) */
    BMP180_ERR_NULL_PTR  = 40   /*!< NULL error */
} BMP180_Error;



typedef struct{
	uint8_t code;
	char *message;
}BMP180_ERROR;


static const BMP180_ERROR bmp180_errors[]={
		{255,"Unknown"},
		{BMP180_OK,"Success"},
		{BMP180_ERR_I2C_READ, "I2C Read"},
		{BMP180_ERR_I2C_WRITE,"I2C Write"},
		{BMP180_ERR_CALIB_AC1,"AC1 read"},
		{BMP180_ERR_CALIB_AC2,"AC2 read"},
		{BMP180_ERR_CALIB_AC3,"AC3 read"},
		{BMP180_ERR_CALIB_AC4,"AC4 read"},
		{BMP180_ERR_CALIB_AC5,"AC5 read"},
		{BMP180_ERR_CALIB_AC6,"AC6 read"},
		{BMP180_ERR_CALIB_B1, "B1 read"},
		{BMP180_ERR_CALIB_B2, "B2 read"},
		{BMP180_ERR_CALIB_MB, "MB read"},
		{BMP180_ERR_CALIB_MC, "MC read"},
		{BMP180_ERR_CALIB_MD, "MD read"},
		{BMP180_ERR_CHIP_ID,  "Chip ID"},
		{BMP180_ERR_NULL_PTR, "NULL Ptr"},
};

/** Altitude lookup table between 300-1100hpa */
static const int16_t param_b[37]={9165,9054,8923,8773,8604,8418,8217,8001,7771,7528,7275,7011,6738,6457,6169,5875,5575,5270,4961,4649,4335,4018,3699,3378,3057,2735,2413,2091,1769,1447,1127,806,487,169,-146,-462,-776};


typedef struct{
	bmp180_read_byte  ReadByte;
	bmp180_write_byte WriteByte;
	delay_ms Delay_ms;

	int32_t B5;

	int16_t  AC1;
	int16_t  AC2;
	int16_t  AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;

	int16_t B1;
	int16_t B2;

	int16_t MB;
	int16_t MC;
	int16_t MD;

	int32_t temperature; /*!< Temperature in 0.1°C steps (e.g., 157 = 15.7°C) */
	int32_t pressure;    /*!<  Pressure in Pa */
	int32_t altitude;    /*!<  Altitude in cm */

}BMP180_INF;


uint8_t *Get_Error_Message(uint8_t error_code);

/** Initialize the BMP180 sensor
 ** Call this once before using the sensor
 ** bmp180_info: This function fills the structure with calibration data
 **/
uint8_t BMP180_Init(BMP180_INF * bmp180_inf);

/** Read data from the sensor into the BMP180_INF structure and calculate the altitude
 ** bmp180_inf: Structure that stores the data
 ** resolution: Sensor resolution (0–3)
 **/
int32_t BMP180_Read(BMP180_INF * bmp180_inf, uint8_t resolution);




#endif /* BMP180_H_ */
