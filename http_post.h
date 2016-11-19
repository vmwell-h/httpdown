#ifndef  THREAD_HTTP_HB_H
#define  THREAD_HTTP_HB_H
void * thread_http_heart_beat_process(void * paraim);
int start_upgrade(char *upgrade_version);
void set_thread_http_heart_beat_status(int status);
#endif
