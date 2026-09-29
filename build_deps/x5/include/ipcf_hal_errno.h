/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/

/**
 * @file ipcf_hal_errno.h
 *
 * @NO{S17E09C01I}
 * @ASIL{B}
 */

#ifndef IPCF_HAL_ERRNO_H
#define IPCF_HAL_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif
#define IPCF_HAL_E_OK			0/**< General OK*/
#define IPCF_HAL_E_NOK			1/**< General Not OK*/
#define IPCF_HAL_E_CONFIG_FAIL		2/**< Config fail*/
#define IPCF_HAL_E_WRONG_CONFIGURATION	3/**< Wrong configuration*/
#define IPCF_HAL_E_NULL_POINTER		4/**< A null pointer was passed as an argument*/
#define IPCF_HAL_E_PARAM_INVALID	5/**< A parameter was invalid*/
#define IPCF_HAL_E_LENGTH_TOO_SMALL	6/**< Length too small*/
#define IPCF_HAL_E_INIT_FAILED		7/**< Initialization failed*/
#define IPCF_HAL_E_UNINIT		8/**< Called before initialization*/
#define IPCF_HAL_E_BUFFER_OVERFLOW	9/**< Source address or destination address Buffer overflow*/
#define IPCF_HAL_E_ALLOC_FAIL		10/**< Source alloc fail*/
#define IPCF_HAL_E_TIMEOUT		11/**< Expired the time out*/
#define IPCF_HAL_E_REINIT		12/**< Re initilize*/
#define IPCF_HAL_E_BUSY			13/**< Busy*/
#define IPCF_HAL_E_CHANNEL_INVALID	14/**< Channel is invalid*/

#ifdef __cplusplus
}
#endif

#endif/*IPCF_HAL_ERRNO_H*/
