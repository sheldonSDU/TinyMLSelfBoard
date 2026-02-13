/**
  ******************************************************************************
  * @file    ov5640_dcmi_drv.c
  * @brief   This file contains the OV5640 camera driver using DCMI interface.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ov5640_dcmi_drv.h"
#include "ov5640.h"
#include "ov5640_reg.h"
#include "main.h"  // for hi2c2, hdcmi, hdma_dcmi handles
#include <stdio.h>

/* Private defindes -----------------------------------------------------------*/
#define CAM_SENSOR_I2C_ADDR_8BIT        0x78 /* OV5640 8-bit address */
#define CAM_SENSOR_I2C_ADDR_7BIT        0x3C /* OV5640 7-bit address */
/* Private variables ---------------------------------------------------------*/
static uint32_t IsCameraInitialized = 0;
static cam_resolution_t CurrentResolution;
static cam_pixformat_t CurrentPixelFormat;

/* Camera object and IO structure */
static OV5640_Object_t CameraObj;
static OV5640_IO_t CameraIO;

/* Private function prototypes -----------------------------------------------*/
static int32_t CAM_I2C_Write(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t CAM_I2C_Read(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t MapResolution(cam_resolution_t res, uint32_t *ov5640_res);
static int32_t MapPixelFormat(cam_pixformat_t pf, uint32_t *ov5640_pf);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Write data to the camera register via I2C
  * @param  DevAddr: Device address on communication Bus
  * @param  Reg: Camera register address to write
  * @param  pData: Data pointer to write
  * @param  Length: Number of data to write
  * @retval 0 in case of success, -1 in case of failure
  */
static int32_t CAM_I2C_Write(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  // Use HAL_I2C_Mem_Write which handles the register address + data write sequence properly
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c2, DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, 100);
  
  if (status != HAL_OK)
  {
    return -1;
  }

  return 0;
}

/**
  * @brief  Read data from the camera register via I2C
  * @param  DevAddr: Device address on communication Bus
  * @param  Reg: Camera register address to read
  * @param  pData: Data pointer to store register value
  * @param  Length: Number of data to read
  * @retval 0 in case of success, -1 in case of failure
  */
static int32_t CAM_I2C_Read(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  // Use HAL_I2C_Mem_Read which handles the register address + data read sequence properly
  HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, 100);
  
  if (status != HAL_OK)
  {
    return -1;
  }

  return 0;
}

/* IO interface adapter functions */
static int32_t CAM_IO_Init(void)
{
  return 0; /* I2C already initialized */
}

static int32_t CAM_IO_DeInit(void)
{
  return 0;
}

static int32_t CAM_IO_WriteReg(uint16_t Addr, uint16_t Reg, uint8_t *Buffer, uint16_t Length)
{
  return CAM_I2C_Write(Addr, Reg, Buffer, Length);
}

static int32_t CAM_IO_ReadReg(uint16_t Addr, uint16_t Reg, uint8_t *Buffer, uint16_t Length)
{
  return CAM_I2C_Read(Addr, Reg, Buffer, Length);
}

static int32_t CAM_IO_GetTick(void)
{
  return (int32_t)HAL_GetTick();
}

/**
  * @brief  Map user resolution to OV5640 resolution
  * @param  res: User resolution enum
  * @param  ov5640_res: Pointer to store OV5640 resolution value
  * @retval CAM_OK in case of success, CAM_UNSUPPORTED in case of failure
  */
static int32_t MapResolution(cam_resolution_t res, uint32_t *ov5640_res)
{
  switch(res)
  {
    case CAM_R160x120:
      *ov5640_res = OV5640_R160x120;
      break;
    case CAM_R320x240:
      *ov5640_res = OV5640_R320x240;
      break;
    case CAM_R480x272:
      *ov5640_res = OV5640_R480x272;
      break;
    case CAM_R640x480:
      *ov5640_res = OV5640_R640x480;
      break;
    case CAM_R800x480:
      *ov5640_res = OV5640_R800x480;
      break;
    default:
      return CAM_UNSUPPORTED;
  }
  return CAM_OK;
}

/**
  * @brief  Map user pixel format to OV5640 pixel format
  * @param  pf: User pixel format enum
  * @param  ov5640_pf: Pointer to store OV5640 pixel format value
  * @retval CAM_OK in case of success, CAM_UNSUPPORTED in case of failure
  */
static int32_t MapPixelFormat(cam_pixformat_t pf, uint32_t *ov5640_pf)
{
  switch(pf)
  {
    case CAM_PF_RGB565:
      *ov5640_pf = OV5640_RGB565;
      break;
    case CAM_PF_RGB888:
      *ov5640_pf = OV5640_RGB888;
      break;
    case CAM_PF_YUV422:
      *ov5640_pf = OV5640_YUV422;
      break;
    default:
      return CAM_UNSUPPORTED;
  }
  return CAM_OK;
}

/**
  * @brief  Initialize sensor (I2C) + configure sensor output format/resolution.
  *         Note: DCMI and I2C must already be initialized (you said you did).
  * @param  res: Camera resolution
  * @param  pf: Camera pixel format
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_Init(cam_resolution_t res, cam_pixformat_t pf)
{
  int32_t ret;
  uint32_t device_id;
  uint32_t ov5640_res, ov5640_pf;

  if (IsCameraInitialized)
  {
    return CAM_BUSY;
  }

  CameraIO.Init = CAM_IO_Init;
  CameraIO.DeInit = CAM_IO_DeInit;
  CameraIO.Address = CAM_SENSOR_I2C_ADDR_8BIT;
  CameraIO.WriteReg = CAM_IO_WriteReg;
  CameraIO.ReadReg = CAM_IO_ReadReg;
  CameraIO.GetTick = CAM_IO_GetTick;

  ret = OV5640_RegisterBusIO(&CameraObj, &CameraIO);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  ret = OV5640_ReadID(&CameraObj, &device_id);
	printf("Camera ID is 0x%04X\n", device_id); 
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  if (device_id != OV5640_ID)
  {
    return CAM_ERROR;
  }

  if (MapResolution(res, &ov5640_res) != CAM_OK)
  {
    return CAM_UNSUPPORTED;
  }

  if (MapPixelFormat(pf, &ov5640_pf) != CAM_OK)
  {
    return CAM_UNSUPPORTED;
  }

  ret = OV5640_Init(&CameraObj, ov5640_res, ov5640_pf);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  CurrentResolution = res;
  CurrentPixelFormat = pf;
  IsCameraInitialized = 1;

  return CAM_OK;
}

/**
  * @brief  DeInit (optional powerdown hook, stop DCMI)
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_DeInit(void)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  /* Stop DCMI if running */
  if (HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_RESET)
  {
    HAL_DCMI_Stop(&hdcmi);
  }

  /* Power down camera - currently not implemented in OV5640 driver */
  /* Just return success */

  IsCameraInitialized = 0;

  return CAM_OK;
}

/**
  * @brief Start capturing into buff using DMA.
  * @param buff   Destination buffer (must be large enough)
  * @param mode   CAM_MODE_CONTINUOUS or CAM_MODE_SNAPSHOT
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_Start(uint8_t *buff, uint32_t mode)
{
  HAL_StatusTypeDef status;
  uint32_t buff_size;
  
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  /* Calculate buffer size based on resolution and pixel format */
  switch(CurrentResolution)
  {
    case CAM_R160x120:
      buff_size = 160 * 120;
      break;
    case CAM_R320x240:
      buff_size = 320 * 240;
      break;
    case CAM_R480x272:
      buff_size = 480 * 272;
      break;
    case CAM_R640x480:
      buff_size = 640 * 480;
      break;
    case CAM_R800x480:
      buff_size = 800 * 480;
      break;
    default:
      return CAM_UNSUPPORTED;
  }
  
  /* Adjust for pixel format (bytes per pixel) */
  if (CurrentPixelFormat == CAM_PF_RGB888)
  {
    buff_size *= 3;  /* 3 bytes per pixel */
  }
  else
  {
    buff_size *= 2;  /* 2 bytes per pixel (RGB565 or YUV422) */
  }

  status = HAL_DCMI_Start_DMA(&hdcmi, mode, (uint32_t)buff, buff_size / 4);
  
  if (status != HAL_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief Stop camera capture.
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_Stop(void)
{
  HAL_StatusTypeDef status;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  status = HAL_DCMI_Stop(&hdcmi);

  if (status != HAL_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief Suspend camera capture.
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_Suspend(void)
{
  HAL_StatusTypeDef status;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  status = HAL_DCMI_Suspend(&hdcmi);

  if (status != HAL_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief Resume camera capture.
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_Resume(void)
{
  HAL_StatusTypeDef status;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  status = HAL_DCMI_Resume(&hdcmi);

  if (status != HAL_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief  Get the Camera Capabilities.
  * @param  Capabilities  pointer to camera Capabilities
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_GetCapabilities(cam_capabilities_t *Capabilities)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  /* Fill in capabilities structure based on OV5640 capabilities */
  Capabilities->Resolution = 0xFFFFFFFF;  /* All resolutions supported */
  Capabilities->PixelFormat = 0xFFFFFFFF;  /* All pixel formats supported */
  Capabilities->Brightness = 1;  /* Brightness control supported */
  Capabilities->Saturation = 1;  /* Saturation control supported */
  Capabilities->Contrast = 1;  /* Contrast control supported */
  Capabilities->HueDegree = 0;  /* Hue control not supported */
  Capabilities->MirrorFlip = 0;  /* Mirror/flip not supported */
  Capabilities->Zoom = 0;  /* Zoom not supported */
  Capabilities->NightMode = 0;  /* Night mode not supported */

  return CAM_OK;
}

/**
  * @brief  Set the camera pixel format.
  * @param  PixelFormat pixel format to be configured
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_SetPixelFormat(cam_pixformat_t PixelFormat)
{
  int32_t ret;
  uint32_t ov5640_pf;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  if (MapPixelFormat(PixelFormat, &ov5640_pf) != CAM_OK)
  {
    return CAM_UNSUPPORTED;
  }

  ret = OV5640_SetPixelFormat(&CameraObj, ov5640_pf);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  CurrentPixelFormat = PixelFormat;

  return CAM_OK;
}

/**
  * @brief  Get the camera pixel format.
  * @param  PixelFormat pixel format to be returned
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_GetPixelFormat(cam_pixformat_t *PixelFormat)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  *PixelFormat = CurrentPixelFormat;

  return CAM_OK;
}

/**
  * @brief  Set the camera Resolution.
  * @param  Resolution Resolution to be configured
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_SetResolution(cam_resolution_t Resolution)
{
  int32_t ret;
  uint32_t ov5640_res;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  if (MapResolution(Resolution, &ov5640_res) != CAM_OK)
  {
    return CAM_UNSUPPORTED;
  }

  ret = OV5640_SetResolution(&CameraObj, ov5640_res);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  CurrentResolution = Resolution;

  return CAM_OK;
}

/**
  * @brief  Get the camera Resolution.
  * @param  Resolution Resolution to be returned
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_GetResolution(cam_resolution_t *Resolution)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  *Resolution = CurrentResolution;

  return CAM_OK;
}

/**
  * @brief  Set the camera Brightness Level.
  * @param  Brightness Brightness Level (-4 to 4)
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_SetBrightness(int32_t Brightness)
{
  int32_t ret;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  if ((Brightness < -4) || (Brightness > 4))
  {
    return CAM_ERROR;
  }

  ret = OV5640_SetBrightness(&CameraObj, Brightness);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief  Get the camera Brightness Level.
  * @param  Brightness Brightness Level
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_GetBrightness(int32_t *Brightness)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  /* Note: This would require a getter function in the OV5640 driver */
  /* For now, we'll return an error since we don't have a getter */
  return CAM_ERROR;
}

/**
  * @brief  Set the camera Saturation Level.
  * @param  Saturation Saturation Level (-4 to 4)
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_SetSaturation(int32_t Saturation)
{
  int32_t ret;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  if ((Saturation < -4) || (Saturation > 4))
  {
    return CAM_ERROR;
  }

  ret = OV5640_SetSaturation(&CameraObj, Saturation);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief  Get the camera Saturation Level.
  * @param  Saturation Saturation Level
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_GetSaturation(int32_t *Saturation)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  /* Note: This would require a getter function in the OV5640 driver */
  /* For now, we'll return an error since we don't have a getter */
  return CAM_ERROR;
}

/**
  * @brief  Set the camera Contrast Level.
  * @param  Contrast Contrast Level (-4 to 4)
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_SetContrast(int32_t Contrast)
{
  int32_t ret;

  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  if ((Contrast < -4) || (Contrast > 4))
  {
    return CAM_ERROR;
  }

  ret = OV5640_SetContrast(&CameraObj, Contrast);
  if (ret != OV5640_OK)
  {
    return CAM_ERROR;
  }

  return CAM_OK;
}

/**
  * @brief  Get the camera Contrast Level.
  * @param  Contrast Contrast Level
  * @retval CAM_OK in case of success, CAM_ERROR in case of failure
  */
cam_status_t CAM_GetContrast(int32_t *Contrast)
{
  if (!IsCameraInitialized)
  {
    return CAM_ERROR;
  }

  /* Note: This would require a getter function in the OV5640 driver */
  /* For now, we'll return an error since we don't have a getter */
  return CAM_ERROR;
}

/* Forward interrupts to user callbacks */
void CAM_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&hdcmi);
}

void CAM_DMA_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_dcmi);
}

/* Callback functions - these should be implemented by user application */
__weak void CAM_LineEventCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the CAM_LineEventCallback could be implemented in the user file
   */
}

__weak void CAM_FrameEventCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the CAM_FrameEventCallback could be implemented in the user file
   */
}

__weak void CAM_VsyncEventCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the CAM_VsyncEventCallback could be implemented in the user file
   */
}

__weak void CAM_ErrorCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the CAM_ErrorCallback could be implemented in the user file
   */
}

/* DCMI HAL callbacks - forward to user callbacks */
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  CAM_LineEventCallback();
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  CAM_FrameEventCallback();
}

void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  CAM_VsyncEventCallback();
}

void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdcmi);

  CAM_ErrorCallback();
}