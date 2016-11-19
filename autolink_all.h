/***************************************************************************//**
    \file          autolink-all.h
    \author        huanghai
    \mail          huanghai@auto-link.com
    \version       0
    \date          2016-05-25
*******************************************************************************/
#ifndef _AUTOLINK_ALL_H
#define _AUTOLINK_ALL_H

/*******************************************************************************
    Include Files
*******************************************************************************/
#include  "autolink_log.h"

#define KB_DEBUG
//#define GST_RELEASE
#define LRC_DEBUG


#define ERR_OK  0
#define ERR_PARA_INVALID -99

/*******************************************************************************
    Type Definition
*******************************************************************************/
#define ERR_LOG(format,...)\
    autolink_log(loginfo,AUTOKINK_LOGLEVEL_0,AUTOKINK_LOGTIME_TRUE, \
    "\33[31mMEDIA ERR: \33[0m"format,##__VA_ARGS__);

#define INFO_LOG(format,...)\
    autolink_log(loginfo,AUTOKINK_LOGLEVEL_1,AUTOKINK_LOGTIME_TRUE, \
    "\33[36mMEDIA INFO: \33[0m"format,##__VA_ARGS__);


#define DBG_LOG(format,...)\
    autolink_log(loginfo,AUTOKINK_LOGLEVEL_2,AUTOKINK_LOGTIME_TRUE, \
    "\33[36mMEDIA DBG: \33[0m"format,##__VA_ARGS__);

void* loginfo;

/*******************************************************************************
    Function  Definition
*******************************************************************************/


#endif
/****************EOF****************/