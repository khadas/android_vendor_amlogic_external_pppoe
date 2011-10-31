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
#include <sys/stat.h>
#include <android/log.h>
#include "pppoe_ctrl.h"


#define LOCAL_TAG "PPPOE_CTRL"
#define TMPDIR  "/etc/ppp"

struct pppoe_ctrl * pppoe_ctrl_open(const char *ctrl_path)
{
	struct pppoe_ctrl *ctrl;
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

try_again:
	ret = snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path),
			  "%s/ppp_cli-%d", TMPDIR, getpid());

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"%s: ctrl->local.sun_path: %s\n", __FUNCTION__, ctrl->local.sun_path);
	if (ret < 0 || (size_t) ret >= sizeof(ctrl->local.sun_path)) {
		close(ctrl->s);
		free(ctrl);
		return NULL;
	}
	tries++;
	if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
		    sizeof(ctrl->local)) < 0) {
#if 1		    
		if (errno == EADDRINUSE && tries < 2) {
			/*
			 * getpid() returns unique identifier for this instance
			 * of pppoe_ctrl, so the existing socket file must have
			 * been left by unclean termination of an earlier run.
			 * Remove the file and try again.
			 */
			unlink(ctrl->local.sun_path);
            		__android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"%s: bind failed(%s)\n", __FUNCTION__, strerror(errno));
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

#define REQUEST_BUF_LEN 1024
#define ACK_BUF_LEN 128
#define RESEND_CNT_MAX 10

static char request_buf[REQUEST_BUF_LEN];
static char ack_buf[ACK_BUF_LEN];
static int  req_no = 0;


int pppoe_ctrl_request(struct pppoe_ctrl *ctrl, const char *cmd, size_t cmd_len)
{   
    int nwritten = -1;
    int acked = 0;
    fd_set rfds;
    struct timeval tv;
    int res;
    int pid_and_reqno_len =0;
    int resend_cnt = 0;
    
    do {
        request_buf[0] = 0;
        nwritten = sprintf(request_buf, "%d\t", getpid());
        nwritten += sprintf(request_buf + nwritten, "%d\t", req_no++);
        pid_and_reqno_len = nwritten;
        nwritten += sprintf(request_buf + nwritten, "%s", cmd);

    	if (send(ctrl->s, request_buf, nwritten, 0) < 0) {
    		__android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"send command[%s] failed(%s)\n", cmd, strerror(errno));
    		goto exit_func;
    	}

        resend_cnt++;

		tv.tv_sec = 10;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(ctrl->s, &rfds);
		res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
        if ( res < 0 ) {
        	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"failed to select(%s)\n", strerror(errno));
            goto exit_func;
        } else if ( 0 == res ){
        	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"Timeout to recv ack, resend request\n");        
            continue;            
        }else if (FD_ISSET(ctrl->s, &rfds)) {
			res = recv(ctrl->s, ack_buf, ACK_BUF_LEN-1, 0);
			if (res < 0) {
            	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"failed to recv ack(%s)\n", strerror(errno));
                goto exit_func;
			}

            if (0 == strncmp(ack_buf, request_buf,pid_and_reqno_len)) {
            	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"recved VALID ack\n");
                acked = 1;                
            }
            else {
            	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"recved INVALID ack: pid_and_reqno_len(%d)\n", pid_and_reqno_len);
                ack_buf[pid_and_reqno_len] = 0;
                request_buf[pid_and_reqno_len] = 0;
            	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"ack_buf[%s]\n", ack_buf);
            	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"request_buf[%s]\n", request_buf);                
            }
		}
    }while(!acked && resend_cnt < RESEND_CNT_MAX);

exit_func:
	__android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,"send command[%s] %s\n", cmd, acked ? "OK" : "failed");
	return acked ? 0 : -1;
}


int pppoe_ctrl_get_fd(struct pppoe_ctrl *ctrl)
{
	return ctrl->s;
}


