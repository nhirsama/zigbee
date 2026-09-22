#include "bemfa_protocol.h"

#include <stdio.h>
#include <stdarg.h>

static int bemfa_checked_snprintf(char *buf, size_t buf_size, const char *fmt, ...)
{
    int len;
    va_list args;

    if (buf == 0 || buf_size == 0 || fmt == 0)
    {
        return BEMFA_ERR_PARAM;
    }

    va_start(args, fmt);
    len = vsnprintf(buf, buf_size, fmt, args);
    va_end(args);

    if (len < 0)
    {
        return BEMFA_ERR_BUFFER;
    }
    if ((size_t)len >= buf_size)
    {
        buf[0] = '\0';
        return BEMFA_ERR_BUFFER;
    }

    return len;
}

int bemfa_format_subscribe(char *buf,
                           size_t buf_size,
                           const char *uid,
                           const char *topic)
{
    if (uid == 0 || topic == 0 || uid[0] == '\0' || topic[0] == '\0')
    {
        return BEMFA_ERR_PARAM;
    }

    return bemfa_checked_snprintf(buf,
                                  buf_size,
                                  "cmd=1&uid=%s&topic=%s\r\n",
                                  uid,
                                  topic);
}

int bemfa_format_publish(char *buf,
                         size_t buf_size,
                         const char *uid,
                         const char *topic,
                         const char *message)
{
    if (uid == 0 || topic == 0 || message == 0 ||
        uid[0] == '\0' || topic[0] == '\0')
    {
        return BEMFA_ERR_PARAM;
    }

    return bemfa_checked_snprintf(buf,
                                  buf_size,
                                  "cmd=2&uid=%s&topic=%s&msg=%s\r\n",
                                  uid,
                                  topic,
                                  message);
}

int bemfa_format_publish_pair(char *buf,
                              size_t buf_size,
                              const char *uid,
                              const char *topic,
                              uint32_t first,
                              uint32_t second)
{
    char message[40];
    int len;

    len = snprintf(message,
                   sizeof(message),
                   "#%lu#%lu",
                   (unsigned long)first,
                   (unsigned long)second);
    if (len < 0 || (size_t)len >= sizeof(message))
    {
        return BEMFA_ERR_BUFFER;
    }

    return bemfa_format_publish(buf, buf_size, uid, topic, message);
}

int bemfa_format_publish_sensor(char *buf,
                                size_t buf_size,
                                const char *uid,
                                const char *topic,
                                int temperature,
                                int humidity,
                                int mode)
{
    char message[48];
    int len;

    len = snprintf(message,
                   sizeof(message),
                   "#%d#%d#%d",
                   temperature,
                   humidity,
                   mode);
    if (len < 0 || (size_t)len >= sizeof(message))
    {
        return BEMFA_ERR_BUFFER;
    }

    return bemfa_format_publish(buf, buf_size, uid, topic, message);
}
