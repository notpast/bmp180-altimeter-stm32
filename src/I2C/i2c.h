/*
 * i2c.h
 *
 *  Created on: Mar 18, 2026
 *      Author: nebula
 */

#ifndef I2C_I2C_H_
#define I2C_I2C_H_

#include "stm32f10x.h"

uint8_t WriteByte(I2C_TypeDef * port,uint8_t i2c_address,uint8_t reg,uint8_t * buffer,uint8_t len);
uint8_t ReadByte(I2C_TypeDef * port,uint8_t i2c_address,uint8_t reg,uint8_t * buffer,uint8_t len);

uint8_t Check_I2C_Port(I2C_TypeDef * port,uint8_t i2c_address);


#endif /* I2C_I2C_H_ */
