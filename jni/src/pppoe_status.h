#ifndef PPPOE_STATUS_H
#define PPPOE_STATUS_H

#define PPP_STATUS_CONNECTED  0x10
#define PPP_STATUS_DISCONNECTED  0x20
#define PPP_STATUS_CONNECTING  0x40

#include "../../../ppp/pppd/pathnames.h"
#define PPPOE_PIDFILE _ROOT_PATH "/etc/ppp/pppoe.pid"
#define PPPOE_WRAPPER_CLIENT_PATH _ROOT_PATH "/etc/ppp/pppoe"
#define PPPOE_WRAPPER_SERVER_PATH "/dev/socket/pppoe_wrapper"

#ifdef __cplusplus
extern "C" {
#endif

int get_pppoe_status( const char *ether_if_name);


#ifdef __cplusplus
}
#endif

#endif

