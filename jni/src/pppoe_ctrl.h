#ifndef PPPOE_CTRL_H
#define PPPOE_CTRL_H

#define PPPOE_PIDFILE "/data/misc/ppp/pppoe.pid"

struct pppoe_ctrl {
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};


#ifdef __cplusplus
extern "C" {
#endif
struct pppoe_ctrl * pppoe_ctrl_open(const char *ctrl_path);

void pppoe_ctrl_close(struct pppoe_ctrl *ctrl);

int pppoe_ctrl_request(struct pppoe_ctrl *ctrl, const char *cmd, size_t cmd_len);

int pppoe_ctrl_get_fd(struct pppoe_ctrl *ctrl);


#ifdef __cplusplus
}
#endif


#endif
