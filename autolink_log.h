/***************************************************************************//**
    \file          autolink_log.h
    \author        wangqiang
    \mail          wangqiang@auto-link.com.cn
    \version       0.1
    \date          2016-01-05
------------------------------------------------------------------------------\n
    No.       Author          Date           Version      Description\n
------------------------------------------------------------------------------\n
    001       wangqiang       2016-01-05      0.1          Create.\n
*******************************************************************************/
#ifndef _AUTOLINK_LOG_H
#define _AUTOLINK_LOG_H

/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
    Type Definition
*******************************************************************************/
typedef enum{
    AUTOKINK_LOGLEVEL_0 = 0,
    AUTOKINK_LOGLEVEL_1,
    AUTOKINK_LOGLEVEL_2,
    AUTOKINK_LOGLEVEL_3,
    AUTOKINK_LOGLEVEL_4,
    AUTOKINK_LOGLEVEL_5,
    AUTOKINK_LOGLEVEL_6,
    AUTOKINK_LOGLEVEL_7,
    AUTOKINK_LOGLEVEL_8,
    AUTOKINK_LOGLEVEL_9,
    AUTOKINK_LOGLEVEL_PRINTT = 125,
    AUTOKINK_LOGLEVEL_DATA = 126,
    AUTOKINK_LOGLEVEL_MAX = 127
}autolink_loglevel_enum;

typedef enum{
    AUTOKINK_LOGCTR_TRUE = 0,
    AUTOKINK_LOGCTR_FALSE,
}autolink_logctr_enum;

typedef enum{
    AUTOKINK_LOGTIME_TRUE = 0,
    AUTOKINK_LOGTIME_FALSE,
}autolink_logtime_enum;

/*******************************************************************************
    Function  Definition
*******************************************************************************/
/*******************************************************************************
    \fn         extern void* autolink_loglevel_init(char* name,
                autolink_loglevel_enum loglevel,autolink_logctr_enum ctr)
    \brief      init logvel
    \return     >NULL : Success.\n
                Others : Failed.
*******************************************************************************/
extern void* autolink_loglevel_init(char* name,autolink_loglevel_enum loglevel,
                                    autolink_logctr_enum ctr);

/*******************************************************************************
    \fn         extern void autolink_log(void* logident,autolink_loglevel_enum
                level,autolink_logtime_enum logtime,const char* fmt, ...);
    \brief      printf console log
    \return     void
*******************************************************************************/
extern void autolink_log(void* logident,autolink_loglevel_enum level,
                         autolink_logtime_enum logtime,const char* fmt, ...);

/*******************************************************************************
    \fn         int autolink_loglevel_uninit(void* logident)
    \brief      uninit log
    \return     0 : Success.\n
                Others : Failed.
*******************************************************************************/
extern int autolink_loglevel_uninit(void* logident);


#ifdef __cplusplus
}
#endif
#endif /*_AUTOLINK_LOG_H */
