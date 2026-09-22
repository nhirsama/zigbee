#include "bemfa_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char buf[128];
    int len;

    len = bemfa_format_subscribe(buf, sizeof(buf), "uid", "zigbee");
    assert(len > 0);
    assert(strcmp(buf, "cmd=1&uid=uid&topic=zigbee\r\n") == 0);

    len = bemfa_format_publish(buf, sizeof(buf), "uid", "zigbee", "#1#2#3");
    assert(len > 0);
    assert(strcmp(buf, "cmd=2&uid=uid&topic=zigbee&msg=#1#2#3\r\n") == 0);

    len = bemfa_format_publish_pair(buf, sizeof(buf), "uid", "zigbee", 10u, 20u);
    assert(len > 0);
    assert(strcmp(buf, "cmd=2&uid=uid&topic=zigbee&msg=#10#20\r\n") == 0);

    len = bemfa_format_publish_sensor(buf, sizeof(buf), "uid", "zigbee", 23, 56, 1);
    assert(len > 0);
    assert(strcmp(buf, "cmd=2&uid=uid&topic=zigbee&msg=#23#56#1\r\n") == 0);

    assert(bemfa_format_subscribe(buf, sizeof(buf), "", "zigbee") == BEMFA_ERR_PARAM);
    assert(bemfa_format_publish(buf, 8, "uid", "zigbee", "#1#2#3") == BEMFA_ERR_BUFFER);

    puts("bemfa_protocol tests passed");
    return 0;
}
