#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>

#include "api_https.h"
#include "model_default.h"
#include "json/json.h"

#include "util_encode.h"
#include "story_system_upgrade.h"
#include "model_externs.h"


#define SOFTWARE_VERSION "1.0.0"


#define HTTP_POST_TIME (1*60*60)
#define HTTP_POST_PATH "http://yapi-us-prod.upster.me:8080/v1/anker/c/device/upgrade_info"
#define MAX_BUF_LEN (256)
#define DEVICE_CONFIG_FILE "/root/config/device.ini"
#define USER_CONFIG_FILE "/root/config/user.ini"

#define FILENAME "/root/download/updater.tar"
#define	JSON_DEVICE_ID_KEY "device_id"			/*  */
#define	JSON_MD5_KEY "md5"			            /*  */
#define	JSON_UPGRADE_KEY "need_upgrade"			/*  */
#define	JSON_SOFT_VERSION_KEY "software_version"			/*  */
#define	JSON_HARD_VERSION_KEY "hardware_version"			/*  */
#define	JSON_URL_KEY "url"			            /*  */

#define	NEED_UPGRADE_FLAG 1			            /*  */
#define ZLOG_CONFIG_FILE "/root/app/config/app_test.conf"     /* the  position of zlog configure*/
#define ZLOG_CATEGORY_NAME "y_upgrade"                        /* the catergory name match with the catergory name of configure */
typedef struct _post_request_data_t
{
    char device_id[MAX_BUF_LEN];
    char software_version[MAX_BUF_LEN];
    char hardware_version[MAX_BUF_LEN];
}post_request_data_t;


typedef struct _post_response_data_t
{
    char device_id[MAX_BUF_LEN];
    char md5[MAX_BUF_LEN];
	int need_upgrade;
    char software_version[MAX_BUF_LEN];
    char url[MAX_BUF_LEN];
}post_response_data_t;

static int http_json_to_response_data(struct json_object *obj,post_response_data_t * post_response_data)
{
	struct json_object *device_obj = json_object_object_get(obj,JSON_DEVICE_ID_KEY);
	struct json_object *need_upgrade = json_object_object_get(obj,JSON_UPGRADE_KEY);
	struct json_object *md5_obj = json_object_object_get(obj,JSON_MD5_KEY);
	struct json_object *software_obj = json_object_object_get(obj,JSON_SOFT_VERSION_KEY);
	struct json_object *url_obj = json_object_object_get(obj,JSON_URL_KEY);
	memset (post_response_data,0x0,sizeof(post_response_data_t));
	if (device_obj) {
		strcpy (post_response_data->device_id, json_object_get_string(device_obj));
		DBG_LOG ("device id:%s\n",post_response_data->device_id);
		json_object_put (device_obj);
	}
	if (need_upgrade) {
		post_response_data->need_upgrade = json_object_get_int(need_upgrade);
		DBG_LOG ("need upgrade:%d\n",post_response_data->need_upgrade);
		json_object_put (need_upgrade);
	}
	if (md5_obj) {
		strcpy (post_response_data->md5,json_object_get_string(md5_obj));
		DBG_LOG ("md5:%s\n",post_response_data->md5);
		json_object_put (md5_obj);
	}
	if (software_obj) {
		strcpy (post_response_data->software_version,json_object_get_string(software_obj));
		DBG_LOG ("software version:%s\n",post_response_data->software_version);
		json_object_put (software_obj);
	}
	if (url_obj) {
		strcpy (post_response_data->url,json_object_get_string(url_obj));
		DBG_LOG ("url:%s\n",post_response_data->url);
		json_object_put (url_obj);
	}
}
int http_post_heart_beat_back(struct json_object *obj)
{
	char md5[MAX_STRING_LENGTH]={0};
	int ret;
	pthread_t upgrade_fd;
	post_response_data_t post_response_data;
	http_json_to_response_data(obj,&post_response_data);

	if (post_response_data.need_upgrade != NEED_UPGRADE_FLAG ) {
		INFO_LOG ("need upgrade flag false\n");
		return 0;
	}
	/*
	ret = pthread_create (&upgrade_fd, NULL, thread_download_file, NULL);
	if ( ret != STATUS_OK)
	{
	    ERR_LOG ("create download file pthread success\n");
	}
	*/
	if (strcmp (post_response_data.software_version, configure_data.server_upgrade_version) == 0 )
	{
		ERR_LOG("device version == upgrade version:%s\n", post_response_data.software_version);
		return STATUS_ERROR;
	}

	ret = https_download (post_response_data.url, FILENAME);
	if(ret!=STATUS_OK)
	{
		return STATUS_ERROR;
	}
	
	MD5String(md5, FILENAME, MD5_TYPE_FILE);
	if(strcmp(md5,post_response_data.md5)==0)
	{
		start_upgrade(post_response_data.software_version);
	}
	else
	{
		ERR_LOG("md5 error, server:%s, local:%s\n",post_response_data.md5,md5);
	}
}

int http_post_data_buf_to_json(post_request_data_t post_request_data ,struct json_object *upgrade_obj)
{
	json_object_object_add(upgrade_obj,JSON_DEVICE_ID_KEY,json_object_new_string(post_request_data.device_id));
	json_object_object_add(upgrade_obj,JSON_SOFT_VERSION_KEY,json_object_new_string(post_request_data.software_version));
	json_object_object_add(upgrade_obj,JSON_HARD_VERSION_KEY,json_object_new_string(post_request_data.hardware_version));
    //https_post("http://yapi-us-qa.upster.me:8080/v1/anker/c/device/upgrade_info", json_object_to_json_string(upgrade),http_post_heart_beat_back,6);
	printf ("The json object created: %sn",json_object_to_json_string(upgrade_obj));
}

volatile int http_process_status = THREAD_START_STATUS;
void set_thread_http_heart_beat_status(int status)
{
	http_process_status = status;
}

void * thread_http_heart_beat_process(void * paraim)
{
	char ipaddress_buf[MAX_BUF_LEN] = {0};
	struct json_object *upgrade_obj = json_object_new_object ();
	post_request_data_t post_request;
	int ret;
	ret = get_string_value (DEVICE_CONFIG_FILE,"device","device_id",post_request.device_id,"");
	ret = get_string_value (DEVICE_CONFIG_FILE,"device","hardware_version",post_request.hardware_version,"");
	strcpy (post_request.software_version,SOFTWARE_VERSION);
	http_post_data_buf_to_json (post_request ,upgrade_obj);
	while (1)
	{
		if (http_process_status == THREAD_PAUSE_STATUS) {
			msleep(200);
			continue;
		}
		if (get_dev_ipaddress (ipaddress_buf) != STATUS_OK)
		{
			//INFO_LOG("no ip\n");
			sleep(3);
			continue;
		}
		unsigned int sleep_count;
		sleep_count = HTTP_POST_TIME + ( random()%30 )*60;
		sleep (sleep_count);
		https_post (HTTP_POST_PATH, json_object_to_json_string(upgrade_obj), http_post_heart_beat_back, 1);
		//sleep(3);
	}
	if (upgrade_obj) {
		json_object_put (upgrade_obj);
	}
	
}


/**
 * main - http test program 
 *
 * @argc:
 * @argv:
 *
 * returns:
 * 
 */
int
main (int argc, char **argv)
{
    int ret;
    int *pret;
    pthread_t pid_http;
    /*-----------------------------------------------------------------------------
     *  http post升级线程
     *-----------------------------------------------------------------------------*/
    ret = pthread_create (&pid_http, NULL, thread_http_heart_beat_process, NULL);
    if (ret != 0)
    {
        ERR_LOG ("create http heart beatt process pthread fail %d\n",ret);
        return -1;
    }

    while (1)
    {
      break;
    }

    if (tid_http != 0)
      pthread_join (tid_http, (void *)&pret);

}
