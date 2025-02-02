/*******************************************************************************
 * @file
 * @brief Co-Processor Communication Protocol(CPC) - Utils
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <errno.h>
#include <sys/stat.h>

#include "cpcd/logging.h"
#include "cpcd/utils.h"

int recursive_mkdir(const char *dir, size_t len, const mode_t mode)
{
  char *tmp = NULL;
  char *p = NULL;
  struct stat sb;
  int ret;

  tmp = (char *)zalloc(len + 1);
  FATAL_ON(tmp == NULL);

  /* copy path */
  ret = snprintf(tmp, len + 1, "%s", dir);
  FATAL_ON(ret < 0 || (size_t) ret >= (len + 1));

  /* remove trailing slash */
  if (tmp[len - 1] == '/') {
    tmp[len - 1] = '\0';
  }

  /* check if path exists and is a directory */
  ret = stat(tmp, &sb);
  if (ret == 0) {
    /* ret is 0 if S_ISDIR returns a non-zero value, meaning that the path is a directory */
    ret = S_ISDIR(sb.st_mode) != 0 ? 0 : -1;
    goto cleanup;
  }

  /* recursive mkdir */
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = 0;

      /*
       * mkdir first and then check stat to avoid potential
       * time of check/time of use issues
       */
      ret = mkdir(tmp, mode);
      if (ret < 0) {
        if (errno != EEXIST) {
          goto cleanup;
        }
      }

      ret = stat(tmp, &sb);
      if (ret != 0) {
        goto cleanup;
      }

      ret = S_ISDIR(sb.st_mode) != 0 ? 0 : -1;
      if (ret) {
        goto cleanup;
      }

      *p = '/';
    }
  }

  ret = mkdir(tmp, mode);
  if (ret < 0) {
    if (errno != EEXIST) {
      goto cleanup;
    }
  }

  ret = stat(tmp, &sb);
  if (ret != 0) {
    goto cleanup;
  }

  ret = S_ISDIR(sb.st_mode) != 0 ? 0 : -1;

  /* Fall through to return ret */

  cleanup:
  free(tmp);
  return ret;
}
