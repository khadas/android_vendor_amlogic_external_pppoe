#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <netdb.h>
#include <utmp.h>
#include <pwd.h>
#include <setjmp.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/properties.h>
#include <sys/un.h>

#include <android/log.h>

#include "pppoe_ctrl.h"

#define LOCAL_TAG "PPPOE_WRAPPER"

#define PPP_CMD_LEN_MAX  512

static char ppp_cmd[PPP_CMD_LEN_MAX];


static pid_t read_pid(const char *pidfile)
{
	FILE *fp;
	pid_t pid;

	if ((fp = fopen(pidfile, "r")) == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
            "failed to open %s (%s)\n", pidfile, strerror(errno));
		errno = ENOENT;
		return 0;
	}

	if (fscanf(fp, "%d", &pid) != 1) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
            "failed to read pid, make pid as 0\n");
		pid = 0;
	}
	fclose(fp);

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
        "read_pid: %d\n", pid);
    
	return pid;
}

static int ppp_stop()
{
	pid_t pid;
    int ret;
    
    pid = read_pid(PPPOE_PIDFILE);
    if ( 0 == pid ) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
            "failed to stop ppp for no pid got\n" );
        return -1;
    }

    ret = kill(pid, 0);
    if ( 0 != ret ) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
            "process(#%d) died already???\n", pid );
        return -1;
    }

    /*
    The signals SIGKILL and SIGSTOP cannot 
    be caught, blocked, or ignored.
    So send SIGUSR1 to notify pppoe to send PADT.
    */
    ret = kill(pid, SIGUSR1);
    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
        "Send SIGUSR1 to pid(#%d), ret = %d\n", pid, ret );

    /*
    If no sleep before send SIGKILL, pppoe will just being killed
    rather than sending PADT.
    */
    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
        "sleep before send SIGKILL to pid(#%d)\n", pid );
    
    sleep(5);

    ret = kill(pid, SIGKILL);
    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
        "Send SIGKILL to pid(#%d), ret = %d\n", pid, ret );

	unlink(PPPOE_PIDFILE);
    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
        "removed %s\n", PPPOE_PIDFILE );

    return 0;    
}


int main(int argc, char * argv[])
{
    int socket_fd;
    struct sockaddr_un cli_addr, serv_addr;
    int i, len, clilen = 0;
    int ppp_cmd_len;
    
    socket_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
            "failed to create socket(%s)\n", strerror(errno));
        exit(-1);
    }

	memset(&serv_addr,0,sizeof(serv_addr));
    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
        "create AF_UNIX socket:%d OK\n",socket_fd);

	unlink("/dev/socket/pppd");
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, "/dev/socket/pppd", sizeof(serv_addr.sun_path) - 1);

	if (bind(socket_fd, (struct sockaddr *)&serv_addr,  sizeof(struct sockaddr_un)) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,
            "failed to bind socket(%s)\n", strerror(errno));
		exit(-1);
	}

    {
	struct timeval tv;
	int res;
	fd_set rfds;
    char *cmd;

	for (;;) {
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(socket_fd, &rfds);
		res = select(socket_fd + 1, &rfds, NULL, NULL, &tv);
		if (res > 0 && FD_ISSET(socket_fd, &rfds)) {
            clilen = sizeof (struct sockaddr_un);
			res = recvfrom(socket_fd, ppp_cmd, PPP_CMD_LEN_MAX-1, 0,
                            (struct sockaddr *)&cli_addr,&clilen);
			if (res < 0) {
                __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
                    "FAILED TO RECVFROM\n");
               
				return res;
			}
            
            __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
                "client: [%s][%d]\n",
                cli_addr.sun_path, clilen);
            
            ppp_cmd[res] = '\0';
			ppp_cmd_len = res;

            cmd = strchr(ppp_cmd, '\t');
            if (!cmd) {
                __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
                    "recv invalid cmd(No TAB found): [%s]\n",ppp_cmd);
                continue;
            }
            cmd++;
            
            cmd = strchr(cmd, '\t');
            if (!cmd) {
                __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
                    "recv invalid cmd(Second TAB NOT found): [%s]\n",ppp_cmd);
                continue;
            }
            cmd++;
            
        	if (sendto(socket_fd, ppp_cmd, cmd - ppp_cmd, 0,
                        (struct sockaddr *)&cli_addr, clilen) < 0) {
        		__android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG,"failed to send ACK(%s)\n", strerror(errno));
        		continue;
        	}

            __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG,
                "recv cmd: [%s]\n",cmd);
            
            
            if ( 0 == strcmp(cmd, "ppp-stop") ) {
                ppp_stop();
            }
            else {
                system(cmd);
            }
		}
	}
    }

    __android_log_print(ANDROID_LOG_INFO, LOCAL_TAG, "EXIT\n");
    close(socket_fd);
    return 0;
}

