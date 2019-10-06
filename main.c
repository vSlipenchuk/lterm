#include <stdio.h>
#include <stdlib.h>
#include "vos.h"
//#include <socket.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include "strutil.h"


int sock;
struct sockaddr sa;
char buf[256];
int aborted = 0;
int port = 30000; // default PORT

int udp_lterm_send(char *host,int port,int ncmd, char **cmd) {
int sock = udp_sock(port,0); // any socket, no listen on any interface
if (sock<0) {
    fprintf(stderr,"ERROR: cant create UDP on port %d\n",port);
    return 2;
    }
if (net_sa(&sa,host,port)<0) {
    fprintf(stderr,"ERROR: cant create dest sockaddr for host:port %s:%d\n",host,port);
    return 3;
    }
fprintf(stderr,"stdin->udp: <%s:%d>\n",host,port);
while(!aborted) {
  if (ncmd>0) { // first - read commands from command line
    strNcpy(buf,cmd[0]);
    ncmd--; cmd++;
    } else fgets(buf,sizeof(buf),stdin);
  if (!buf[0]) { aborted=1; break;}
  if (strcmp(buf,"exit")==0) break;
  sendto(sock,buf,strlen(buf),0,&sa,sizeof(sa));
  }
return 0;
}


int main(int npar,char **par)
{
    net_init();
    if (npar<3) {
      fprintf(stderr,"usage: <udp> <host:port>\n");
      return 1;
      }
    char *cmd = par[1];
    if (lcmp(&cmd,"udp")) {
        char *host = par[2],*p;
        p = strchr(host,':'); if (p) {*p=0; sscanf(p+1,"%d",&port);}
        return udp_lterm_send(host,port,npar-2,par+2);
        }
    else {
       fprintf(stderr,"command %s unknown\n",cmd);
       }
    return 0;
}
