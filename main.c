/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>

#include "autolink_all.h"
#include "api_https.h"

/*******************************************************************************
    Type Definition
*******************************************************************************/
#define FILENAME "json.zip"
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
  char *url = "https://codeload.github.com/jehiah/json-c/zip/master";

  /* init media log */
  loginfo = autolink_loglevel_init("media",AUTOKINK_LOGLEVEL_2,AUTOKINK_LOGCTR_TRUE);

  INFO_LOG ("https download start.\n");
  
  ret = https_download (url, FILENAME);

  INFO_LOG ("https download %s.\n",(!ret)?"success":"fail");
}
