/**
 ******************************************************************************
 * @file    uvis25.c
 * @author  William Xu
 * @version V1.0.0
 * @date    21-May-2015
 * @brief
 ******************************************************************************
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "uvis25.h"

#define uvis25_log(M, ...) custom_log("UVIS25", M, ##__VA_ARGS__)
#define uvis25_log_trace() custom_log_trace("UVIS25")

static merr_t UVIS25_Init(void);
static merr_t UVIS25_GetUXindex(float *pfData);

/* I2C device */
mxos_i2c_device_t uvis25_i2c_device = {
  UVIS25_I2C_PORT, 0x47, I2C_ADDRESS_WIDTH_7BIT, I2C_STANDARD_SPEED_MODE
};

merr_t UVIS25_IO_Init(void)
{
  // I2C init
  mxos_i2c_deinit(&uvis25_i2c_device);   // in case error
  mxos_i2c_init(&uvis25_i2c_device);

  if( false == mxos_i2c_probe_dev(&uvis25_i2c_device, 5) ){
    uvis25_log("UVI25S_ERROR: no i2c device found!");
    return kNotInitializedErr;
  }
  return kNoErr;
}


/*	\Brief: The function is used as I2C bus write
*	\Return : Status of the I2C write
*	\param dev_addr : The device address of the sensor
*	\param reg_addr : Address of the first register, will data is going to be written
*	\param reg_data : It is a value hold in the array,
*		will be used for write the value into the register
*	\param cnt : The no of byte of data to be write
*/
merr_t UVIS25_IO_Write(uint8_t* pBuffer, uint8_t RegisterAddr, uint16_t NumByteToWrite)
{
  mxos_i2c_message_t uvis25_i2c_msg = {NULL, NULL, 0, 0, 0, false};
  merr_t iError = kNoErr;
  uint8_t array[8];
  uint8_t stringpos;
  array[0] = RegisterAddr;
  for (stringpos = 0; stringpos < NumByteToWrite; stringpos++) {
    array[stringpos + 1] = *(pBuffer + stringpos);
  }
  
  iError = mxos_i2c_build_tx_msg(&uvis25_i2c_msg, array, NumByteToWrite + 1, 3);
  iError = mxos_i2c_transfer(&uvis25_i2c_device, &uvis25_i2c_msg, 1);
  if(kNoErr != iError){
    iError = kWriteErr;
  }
  
  return kNoErr;
}

/*	\Brief: The function is used as I2C bus read
*	\Return : Status of the I2C read
*	\param dev_addr : The device address of the sensor
*	\param reg_addr : Address of the first register, will data is going to be read
*	\param reg_data : This data read from the sensor, which is hold in an array
*	\param cnt : The no of byte of data to be read
*/
merr_t UVIS25_IO_Read(uint8_t* pBuffer, uint8_t RegisterAddr, uint16_t NumByteToRead)
{
  mxos_i2c_message_t uvis25_i2c_msg = {NULL, NULL, 0, 0, 0, false};
  merr_t iError = kNoErr;
  uint8_t array[8] = {0};
  array[0] = RegisterAddr;
  
  iError = mxos_i2c_build_comb_msg(&uvis25_i2c_msg, array, pBuffer, 1, NumByteToRead, 3);
  if(kNoErr != iError){
    return kReadErr; 
  }
  iError = mxos_i2c_transfer(&uvis25_i2c_device, &uvis25_i2c_msg, 1);
  if(kNoErr != iError){
    return kReadErr;
  }
  return kNoErr;
}


static merr_t UVIS25_Init(void)
{
  merr_t err = kNoErr;
  uint8_t temp = 0x03;
  
  if((err = UVIS25_IO_Init()) != kNoErr){
    return err;
  }
  
  if((err = UVIS25_IO_Write(&temp, UVIS25_CTRL_REG1, 1)) != kNoErr){
    return err;
  }
  
  return err;
}


static merr_t UVIS25_GetUXindex(float *pfData) 
{
  merr_t err = kNoErr;
  uint8_t temp = 0x00;
  uint8_t data = 0;
  
  do{
    UVIS25_IO_Read(&temp, UVIS25_STATUS_REG, 1);
  }while( temp != 0x01 );
  
  if((err = UVIS25_IO_Read(&data, UVIS25_UV_OUT_REG, 1)) != kNoErr){
    return err;
  }
  
  *pfData = (float)data/16;
  
  return err;
}

merr_t uvis25_sensor_init(void)
{ 
  return UVIS25_Init();
}

merr_t uvis25_Read_Data(float *uv_index)
{
  return UVIS25_GetUXindex(uv_index);
}

merr_t uvis25_sensor_deinit(void)
{
  return mxos_i2c_deinit(&uvis25_i2c_device);
}

