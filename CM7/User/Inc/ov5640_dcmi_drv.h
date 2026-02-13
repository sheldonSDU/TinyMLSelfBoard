/**
  ******************************************************************************
  * @file    ov5640_dcmi_drv.h
  * @brief   This file contains the common defines and functions prototypes for
  *          the ov5640_dcmi_drv.c driver.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OV5640_DCMI_DRV_H
#define OV5640_DCMI_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* ===================== User Provided Handles ===================== */
extern I2C_HandleTypeDef hi2c2;
extern DCMI_HandleTypeDef hdcmi;
extern DMA_HandleTypeDef hdma_dcmi;

/* ===================== Status enums (same enum style as BSP) ===================== */
typedef enum
{
  CAM_OK       = 0,
  CAM_ERROR    = 1,
  CAM_BUSY     = 2,
  CAM_TIMEOUT  = 3,
  CAM_UNSUPPORTED = 4
} cam_status_t;

/* ===================== Resolutions (same enum style as BSP) ===================== */
typedef enum
{
  CAM_R160x120 = 0,   /* QQVGA */
  CAM_R320x240,       /* QVGA  */
  CAM_R480x272,       /* 480x272 */
  CAM_R640x480,       /* VGA */
  CAM_R800x480        /* WVGA */
} cam_resolution_t;

/* ===================== Pixel Formats ===================== */
typedef enum
{
  CAM_PF_RGB565 = 0,  /* Pixel Format RGB565 */
  CAM_PF_RGB888,      /* Pixel Format RGB888 */
  CAM_PF_YUV422       /* Pixel Format YUV422 */
} cam_pixformat_t;

/* ===================== Modes ===================== */
#define CAM_MODE_CONTINUOUS         DCMI_MODE_CONTINUOUS
#define CAM_MODE_SNAPSHOT           DCMI_MODE_SNAPSHOT

/* ===================== Camera Capabilities Structure ===================== */
typedef struct
{
  uint32_t Resolution;        /*!< Supported resolutions bitmap */
  uint32_t PixelFormat;       /*!< Supported pixel formats bitmap */
  uint32_t Brightness;        /*!< Brightness control capability */
  uint32_t Saturation;        /*!< Saturation control capability */
  uint32_t Contrast;          /*!< Contrast control capability */
  uint32_t HueDegree;         /*!< Hue control capability */
  uint32_t MirrorFlip;        /*!< Mirror/Flip capability */
  uint32_t Zoom;              /*!< Zoom capability */
  uint32_t NightMode;         /*!< Night mode capability */
} cam_capabilities_t;

/* ===================== Brightness/Saturation/Contrast Ranges ===================== */
#define CAM_BRIGHTNESS_MIN          -4
#define CAM_BRIGHTNESS_MAX           4

#define CAM_SATURATION_MIN          -4
#define CAM_SATURATION_MAX           4

#define CAM_CONTRAST_MIN            -4
#define CAM_CONTRAST_MAX             4

/* ===================== API ===================== */
/**
 * @brief  Initialize sensor (I2C) + configure sensor output format/resolution.
 *         Note: DCMI and I2C must already be initialized (you said you did).
 */
cam_status_t CAM_Init(cam_resolution_t res, cam_pixformat_t pf);

/** @brief DeInit (optional powerdown hook, stop DCMI) */
cam_status_t CAM_DeInit(void);

/**
 * @brief Start capturing into buff using DMA.
 * @param buff   Destination buffer (must be large enough)
 * @param mode   CAM_MODE_CONTINUOUS or CAM_MODE_SNAPSHOT
 */
cam_status_t CAM_Start(uint8_t *buff, uint32_t mode);

/** @brief Stop camera capture */
cam_status_t CAM_Stop(void);

/** @brief Suspend camera capture */
cam_status_t CAM_Suspend(void);

/** @brief Resume camera capture */
cam_status_t CAM_Resume(void);

/** @brief Get camera capabilities */
cam_status_t CAM_GetCapabilities(cam_capabilities_t *Capabilities);

/** @brief Set pixel format */
cam_status_t CAM_SetPixelFormat(cam_pixformat_t PixelFormat);

/** @brief Get pixel format */
cam_status_t CAM_GetPixelFormat(cam_pixformat_t *PixelFormat);

/** @brief Set resolution */
cam_status_t CAM_SetResolution(cam_resolution_t Resolution);

/** @brief Get resolution */
cam_status_t CAM_GetResolution(cam_resolution_t *Resolution);

/** @brief Set brightness level (-4 to 4) */
cam_status_t CAM_SetBrightness(int32_t Brightness);

/** @brief Get brightness level */
cam_status_t CAM_GetBrightness(int32_t *Brightness);

/** @brief Set saturation level (-4 to 4) */
cam_status_t CAM_SetSaturation(int32_t Saturation);

/** @brief Get saturation level */
cam_status_t CAM_GetSaturation(int32_t *Saturation);

/** @brief Set contrast level (-4 to 4) */
cam_status_t CAM_SetContrast(int32_t Contrast);

/** @brief Get contrast level */
cam_status_t CAM_GetContrast(int32_t *Contrast);

/* ===================== IRQ Handlers ===================== */
void CAM_IRQHandler(void);
void CAM_DMA_IRQHandler(void);

/* ===================== Callback Functions (to be implemented by user) ===================== */
void CAM_LineEventCallback(void);
void CAM_FrameEventCallback(void);
void CAM_VsyncEventCallback(void);
void CAM_ErrorCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* OV5640_DCMI_DRV_H */