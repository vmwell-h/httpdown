/*
 * =====================================================================================
 *
 *       Filename:  api_https.h
 *
 *    Description:  https handle, download upgrade package
 *
 *        Version:  1.0
 *        Created:  2015年12月31日
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Jay.Zhou (Jay), jay.zhou@oceanwing.com
 *   Organization:  Yproject
 *
 * =====================================================================================
 */

#ifndef __API_HTTPS_H__
#define __API_HTTPS_H__
//#include 	<json.h>

#include "json/json.h"
#define HTTP_RETRY_CODE         -100
#define	MAX_POST_BUFF_LEN       (1024)

typedef int ( *https_callback )( struct json_object *obj);
int https_init();
int https_deinit();
int https_download(char *url, const char *filename);
int https_post(char* url, char* params,https_callback callback, int retry_times);
//int https_upload(const char *dst_url, const char *file);

#endif

/*-----------------------------EOF-----------------------------------*/
