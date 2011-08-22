#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <errno.h>

#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/un.h>
#include <android/log.h>
#include "pppoe_ctrl.h"


#define LOCAL_TAG "PPPOE_CTRL"

struct pppoe_ctrl * pppoe_ctrl_open(const char *ctrl_path)
{
	struct pppoe_ctrl *ctrl;
	static int counter = 0;
	int ret;
	size_t res;
	int tries = 0;

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"%s(ctrl_path = %s)\n", __FUNCTION__, ctrl_path);
	ctrl = malloc(sizeof(*ctrl));
	if (ctrl == NULL)
		return NULL;
	memset(ctrl, 0, sizeof(*ctrl));

	ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ctrl->s < 0) {
		free(ctrl);
		return NULL;
	}
    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"%s(ctrl->s = %d)\n", __FUNCTION__, ctrl->s);

	ctrl->local.sun_family = AF_UNIX;
	counter++;
try_again:
	ret = snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path),
			  "/dev/socket/ppp_cli-%d", getpid());

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"%s: ctrl->local.sun_path: %s\n", __FUNCTION__, ctrl->local.sun_path);
	if (ret < 0 || (size_t) ret >= sizeof(ctrl->local.sun_path)) {
		close(ctrl->s);
		free(ctrl);
		return NULL;
	}
	tries++;
	if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
		    sizeof(ctrl->local)) < 0) {
#if 0		    
		if (errno == EADDRINUSE && tries < 2) {
			/*
			 * getpid() returns unique identifier for this instance
			 * of pppoe_ctrl, so the existing socket file must have
			 * been left by unclean termination of an earlier run.
			 * Remove the file and try again.
			 */
			unlink(ctrl->local.sun_path);
			goto try_again;
		}
        
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"%s: bind failed(%s)\n", __FUNCTION__, strerror(errno));
		close(ctrl->s);
		free(ctrl);
		return NULL;
#endif
    }

	ctrl->dest.sun_family = AF_UNIX;
	res = strlcpy(ctrl->dest.sun_path, ctrl_path,
			 sizeof(ctrl->dest.sun_path));
	if (res >= sizeof(ctrl->dest.sun_path)) {
		close(ctrl->s);
		free(ctrl);
		return NULL;
	}

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"%s: ctrl->dest.sun_path: %s\n", __FUNCTION__, ctrl->dest.sun_path);
	if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
		    sizeof(ctrl->dest)) < 0) {
		close(ctrl->s);
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"%s: connection failed\n", __FUNCTION__);
		unlink(ctrl->local.sun_path);
		free(ctrl);
		return NULL;
	}

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"connection to ppp_wrapper OK\n");
	return ctrl;
}


void pppoe_ctrl_close(struct pppoe_ctrl *ctrl)
{
	if (ctrl == NULL)
		return;
	unlink(ctrl->local.sun_path);
	if (ctrl->s >= 0)
		close(ctrl->s);
	free(ctrl);
}



int pppoe_ctrl_request(struct pppoe_ctrl *ctrl, const char *cmd, size_t cmd_len)
{
	if (send(ctrl->s, cmd, cmd_len, 0) < 0) {
		__android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"send command[%s] failed\n", cmd);
		return -1;
	}

	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"send command[%s] OK\n", cmd);
	return 0;
}


int pppoe_ctrl_get_fd(struct pppoe_ctrl *ctrl)
{
	return ctrl->s;
}


