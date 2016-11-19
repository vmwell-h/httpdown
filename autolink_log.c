/***************************************************************************//**
    \file          autolink_log.c
    \author        wangqiang
    \mail          wangqiang@auto-link.com.cn
    \version       0.1
    \date          2016-01-06
------------------------------------------------------------------------------\n
    No.       Author          Date           Version      Description\n
------------------------------------------------------------------------------\n
    001       wangqiang       2016-01-05      0.1          Create.\n
*******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <linux/un.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "autolink_log.h"
/*******************************************************************************
    Type Definition
*******************************************************************************/
#define LOG_FILEPATH_LENMAX     256
#define LOG_PATH    "/tmp/log/"
#define SOCK_PREFIX "@/tmp/"
#define NAME_PREFIX "loglevel_"

#define LOG_FILE_SIZE_MAX (524288) //512*1024
typedef struct{
    int fp;
    int ssockfd;
    char file[LOG_FILEPATH_LENMAX];
    int file_count;
    int file_suffix;
    autolink_loglevel_enum current_loglevel;
    char name[32];
}autolink_loginfo_struct;


/*******************************************************************************
    Function  Definition
*******************************************************************************/
static void autolink_loglevel_create_file(void* logident)
{
    char *p = NULL;
    char *q = NULL;
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    int count = 0;

    printf("loglevel:log to file\n");

    autolink_loginfo_struct *loginfo = (autolink_loginfo_struct *)logident;

    q = loginfo->file;
    sprintf(q,"%s%s",LOG_PATH,loginfo->name);

    p = strsep(&q, "/");
    chdir("/");

    while((p = strsep(&q, "/")) != NULL)
    {
        mkdir(p, S_IRWXU);
        chdir(p);
        //printf("%s\n",p);
    }

    if(loginfo ->file_suffix <= 0 )
    {
        sprintf(loginfo->file,"%s%s",LOG_PATH,loginfo->name);

        dir = opendir(loginfo->file);
        while((ptr = readdir(dir)) != NULL)
        {
            //printf("%s %d\n",ptr->d_name,atoi(ptr->d_name));
            if((atoi(ptr->d_name))>count)
            {
                count = atoi(ptr->d_name);
            }
        }

        closedir(dir);
        count++;
        loginfo -> file_count = count;

        //printf("%d\n",count);
    }

    sprintf(loginfo->file,
            "%s%s/%d_%d.log",
            LOG_PATH,
            loginfo->name,
            loginfo->file_count,
            loginfo->file_suffix);

    //printf("%s\n",loginfo->file);
    printf("loglevel:filemname:%s\n",loginfo->file);
    loginfo->fp = open(loginfo->file, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU);
    if(loginfo->fp<0)
    {
        printf("loglevel:open %s failed\n",loginfo->file);
    }

}
/*******************************************************************************
    Function  Definition
*******************************************************************************/
static void* autolink_loglevel_mainloop(void* user_data)
{
    struct sockaddr_un ssockaddr;
    struct sockaddr_un csockaddr;
    int ssockfd = 0;
    int csockfd = 0;
    int ssize = 0;
    socklen_t csize = 0;
    int nread = 0;
    char buff[32];
    char sockname[32];
    memset(sockname,0,32);

    autolink_loginfo_struct *loginfo = (autolink_loginfo_struct *)user_data;

    loginfo->ssockfd = ssockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    ssockaddr.sun_family = AF_UNIX;
    sprintf(sockname,"%s%s",SOCK_PREFIX,loginfo->name);
    strcpy(ssockaddr.sun_path,sockname);
    ssockaddr.sun_path[0]=0;
    ssize = offsetof(struct sockaddr_un, sun_path) + strlen(sockname);

    bind(ssockfd, (struct sockaddr *)&ssockaddr, ssize);
    listen(ssockfd, 5);

    csize = sizeof(csockaddr);
    while(1){
        memset(buff,0,32);
        if(csockfd == 0){
            csockfd = accept(ssockfd,(struct sockaddr*)&csockaddr, &csize);
        }

        nread = read(csockfd, buff, 32);
        if(nread == 0){
            if(csockfd)
                close(csockfd);
            csockfd = 0;
            continue;
        }

        printf("%s\n",buff);
        if(strncmp(buff,"kill",4) == 0){
            close(ssockfd);
            return NULL;
        }
        loginfo->current_loglevel = atoi(buff);
        printf("loglevel:%d\n",loginfo->current_loglevel);
        sleep(1);
    }

    return NULL;
}
/*******************************************************************************
    \fn          autolink_loglevel_init(char* name,autolink_loglevel_enum loglevel,
                autolink_logctr_enum ctr)
    \brief      init logvel
    \return     >NULL : Success.\n
                Others : Failed.
*******************************************************************************/
extern void* autolink_loglevel_init(char* name,autolink_loglevel_enum loglevel,
                                    autolink_logctr_enum ctr)
{
    pthread_t tid;
    autolink_loginfo_struct *loginfo =
        (autolink_loginfo_struct *)malloc(sizeof(autolink_loginfo_struct));
    if( NULL == loginfo ){
        return NULL;
    }

    memset(loginfo,0,sizeof(autolink_loginfo_struct));
    loginfo->current_loglevel = loglevel;
    sprintf(loginfo->name,"%s%s",NAME_PREFIX,name);

#if defined(AUTOLINK_LOG_FILE)
    autolink_loglevel_create_file(loginfo);
#endif

#if !defined(AUTOLINK_RELEASE)
    if(ctr == AUTOKINK_LOGCTR_TRUE){
        pthread_create(&tid, NULL, autolink_loglevel_mainloop, (void*)loginfo);
        pthread_setname_np(tid,loginfo->name);
    }
#endif
    return loginfo;
}

/*******************************************************************************
    \fn         extern void autolink_log(void* logident,autolink_loglevel_enum
                level,autolink_logtime_enum logtime,const char* fmt, ...);
    \brief      printf console log
    \return     void
*******************************************************************************/
extern void autolink_log(void* logident,autolink_loglevel_enum level,
                         autolink_logtime_enum logtime,const char* fmt, ...)
{
    time_t now;
    struct tm *tm_now;
    char log[256];
    va_list args;

    autolink_loginfo_struct *loginfo = (autolink_loginfo_struct *)logident;

    if(logident == NULL)
    {
        return;
    }

    if(logtime == AUTOKINK_LOGTIME_TRUE)
    {
        time(&now);
        tm_now = localtime(&now);
        memset(log,0,256);
        sprintf(log,"[%04d-%02d-%02d %02d:%02d:%02d]",
                1900+tm_now->tm_year,tm_now->tm_mon,
                tm_now->tm_mday, tm_now->tm_hour,
                tm_now->tm_min, tm_now->tm_sec);

    }

    if((loginfo->current_loglevel)>=level &&
       (loginfo->current_loglevel)< AUTOKINK_LOGLEVEL_PRINTT)
    {
        if(logtime == AUTOKINK_LOGTIME_TRUE)
            printf("%s",log);

        va_start(args,fmt);
        vprintf(fmt,args);
        va_end(args);
    }

#if defined(AUTOLINK_LOG_FILE)
    if(loginfo->fp < 0)
        return;

    if(level <= AUTOKINK_LOGLEVEL_1)
    {
        if(logtime == AUTOKINK_LOGTIME_TRUE)
        {
            write(loginfo->fp,log,strlen(log));
        }
        memset(log,0,256);
        va_start(args,fmt);
        vsnprintf(log,255,fmt,args);
        va_end(args);
        write(loginfo->fp,log,strlen(log));


    }

    if(level == AUTOKINK_LOGLEVEL_DATA)
    {
        write(loginfo->fp,fmt,strlen(fmt));
    }

    if(lseek(loginfo->fp,0,SEEK_CUR)>=LOG_FILE_SIZE_MAX)
    {
        close(loginfo->fp);
        loginfo->file_suffix ++;
        autolink_loglevel_create_file(logident);
    }

#endif

}
/*******************************************************************************
    \fn         int autolink_loglevel_uninit(void* logident)
    \brief      uninit log
    \return     0 : Success.\n
                Others : Failed.
*******************************************************************************/
extern int autolink_loglevel_uninit(void* logident)
{
    autolink_loginfo_struct *loginfo = (autolink_loginfo_struct *)logident;
    if(loginfo != NULL)
    {
        if(loginfo->ssockfd > 0)
            write(loginfo->ssockfd,"kill",4);
        if(loginfo->fp > 0)
            close(loginfo->fp);
        free(loginfo);
    }
    return 0;
}
