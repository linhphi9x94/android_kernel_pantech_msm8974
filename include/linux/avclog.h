#ifndef __AVCLOG_H
#define __AVCLOG_H

#define AVC_FATAL 1
#define AVC_NORMAL 0
#define AVC_CONFIRM 0

#define AVC_DBG_FATAL(x, y)	if (x) y
#define AVC_DBG_NORMAL(x, y)	if (x) y
#define AVC_DBG_CONFIRM(x, y)	if (x) y

extern int avclog_write(unsigned int err, const char *format, ...);

#endif
