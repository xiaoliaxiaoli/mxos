/**
******************************************************************************
* @file    bmg160_user.c 
* @author  William Xu
* @version V1.0.0
* @date    21-May-2015
* @brief   bmg160 sensor control demo.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/
/*--------------------------------------------------------------------------*/
/* Includes*/
/*--------------------------------------------------------------------------*/
#include "bmg160.h"
#include "bmg160_user.h"

#define bmg160_user_log(M, ...) custom_log("BMG160_USER", M, ##__VA_ARGS__)
#define bmg160_user_log_trace() custom_log_trace("BMG160_USER")

#define BMG160_API

/* I2C device */
mxos_i2c_device_t bmg160_i2c_device = {
  BMG160_I2C_DEVICE, BMG160_I2C_ADDR1, I2C_ADDRESS_WIDTH_7BIT, I2C_STANDARD_SPEED_MODE
};

/*---------------------------------------------------------------------------*
*  The following functions are used for reading and writing of
*	sensor data using I2C or SPI communication
*---------------------------------------------------------------------------*/
#ifdef BMG160_API
/*	\Brief: The function is used as I2C bus read
 *	\Return : Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of byte of data to be read
 */
s8 BMG160_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
 /*	\Brief: The function is used as I2C bus write
 *	\Return : Status of the I2C write
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */
s8 BMG160_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
/*	\Brief: The function is used as SPI bus write
 *	\Return : Status of the SPI write
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */
s8 BMG160_SPI_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
/*	\Brief: The function is used as SPI bus read
 *	\Return : Status of the SPI read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of byte of data to be read */
s8 BMG160_SPI_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
/*
 * \Brief: SPI/I2C init routine
*/
s8 BMG160_I2C_routine(void);
s8 BMG160_SPI_routine(void);
#endif
/********************End of I2C/SPI function declarations***********************/
/*	Brief : The delay routine
 *	\param : delay in ms
*/
void BMG160_delay_msek(u32 msek);
/* This function is an example for reading sensor data
 *	\param: None
 *	\return: communication result
 */
s32 bmg160_data_readout_template(void);
/*---------------------------------------------------------------------------*
*  struct bmg160_t parameters can be accessed by using bmg160
 *	bmg160_t having the following parameters
 *	Bus write function pointer: BMG160_WR_FUNC_PTR
 *	Bus read function pointer: BMG160_RD_FUNC_PTR
 *	Burst read function pointer: BMG160_BRD_FUNC_PTR
 *	Delay function pointer: delay_msec
 *	I2C address: dev_addr
 *	Chip id of the sensor: chip_id
*-------------------------------------------------------------------------*/
struct bmg160_t bmg160;

/* This function is an example for reading sensor data
 *	\param: None
 *	\return: communication result
 */
s32 bmg160_data_readout_template(void)
{
	/* Gyro */
	/* variable used for read the sensor data*/
	s16	v_gyro_datax_s16, v_gyro_datay_s16, v_gyro_dataz_s16 = BMG160_INIT_VALUE;
	/* structure used for read the sensor data - xyz*/
	struct bmg160_data_t data_gyro;
	/* structure used for read the sensor data - xyz and interrupt status*/
	struct bmg160_data_t gyro_xyzi_data;
	/* variable used for read the gyro bandwidth data*/
	u8	v_gyro_value_u8 = BMG160_INIT_VALUE;
	/* variable used for set the gyro bandwidth data*/
	u8 v_bw_u8 = BMG160_INIT_VALUE;
	/* result of communication results*/
	s32 com_rslt = 0;

/*-------------------------------------------------------------------------*
 *********************** START INITIALIZATION ***********************
 *-------------------------------------------------------------------------*/
 /*	Based on the user need configure I2C or SPI interface.
  *	It is example code to explain how to use the bmg160 API*/
  #ifdef BMG160_API
  	BMG160_I2C_routine();
	/*BMG160_SPI_routine(); */
	#endif
/*--------------------------------------------------------------------------*
 *  This function used to assign the value/reference of
 *	the following parameters
 *	Gyro I2C address
 *	Bus Write
 *	Bus read
 *	Gyro Chip id
 *----------------------------------------------------------------------------*/
	com_rslt = bmg160_init(&bmg160);
/*----------------------------------------------------------------------------*/
/*	For initialization it is required to set the mode of the sensor as "NORMAL"
 *	data acquisition/read/write is possible in this mode
 *	by using the below API able to set the power mode as NORMAL
 *	NORMAL mode set from the register 0x11 and 0x12
 *	While sensor in the NORMAL mode idle time of at least 2us(micro seconds)
 *	is required to write/read operations
 *	0x11 -> bit 5,7 -> set value as BMG160_INIT_VALUE
 *	0x12 -> bit 6,7 -> set value as BMG160_INIT_VALUE
 *	Note:
 *		If the sensor is in the fast power up mode idle time of least
 *		450us(micro seconds) required for write/read operations
 */

/*-------------------------------------------------------------------------*/
	/* Set the gyro power mode as NORMAL*/
	com_rslt += bmg160_set_power_mode(BMG160_MODE_NORMAL);
/*--------------------------------------------------------------------------*
************************* END INITIALIZATION ******************************
*--------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*
************************* START GET and SET FUNCTIONS DATA ***************
*--------------------------------------------------------------------------*/
/* This API used to Write the bandwidth of the gyro sensor
	input value have to be give 0x10 bit BMG160_INIT_VALUE to 3
	The bandwidth set from the register */
	v_bw_u8 = C_BMG160_BW_230HZ_U8X;/* set gyro bandwidth of 230Hz*/
	com_rslt += bmg160_set_bw(v_bw_u8);

/* This API used to read back the written value of bandwidth for gyro*/
	com_rslt += bmg160_get_bw(&v_gyro_value_u8);
/*---------------------------------------------------------------------*
************************* END GET and SET FUNCTIONS ********************
*----------------------------------------------------------------------*/
/*---------------------------------------------------------------------*
************************* START READ SENSOR DATA(X,Y and Z axis) *********
*-------------------------------------------------------------------------*/
/******************* Read Gyro data xyz**********************/
	com_rslt += bmg160_get_data_X(&v_gyro_datax_s16);/* Read the gyro X data*/

	com_rslt += bmg160_get_data_Y(&v_gyro_datay_s16);/* Read the gyro Y data*/

	com_rslt += bmg160_get_data_Z(&v_gyro_dataz_s16);/* Read the gyro Z data*/

/* accessing the  bmg160_data_t parameter by using data_gyro*/
	com_rslt += bmg160_get_data_XYZ(&data_gyro);/* Read the gyro XYZ data*/

/* accessing the bmg160_data_t parameter by using gyro_xyzi_data*/
/* Read the gyro XYZ data and interrupt status*/
	com_rslt += bmg160_get_data_XYZI(&gyro_xyzi_data);
/*--------------------------------------------------------------------------
************************* END READ SENSOR DATA(X,Y and Z axis) *************
*----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
*********************** START DE-INITIALIZATION *****************************
*--------------------------------------------------------------------------*/
/*	For de-initialization it is required to set the mode of
 *	the sensor as "DEEPSUSPEND"
 *	the device reaches the lowest power consumption only
 *	interface selection is kept alive
 *	No data acquisition is performed
 *	The DEEPSUSPEND mode set from the register 0x11 bit 5
 *	by using the below API able to set the power mode as DEEPSUSPEND
 *	For the read/ write operation it is required to provide least 450us
 *	micro second delay*/

	com_rslt += bmg160_set_power_mode(BMG160_MODE_DEEPSUSPEND);

/*--------------------------------------------------------------------------*
*********************** END DE-INITIALIZATION **************************
*---------------------------------------------------------------------------*/
return com_rslt;
}

#ifdef BMG160_API
/*--------------------------------------------------------------------------*
*	The following function is used to map the I2C bus read, write, delay and
*	device address with global structure bmg160_t
*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*
 *  By using bmg160 the following structure parameter can be accessed
 *	Bus write function pointer: BMG160_WR_FUNC_PTR
 *	Bus read function pointer: BMG160_RD_FUNC_PTR
 *	Delay function pointer: delay_msec
 *	I2C address: dev_addr
 *--------------------------------------------------------------------------*/
 s8 BMG160_I2C_routine(void) {

	bmg160.bus_write = BMG160_I2C_bus_write;
	bmg160.bus_read = BMG160_I2C_bus_read;
	bmg160.delay_msec = BMG160_delay_msek;
	bmg160.dev_addr = BMG160_I2C_ADDR1;

	return BMG160_INIT_VALUE;
}

/*---------------------------------------------------------------------------*
 *	The following function is used to map the SPI bus read, write and delay
 *	with global structure bmg160_t
 *--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*
 *  By using bmg160 the following structure parameter can be accessed
 *	Bus write function pointer: BMG160_WR_FUNC_PTR
 *	Bus read function pointer: BMG160_RD_FUNC_PTR
 *	Delay function pointer: delay_msec
 *-------------------------------------------------------------------------*/
//s8 BMG160_SPI_routine(void) {

//	bmg160.bus_write = BMG160_SPI_bus_write;
//	bmg160.bus_read = BMG160_SPI_bus_read;
//	bmg160.delay_msec = BMG160_delay_msek;

//	return BMG160_INIT_VALUE;
//}

/************** I2C/SPI buffer length ******/
#define	I2C_BUFFER_LEN 8
#define SPI_BUFFER_LEN 5
#define MASK_DATA1	0xFF
#define MASK_DATA2	0x80
#define MASK_DATA3	0x7F
/*-------------------------------------------------------------------*
*
*	This is a sample code for read and write the data by using I2C/SPI
*	Use either I2C or SPI based on your need
*	The device address defined in the bmg160.h file
*
*-----------------------------------------------------------------------*/
 /*	\Brief: The function is used as I2C bus write
 *	\Return : Status of the I2C write
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be written
 *	\param reg_data : It is a value hold in the array,
 *		will be used for write the value into the register
 *	\param cnt : The no of byte of data to be write
 */
s8 BMG160_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
        mxos_i2c_message_t bmg160_i2c_msg = {NULL, NULL, 0, 0, 0, false};
	s32 iError = BMG160_INIT_VALUE;
	u8 array[I2C_BUFFER_LEN];
	u8 stringpos = BMG160_INIT_VALUE;
	array[BMG160_INIT_VALUE] = reg_addr;
	for (stringpos = BMG160_INIT_VALUE; stringpos < cnt; stringpos++) {
		array[stringpos + BMG160_GEN_READ_WRITE_DATA_LENGTH] = *(reg_data + stringpos);
	}
	/*
	* Please take the below function as your reference for
	* write the data using I2C communication
	* "IERROR = I2C_WRITE_STRING(DEV_ADDR, ARRAY, CNT+1)"
	* add your I2C write function here
	* iError is an return value of I2C read function
	* Please select your valid return value
	* In the driver SUCCESS defined as BMG160_INIT_VALUE
    * and FAILURE defined as -1
	* Note :
	* This is a full duplex operation,
	* The first read data is discarded, for that extra write operation
	* have to be initiated. For that cnt+1 operation done in the I2C write string function
	* For more information please refer data sheet SPI communication:
	*/
        iError = mxos_i2c_build_tx_msg(&bmg160_i2c_msg, array, cnt + 1, 3);
        iError = mxos_i2c_transfer(&bmg160_i2c_device, &bmg160_i2c_msg, 1);
        if(0 != iError){
          iError = -1;
        }
	return (s8)iError;
}

 /*	\Brief: The function is used as I2C bus read
 *	\Return : Status of the I2C read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of data to be read
 */
s8 BMG160_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
{
        mxos_i2c_message_t bmg160_i2c_msg = {NULL, NULL, 0, 0, 0, false};
	s32 iError = BMG160_INIT_VALUE;
	u8 array[I2C_BUFFER_LEN] = {BMG160_INIT_VALUE};
//	u8 stringpos = BMG160_INIT_VALUE;
	array[BMG160_INIT_VALUE] = reg_addr;
	/* Please take the below function as your reference
	 * for read the data using I2C communication
	 * add your I2C rad function here.
	 * "IERROR = I2C_WRITE_READ_STRING(DEV_ADDR, ARRAY, ARRAY, 1, CNT)"
	 * iError is an return value of SPI write function
	 * Please select your valid return value
	 * In the driver SUCCESS defined as BMG160_INIT_VALUE
     * and FAILURE defined as -1
	 */
        
        iError = mxos_i2c_build_comb_msg(&bmg160_i2c_msg, array, reg_data, 1, cnt, 3);
         if(0 != iError){
          return (s8)iError; 
        }
        iError = mxos_i2c_transfer(&bmg160_i2c_device, &bmg160_i2c_msg, 1);
        if(0 != iError){
          return (s8)iError;
        }
        
//	for (stringpos = BMG160_INIT_VALUE; stringpos < cnt; stringpos++) {
//		*(reg_data + stringpos) = array[stringpos];
//	}
	return (s8)iError;
}

/*	\Brief: The function is used as SPI bus read
 *	\Return : Status of the SPI read
 *	\param dev_addr : The device address of the sensor
 *	\param reg_addr : Address of the first register, will data is going to be read
 *	\param reg_data : This data read from the sensor, which is hold in an array
 *	\param cnt : The no of data to be read
 */
//s8 BMG160_SPI_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
//{
//	s32 iError=BMG160_INIT_VALUE;
//	u8 array[SPI_BUFFER_LEN]={MASK_DATA1};
//	u8 stringpos;
//	/*	For the SPI mode only 7 bits of register addresses are used.
//	The MSB of register address is declared the bit what functionality it is
//	read/write (read as 1/write as BMG160_INIT_VALUE)*/
//	array[BMG160_INIT_VALUE] = reg_addr|MASK_DATA2;/*read routine is initiated register address is mask with 0x80*/
//	/*
//	* Please take the below function as your reference for
//	* read the data using SPI communication
//	* " IERROR = SPI_READ_WRITE_STRING(ARRAY, ARRAY, CNT+1)"
//	* add your SPI read function here
//	* iError is an return value of SPI read function
//	* Please select your valid return value
//	* In the driver SUCCESS defined as BMG160_INIT_VALUE
//    * and FAILURE defined as -1
//	* Note :
//	* This is a full duplex operation,
//	* The first read data is discarded, for that extra write operation
//	* have to be initiated. For that cnt+1 operation done in the SPI read
//	* and write string function
//	* For more information please refer data sheet SPI communication:
//	*/
//	for (stringpos = BMG160_INIT_VALUE; stringpos < cnt; stringpos++) {
//		*(reg_data + stringpos) = array[stringpos+BMG160_GEN_READ_WRITE_DATA_LENGTH];
//	}
//	return (s8)iError;
//}
//
///*	\Brief: The function is used as SPI bus write
// *	\Return : Status of the SPI write
// *	\param dev_addr : The device address of the sensor
// *	\param reg_addr : Address of the first register, will data is going to be written
// *	\param reg_data : It is a value hold in the array,
// *		will be used for write the value into the register
// *	\param cnt : The no of byte of data to be write
// */
//s8 BMG160_SPI_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
//{
//	s32 iError = BMG160_INIT_VALUE;
//	u8 array[SPI_BUFFER_LEN * C_BMG160_TWO_U8X];
//	u8 stringpos = BMG160_INIT_VALUE;
//	for (stringpos = BMG160_INIT_VALUE; stringpos < cnt; stringpos++) {
//		/* the operation of (reg_addr++)&0x7F done: because it ensure the
//		   BMG160_INIT_VALUE and 1 of the given value
//		   It is done only for 8bit operation*/
//		array[stringpos * C_BMG160_TWO_U8X] = (reg_addr++) & MASK_DATA3;
//		array[stringpos * C_BMG160_TWO_U8X + BMG160_GEN_READ_WRITE_DATA_LENGTH] = *(reg_data + stringpos);
//	}
//	/* Please take the below function as your reference
//	 * for write the data using SPI communication
//	 * add your SPI write function here.
//	 * "IERROR = SPI_WRITE_STRING(ARRAY, CNT*2)"
//	 * iError is an return value of SPI write function
//	 * Please select your valid return value
//	 * In the driver SUCCESS defined as BMG160_INIT_VALUE
//     * and FAILURE defined as -1
//	 */
//	return (s8)iError;
//}

/*	Brief : The delay routine
 *	\param : delay in ms
*/
void BMG160_delay_msek(u32 msek)
{
	/*Here you can write your own delay routine*/
  mxos_thread_msleep(msek);
}
#endif


OSStatus bmg160_sensor_init(void)
{
  OSStatus err = kUnknownErr;
  /* variable used for read the gyro bandwidth data*/
  u8 v_gyro_value_u8 = BMG160_INIT_VALUE;
  /* variable used for set the gyro bandwidth data*/
  u8 v_bw_u8 = BMG160_INIT_VALUE;
  s32 com_rslt = BMG160_ERROR;  // result of communication results
 // u8 v_stand_by_time_u8 = BME280_INIT_VALUE;  //  The variable used to assign the standby time
  
  // I2C init
  mxos_i2c_deinit(&bmg160_i2c_device);   // in case error
  err = mxos_i2c_init(&bmg160_i2c_device);
  require_noerr_action( err, exit, bmg160_user_log("BMG160_ERROR: mxos_i2c_init err = %d.", err) );
  if( false == mxos_i2c_probe_dev(&bmg160_i2c_device, 5) ){
    bmg160_user_log("BMG160_ERROR: no i2c device found!");
    err = kNotFoundErr;
    goto exit;
  }
  
  // sensor init

  /*********************** START INITIALIZATION ************************/
  /*	Based on the user need configure I2C or SPI interface.
  *	It is example code to explain how to use the bme280 API*/
#ifdef BMG160_API
  BMG160_I2C_routine();
  com_rslt = bmg160_init(&bmg160);
  com_rslt += bmg160_set_power_mode(BMG160_MODE_NORMAL);
  /*------------------------------------------------------------------------*
  ************************* START GET and SET FUNCTIONS DATA ***************
  *--------------------------------------------------------------------------*/
  /* This API used to Write the bandwidth of the gyro sensor
  input value have to be give 0x10 bit BMG160_INIT_VALUE to 3
  The bandwidth set from the register */
  v_bw_u8 = C_BMG160_BW_230HZ_U8X;/* set gyro bandwidth of 230Hz*/
  com_rslt += bmg160_set_bw(v_bw_u8);

  /* This API used to read back the written value of bandwidth for gyro*/
  com_rslt += bmg160_get_bw(&v_gyro_value_u8);
  /*---------------------------------------------------------------------*
  ************************* END GET and SET FUNCTIONS ********************
  *----------------------------------------------------------------------*/
  if(com_rslt < 0){
    bmg160_user_log("BMG160_ERROR: bme280 sensor init failed!");
    err = kNotInitializedErr;
    goto exit;
  }
  /************************* END INITIALIZATION *************************/
#endif
  return kNoErr;
  
exit:
  return err;
}


OSStatus bmg160_data_readout(s16 *v_gyro_datax_s16, s16 *v_gyro_datay_s16, s16 *v_gyro_dataz_s16)
{
  OSStatus err = kUnknownErr;
  
  /* result of communication results*/
  s32 com_rslt = BMG160_ERROR;
  
  //-------------------------- NOTE ----------------------------------
  // this is to avoid i2c pin is re-init by other module because they use the same pin.
  mxos_i2c_init(&bmg160_i2c_device);
  //------------------------------------------------------------------
    
  /************ START READ TRUE PRESSURE, TEMPERATURE AND HUMIDITY DATA *********/

  com_rslt = bmg160_get_data_X(v_gyro_datax_s16);/* Read the gyro X data*/

  com_rslt += bmg160_get_data_Y(v_gyro_datay_s16);/* Read the gyro Y data*/

  com_rslt += bmg160_get_data_Z(v_gyro_dataz_s16);/* Read the gyro Z data*/
  
  /************ END READ TRUE PRESSURE, TEMPERATURE AND HUMIDITY ********/
  
  if(0 == com_rslt){
    err = kNoErr;
  }
  return err;
}

OSStatus bmg160_sensor_deinit(void)
{
  OSStatus err = kUnknownErr;
  s32 com_rslt = BMG160_ERROR;
  
  err = mxos_i2c_deinit(&bmg160_i2c_device);
  require_noerr_action( err, exit, bmg160_user_log("BMG160_ERROR: mxos_i2c_deinit err = %d.", err));
  
/*---------------------------------------------------------------------------*
*********************** START DE-INITIALIZATION *****************************
*--------------------------------------------------------------------------*/
/*	For de-initialization it is required to set the mode of
 *	the sensor as "DEEPSUSPEND"
 *	the device reaches the lowest power consumption only
 *	interface selection is kept alive
 *	No data acquisition is performed
 *	The DEEPSUSPEND mode set from the register 0x11 bit 5
 *	by using the below API able to set the power mode as DEEPSUSPEND
 *	For the read/ write operation it is required to provide least 450us
 *	micro second delay*/

	com_rslt = bmg160_set_power_mode(BMG160_MODE_DEEPSUSPEND);

/*--------------------------------------------------------------------------*
*********************** END DE-INITIALIZATION **************************
*---------------------------------------------------------------------------*/

  if(0 == com_rslt){
    err = kNoErr;
  }
  
exit:
  return err;
}











