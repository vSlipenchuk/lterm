
//#include <linux/ioctls.h>

#include "lterm.h"


int prn_help() {
fprintf(stderr,"lterm version: %s, build: "__DATE__"\n",szVersion);
fprintf(stderr,"usage: </dev/tty*:speed|udp:host:port>\n");
return 1;
}


int main(int npar,char **par)
{
    net_init();
    if (npar<2) return prn_help();
    char *cmd = par[1];
    if (lcmp(&cmd,"udp:")) return lterm_udp_main(cmd);
    else if (strncmp(cmd,"/dev/tty",8)==0) {
        return lterm_serial_main(cmd);
       }
    else {
       fprintf(stderr,"command %s unknown\n",cmd);
       }
    return 0;
}
