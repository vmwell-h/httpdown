/*
 * =====================================================================================
 *
 *       Filename:  api_https.c
 *
 *    Description:  https handle, download upgrade package
 *
 *        Version:  1.0
 *        Created: 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Jay.Zhou (Jay), jay.zhou@oceanwing.com
 *   Organization:  Yproject
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <unistd.h>
#include <string.h>

#include "autolink_all.h"
#include "api_https.h"

#define DEFAULT_URL_DATA_SIZE   (4 * 1024)

#define HTTP_TIMEOUT 30
#define HTTP_RETRY_SLEEP 3

#define STATUS_OK    0
#define STATUS_ERROR -1



typedef struct _tag_URL_DATA
{
    size_t size;
    char  *data;
}URL_DATA_S,*pURL_DATA;


size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)                                                                                                                              
{
    pURL_DATA data = (pURL_DATA)userp;
    size_t index = data->size;
    size_t write_size = (size * nmemb);
    char* tmp;
    
    data->size += write_size;
    tmp = (char *)realloc(data->data, data->size + 1); /* +1 for '\0' */

    if(tmp) 
    {
        data->data = tmp;
    } 
    else 
    {
        if(data->data) 
        {
            free(data->data);
        }
        DBG_LOG("allocate memory fail.\n");
        return 0;
    }
    memcpy((data->data + index), buffer, write_size);
    data->data[data->size] = '\0';
    //printf("=======%s\n",data->data);
    return write_size;
}

void set_share_handle(CURL* curl_handle)  
{  
    static CURLSH* share_handle = NULL;  
    if (!share_handle)  
    {  
        share_handle = curl_share_init();  
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);  
    }  
    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);  
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 60);  
} 

int https_post_wrap(char *url, char *params, https_callback callback)
{
    CURL *handle = NULL;
    CURLcode ret;
    URL_DATA_S data;
	int result=-100;
	
    data.size = 0;
    data.data = (char *)malloc(DEFAULT_URL_DATA_SIZE);
    if (NULL == data.data)
    {
        ERR_LOG("malloc memory fail.\n");
        return result;
    }
    memset(data.data, 0, DEFAULT_URL_DATA_SIZE);
    handle = curl_easy_init();
	if(handle==NULL){
		free(data.data);
		ERR_LOG("curl easy init fail.\n");
		return result;
	}
	set_share_handle(handle);
	INFO_LOG("url:%s,params:%s\n",url,params);
	curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, params);

	curl_easy_setopt(handle, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handle, CURLOPT_HTTP_TRANSFER_DECODING, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
	//curl_easy_setopt(handle, CURLOPT_DNS_SERVERS, "8.8.8.8");
	//curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    ret = curl_easy_perform(handle);    

	INFO_LOG("return(%d):%s \n", ret, (char *)data.data);

	if (CURLE_OK == ret && strcmp(data.data,"") != 0){
		struct json_object *obj;
		obj = json_tokener_parse(data.data);
		if(is_error(obj))
		{
			ERR_LOG("invalid response format.\n");
			free(data.data);
			curl_easy_cleanup(handle);
			if (obj) json_object_put(obj);
			return -1;
		}
		result=callback(obj);
		if (obj) json_object_put(obj);
	}
	free(data.data);
	curl_easy_cleanup(handle);
	return result;
}

//timeout
int https_post(char* url, char* params,https_callback callback, int retry_times)
{
	int iResult=0;
	int i = 0;
	
	for (i = 0; retry_times==-1||i < retry_times; i++){
		iResult=https_post_wrap(url,params,callback);
		if(iResult!=HTTP_RETRY_CODE) break;
		sleep(HTTP_RETRY_SLEEP);
	}
	return iResult;
}



static int progress_function(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (dltotal < 10.0)								//in case of empty file downloaded
		return STATUS_OK;
	int j = 0;
	double percent = dlnow / dltotal;
    //DBG_LOG("\r%d%%\t|", (int)(percent*100));
	int i = 0;
	for (; i < (int)(50 * percent); ++i)
	{
		putchar('*');
	}
	for (j = 0; j < 50-i; ++j)
	{
		putchar(' ');
	}
	if (dlnow < 1024)
	{
		printf("| %dB done.\n", (int)dlnow);
	} else
	{
		printf("| %dkB done.\n", (int)(dlnow/1024));
	}
	return STATUS_OK;
}

int https_download(char *url, const char *filename)
{
    CURL *handle = NULL;
    CURLcode ret;
    FILE *file;
	static int has_download=-1;
	if(has_download==1)
	{
        ERR_LOG("downloading:%s\n", filename);
        return STATUS_ERROR;
	}
	has_download=1;
    file = fopen(filename, "wb");
    if (file == NULL) {
        ERR_LOG("open %s fail\n", filename);
        return STATUS_ERROR;
    }

    handle = curl_easy_init();
    ret = curl_easy_setopt(handle,CURLOPT_URL,url);
    if (ret != CURLE_OK)
	{
		ERR_LOG("curl easy set URL fail\n");
		goto err_exit;
	}

    ret = curl_easy_setopt(handle,CURLOPT_NOPROGRESS,0L);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy set NOPROGRESS fail\n");
		goto err_exit;
    }

    ret = curl_easy_setopt(handle,CURLOPT_PROGRESSFUNCTION,progress_function);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy set progress function fail\n");
		goto err_exit;
    }
    /*
    ret = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_file);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy set write function failed!\n");
        goto err_exit;
    }
    */
    ret = curl_easy_setopt(handle,CURLOPT_WRITEDATA,file);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy set file fail\n");
		goto err_exit;
    }

    ret = curl_easy_setopt(handle,CURLOPT_LOW_SPEED_TIME,5L);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy set low speed timeout fail\n");
		goto err_exit;
    }

    ret = curl_easy_setopt(handle,CURLOPT_LOW_SPEED_LIMIT,5L);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy set low speed limit fail\n");
		goto err_exit;
    }

    INFO_LOG("downloading file %s from %s\n",filename,url);
    ret = curl_easy_perform(handle);
    if (ret != CURLE_OK)
    {
        ERR_LOG("curl easy perform fail\n");
		goto err_exit;
    }

    curl_easy_cleanup(handle);
	fclose(file);
	has_download=0;
    return STATUS_OK;

err_exit:
    curl_easy_cleanup(handle);
	fclose(file);
	has_download=0;
    return STATUS_ERROR;

}

int https_init()
{
    CURLcode ret;

    ret = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != ret)
    {
        ERR_LOG("curl global init fail\n");
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

int https_deinit()
{
    curl_global_cleanup();

    return STATUS_OK;
}
/*-----------------------------EOF-----------------------------------*/
