#ifndef PPPOE_STATUS_H
#define PPPOE_STATUS_H

#define PPP_STATUS_CONNECTED  0x10
#define PPP_STATUS_DISCONNECTED  0x20
#define PPP_STATUS_CONNECTING  0x40

#ifdef __cplusplus
extern "C" {
#endif

int get_pppoe_status( const char *ether_if_name);


#ifdef __cplusplus
}
#endif

#endif

