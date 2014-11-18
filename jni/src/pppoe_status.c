#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <errno.h>

#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/sockios.h>

#include <sys/un.h>
#include <android/log.h>
#include "pppoe_status.h"


#define LOCAL_TAG "PPPOE_STATUS"
//#define PRINTF printf
#define PRINTF 

static int if_is_up(const char *if_name)
{
    struct ifreq ifr;
    int s, ret;

    strlcpy(ifr.ifr_name, if_name, IFNAMSIZ);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"if_is_up(%s): failed to create socket(%s)\n", if_name, strerror(errno));

        return -1;
    }
    ret = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (ret < 0) {
        ret = -errno;
        //perror(ifr.ifr_name);
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"if_is_up(%s): failed to ioctl(%s)\n", if_name, strerror(errno));        
        goto done;
    }

    ret = (ifr.ifr_flags &= IFF_UP) ? 1 : 0;
    __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
                        "if_is_up(%s) is %s\n", if_name, ret ? "UP" : "DOWN");        
    
done:
    close(s);
    return ret;
}

#define PPP_IF_NAME "ppp0"

int get_net_updown(const char *phy_if_name)
{
    int ret;
    
    ret = if_is_up(phy_if_name);
    if(ret < 0){
        if(ENODEV == -ret)
        	__android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG, "No such device(%s)\n", phy_if_name);
        return 0;
    }

    if(0 == ret) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG, "%s is DOWN\n", phy_if_name);        
        return 0;
    }
    else {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG, "%s is UP\n", phy_if_name);        
        return 1;
    }
}

