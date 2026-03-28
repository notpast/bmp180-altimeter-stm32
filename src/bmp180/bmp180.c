/*
 ******************************************************************************
 File:     bmp180.c
 Info:     2026-03-18

 */

#include "bmp180.h"
#include "stm32f10x.h"

static uint8_t Read_Calibration_Data(BMP180_INF * bmp180_inf) {

	uint8_t buffer[2];

	if (bmp180_inf == NULL) {
		return BMP180_ERR_NULL_PTR;
	}

	// R_AC1
	if (bmp180_inf->ReadByte(R_AC1, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_AC1;
	}
	bmp180_inf->AC1 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_AC2
	if (bmp180_inf->ReadByte(R_AC2, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_AC2;
	}
	bmp180_inf->AC2 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_AC3
	if (bmp180_inf->ReadByte(R_AC3, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_AC3;
	}
	bmp180_inf->AC3 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_AC4
	if (bmp180_inf->ReadByte(R_AC4, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_AC4;
	}
	bmp180_inf->AC4 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_AC5
	if (bmp180_inf->ReadByte(R_AC5, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_AC5;
	}
	bmp180_inf->AC5 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_AC6
	if (bmp180_inf->ReadByte(R_AC6, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_AC6;
	}
	bmp180_inf->AC6 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_B1
	if (bmp180_inf->ReadByte(R_B1, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_B1;
	}
	bmp180_inf->B1 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_B2
	if (bmp180_inf->ReadByte(R_B2, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_B2;
	}
	bmp180_inf->B2 = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_MB
	if (bmp180_inf->ReadByte(R_MB, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_MB;
	}
	bmp180_inf->MB = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_MC
	if (bmp180_inf->ReadByte(R_MC, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_MC;
	}
	bmp180_inf->MC = (((uint16_t) buffer[0]) << 8) | buffer[1];

	// R_MD
	if (bmp180_inf->ReadByte(R_MD, buffer, 2) != 0) {
		return BMP180_ERR_CALIB_MD;
	}
	bmp180_inf->MD = (((uint16_t) buffer[0]) << 8) | buffer[1];

	bmp180_inf->B5 = 0;

	bmp180_inf->pressure    = 0;
	bmp180_inf->temperature = 0;
	bmp180_inf->altitude    = 0;

	return BMP180_OK;

}


uint8_t BMP180_Init(BMP180_INF * bmp180_inf) {
	if (bmp180_inf == NULL) {
		return BMP180_ERR_NULL_PTR;
	}
	if (bmp180_inf->ReadByte == NULL || bmp180_inf->WriteByte == NULL || bmp180_inf->Delay_ms == NULL) {
		return BMP180_ERR_NULL_PTR;
	}


    // Check chip ID
    uint8_t chip_id;
    if (bmp180_inf->ReadByte(0xD0, &chip_id, 1) != 0) {
        return BMP180_ERR_CHIP_ID;
    }
    if (chip_id != 0x55) {  // BMP180 chip ID
        return BMP180_ERR_CHIP_ID;
    }


	return Read_Calibration_Data(bmp180_inf);
}



static int32_t Get_Temperature(BMP180_INF * bmp180_inf) {

	int32_t temp_data;
	int32_t X1, X2, B5, T;
	uint8_t buffer[2];

	if (bmp180_inf == NULL) {
		return BMP180_ERR_NULL_PTR;
	}

	buffer[0] = 0x2E;
	if (bmp180_inf->WriteByte(0xF4, buffer, 1) != 0) {
		return BMP180_ERR_I2C_WRITE;
	}

	// wait 5ms
	bmp180_inf->Delay_ms(5);

	// read 2 bytes
	if (bmp180_inf->ReadByte(0xF6, buffer, 2) != 0) {
		return BMP180_ERR_I2C_READ;
	}
	temp_data = (((uint16_t) buffer[0]) << 8) | buffer[1];

	X1 = ((temp_data - bmp180_inf->AC6) * bmp180_inf->AC5) >> 15;
	X2 = (bmp180_inf->MC << 11) / (X1 + bmp180_inf->MD);
	B5 = X1 + X2;
	T = (B5 + 8) >> 4;

	bmp180_inf->B5 = B5;
	bmp180_inf->temperature = T;

	return BMP180_OK;
}


static int32_t Get_Pressure(BMP180_INF * bmp180_inf, uint8_t resolution) {

	uint8_t buffer[3];
	uint32_t oss = 3;
	uint8_t reg_value = 0x34;
	uint32_t UP;

	if (bmp180_inf == NULL) {
		return BMP180_ERR_NULL_PTR;
	}


	// Read temperature data
	int32_t error = Get_Temperature(bmp180_inf);
	if (error) return error;


	oss = resolution & 0X03;
	reg_value = reg_value + (oss << 6);

	buffer[0] = reg_value;
	if (bmp180_inf->WriteByte(0xF4, buffer, 1) != 0) {
		return BMP180_ERR_I2C_WRITE;
	}

	uint16_t delay_time = 5;
	if (oss == 0) {
		delay_time = 5;
	} else if (oss == 1) {
		delay_time = 8;
	} else if (oss == 2) {
		delay_time = 14;
	} else if (oss == 3) {
		delay_time = 26;
	}

	//Delay 4.5ms we use 5ms
	bmp180_inf->Delay_ms(delay_time);

	//msb 0xF6, lsb 0xF7

	if (bmp180_inf->ReadByte(0xF6, buffer, 3) != 0) {
		return BMP180_ERR_I2C_READ;
	}
	UP = (((int32_t) buffer[0]) << 16) | (((int32_t) buffer[1]) << 8) | (int32_t) buffer[2];

	UP = UP >> (8 - oss);

	int32_t B1, B2, B5, B3, B6, X1, X2, X3, p;
	uint32_t B4, B7;

	B5 = bmp180_inf->B5;
	B1 = bmp180_inf->B1;
	B2 = bmp180_inf->B2;

	B6 = B5 - 4000;
	X1 = (B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = (bmp180_inf->AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = (((bmp180_inf->AC1 * 4 + X3) << oss) + 2) >> 2;
	X1 = (bmp180_inf->AC3 * B6) >> 13;
	X2 = ((int32_t) B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = (bmp180_inf->AC4 * (uint32_t) (X3 + 32768)) >> 15;
	B7 = ((uint32_t) UP - B3) * (50000 >> oss);

	p = 0;
	if (B7 < 0X80000000) {
		p = (B7 * 2) / B4;
	} else {
		p = (B7 / B4) * 2;
	}

	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;
	p = p + ((X1 + X2 + 3791) >> 4);


	bmp180_inf->pressure = p;

	return BMP180_OK;
}



static int32_t Get_Altitude(BMP180_INF * bmp180_inf) {

	int32_t step = 300;
	int32_t k = 5;

	int32_t p1 = 300;
	int32_t p2 = 300;
	int32_t index = 0;

	int32_t p;

	if (bmp180_inf == NULL) {
		return BMP180_ERR_NULL_PTR;
	}

	int32_t pressure = bmp180_inf->pressure;

	// Filter sensor data
	if(pressure<30000){
		pressure=30000;
	}
	if(pressure>110000){
		pressure=110000;
	}

	// Convert Pa to hPa
	p = pressure/100;

	for (int32_t i = 0; i < 36; i++) {
		if (p >= step) {
			p1 = step;
			index = i;
		} else {
			p2 = step;
			break;
		}
		step += k;
		k++;
	}

	// Convert hpa to Pa
	p1*=100;
	p2*=100;


	int32_t a = param_b[index];
	int32_t b = param_b[index + 1];

	int32_t altitude = 100 * a + (100 * (b - a) * (pressure - p1)) /  (p2 - p1);

	bmp180_inf->altitude = altitude;

	return BMP180_OK;
}

int32_t BMP180_Read(BMP180_INF * bmp180_inf, uint8_t resolution) {

	int32_t error;

	if (bmp180_inf == NULL) {
		return 20;
	}

	resolution = resolution & 0x03;

	// Read pressure and temperature data
	error = Get_Pressure(bmp180_inf, resolution);
	if (error)
		return error;

	// Calculate altitude
	error = Get_Altitude(bmp180_inf);
	if (error)
		return error;

	return BMP180_OK;

}




uint8_t *Get_Error_Message(uint8_t error_code){
	uint8_t i;
	uint8_t len=17;

	for(i=0;i<len;i++){
		if(bmp180_errors[i].code==error_code){
			return bmp180_errors[i].message;
		}
	}

	return bmp180_errors[0].message;

}

