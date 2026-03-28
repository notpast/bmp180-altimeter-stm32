
#include "i2c.h"





uint8_t WriteByte(I2C_TypeDef * port,uint8_t i2c_address,uint8_t reg,uint8_t * buffer,uint8_t len){

	volatile int32_t t=0;

    // BUSY
    t = 0;
    while(I2C_GetFlagStatus(port, I2C_FLAG_BUSY)){
        if(t++ > 100000) return 10;
    }


	I2C_GenerateSTART(port, ENABLE);


	t=0;
    while(!I2C_CheckEvent(port, I2C_EVENT_MASTER_MODE_SELECT))
    {
        t++;
        if(t>100000){
        	I2C_GenerateSTOP(port, ENABLE);
      	  return 1;
        }
    }


    I2C_Send7bitAddress(port, i2c_address, I2C_Direction_Transmitter);

    t=0;
    while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
      t++;
      if(t>100000){
    	  I2C_GenerateSTOP(port, ENABLE);
    	  return 2;
      }
    }

    I2C_SendData(port, reg);

    t=0;
    while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        t++;
        if(t>100000){
        	I2C_GenerateSTOP(port, ENABLE);
      	  return 3;
        }
    }


    for(uint8_t i=0;i<len;i++){
		I2C_SendData(port, buffer[i]);
		t=0;
		while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
			t++;
			if(t>100000){
				I2C_GenerateSTOP(port, ENABLE);
			  return 4;
			}
		}
    }

    I2C_GenerateSTOP(port, ENABLE);

    return 0;
}


uint8_t ReadByte(I2C_TypeDef * port,uint8_t i2c_address,uint8_t reg,uint8_t * buffer,uint8_t len){
	volatile int32_t t=0;

	if(len==0){
		return 0;
	}

    // BUSY
    t = 0;
    while(I2C_GetFlagStatus(port, I2C_FLAG_BUSY)){
        if(t++ > 100000) return 10;
    }

	I2C_AcknowledgeConfig(port,ENABLE);

//Verinin adresi gönderiliyor.
	I2C_GenerateSTART(port, ENABLE);
	t=0;
    while(!I2C_CheckEvent(port, I2C_EVENT_MASTER_MODE_SELECT))
    {
        t++;
        if(t>100000){
        	I2C_AcknowledgeConfig(port,DISABLE);
        	I2C_GenerateSTOP(port, ENABLE);
      	  return 1;
        }
    }


    I2C_Send7bitAddress(port, i2c_address, I2C_Direction_Transmitter);
    t=0;
    while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
      t++;
      if(t>100000){
    	  I2C_AcknowledgeConfig(port,DISABLE);
    	  I2C_GenerateSTOP(port, ENABLE);
    	  return 2;
      }
    }

    I2C_SendData(port, reg);
    t=0;
    while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_BYTE_TRANSMITTED)){
        t++;
        if(t>100000){
        	I2C_AcknowledgeConfig(port,DISABLE);
        	I2C_GenerateSTOP(port, ENABLE);
      	  return 3;
        }
    }


	I2C_GenerateSTART(port, ENABLE);
	t=0;
    while(!I2C_CheckEvent(port, I2C_EVENT_MASTER_MODE_SELECT))
    {
        t++;
        if(t>100000){
        	I2C_AcknowledgeConfig(port,DISABLE);
        	I2C_GenerateSTOP(port, ENABLE);
      	  return 4;
        }
    }

    I2C_Send7bitAddress(port, i2c_address, I2C_Direction_Receiver);
    t=0;
    while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        t++;
        if(t>100000){
        	I2C_AcknowledgeConfig(port,DISABLE);
        	I2C_GenerateSTOP(port, ENABLE);
      	  return 5;
        }
    }

    for(uint8_t i=0;i<len;i++){
    	t=0;

        if(i == len - 1){
            // Son byte
            I2C_AcknowledgeConfig(port, DISABLE);
            I2C_GenerateSTOP(port, ENABLE);
        }

		while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_BYTE_RECEIVED)){
			t++;
			if(t>100000){
				I2C_GenerateSTOP(port, ENABLE);
				I2C_AcknowledgeConfig(port,DISABLE);
			  return 6;
			}
		}
		buffer[i]=I2C_ReceiveData(port);
    }

    I2C_AcknowledgeConfig(port, DISABLE);
    I2C_GenerateSTOP(port, ENABLE);

    return 0;

}


uint8_t Check_I2C_Port(I2C_TypeDef * port,uint8_t i2c_address){
	volatile uint32_t t=0;

    // BUSY
    t = 0;
    while(I2C_GetFlagStatus(port, I2C_FLAG_BUSY)){
        if(t++ > 100000) return 10;
    }


	I2C_GenerateSTART(port, ENABLE);
	t=0;
    while(!I2C_CheckEvent(port, I2C_EVENT_MASTER_MODE_SELECT))
    {
        t++;
        if(t>100000){
        	I2C_GenerateSTOP(port, ENABLE);
        	I2C_AcknowledgeConfig(port,DISABLE);
      	  return 1;
        }
    }

    I2C_Send7bitAddress(port, i2c_address, I2C_Direction_Transmitter);
    t=0;
    while (!I2C_CheckEvent(port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
      t++;
      if(t>100000){
    	  I2C_GenerateSTOP(port, ENABLE);
    	  return 2;
      }
    }

    I2C_GenerateSTOP(port, ENABLE);

    return 0;
}




