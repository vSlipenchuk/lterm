#include "lterm.h"
#include "magma.h"

void magma_crypt(unsigned char *in,unsigned char *out,int len) { // my crypt
unsigned char ctr[sizeof(init_vect_ctr_string)]; memcpy(ctr,init_vect_ctr_string,sizeof(init_vect_ctr_string));
CTR_Crypt(ctr, in, out,  cypher_key, len);
}

/// new one
#define MAX_UDP 1024 // buffer send-recv max

char *sa2data(struct sockaddr_in *sa,char *host,  int *Port) {
//static char szstr[80];
struct sockaddr_in *sin = (struct sockaddr_in*)sa;
uchar *ip  =(void*)& (sin->sin_addr.s_addr);//(uchar*)((&(struct sock_addr_in*)sa)->sin_addr.s_addr);
 int port = ntohs(sin->sin_port);
if (host) sprintf(host,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
*Port = port;
return host;
}


struct sockaddr_in udp_target_sa; // must be reset on a new packets
//int sa_len = sizeof(udp_target_sa);

int lterm_udp_recv_print(int sock) { // print any recv to stdout with newline '\n'
struct sockaddr_in s;
int port=0; char buf[MAX_UDP+1]; unsigned int slen=sizeof(s);
struct sockaddr_in recv_sa;
//printf("getsockname ret=%d\n",ret); if (ret<0) perror("getsockname");
getsockname(sock, (void*)&s, &slen); sa2data(&s,0,&port); // BUG in linux! - udp.port = 0 !!! - cant fix
fprintf(stderr,"[listen sock#%d udp_port:%d started]\n",sock,port);
while(1) {
  unsigned int sa_len = sizeof(recv_sa);
  int len = recvfrom(sock,buf,sizeof(buf)-1,0,(void*)&recv_sa,&sa_len);
  if (memcmp(&recv_sa,&udp_target_sa,sizeof(recv_sa))!=0) {
     char host[80]; int port=0;
     sa2data(&recv_sa,host,&port);
     fprintf(stderr,"[switch to new connect from %s:%u]\n",host,port);
     udp_target_sa = recv_sa;
     }
  if (len<0) break;
  buf[len]=0; // teroterm
  if (magma_ready) { // decipher
     char obuf[MAX_UDP];
        hex_dump("recv:",buf,len);
        //unsigned char ctr[sizeof(init_vect_ctr_string)]; memcpy(ctr,init_vect_ctr_string,sizeof(init_vect_ctr_string));
        //CTR_Crypt(ctr, buf, obuf, cypher_key, len );
        magma_crypt(buf,obuf,len);
        memcpy(buf,obuf,len); // copy here
      //  hex_dump("dec:",buf,len);
     }
  if (len>0) printf("%s\n",buf);
  }
return 0;
}




int lterm_udp_main(char *host) { // host:port
char buf[MAX_UDP+2]; // max udp
strNcpy(buf,host); host=buf; // copy for truncating...
int port = 0; // defport - any
char *p = strchr(host,':'); if (p){ *p=0; port=atoi(p+1);};
//printf("host:%s port=%d\n",host,port);
if (*host) { // define target sa NOW
  if (net_sa(&udp_target_sa,host,port)<0) {
    fprintf(stderr,"ERROR: cant create dest sockaddr for host:port %s:%d\n",host,port);
    return 3;
    }
  }
int sock = udp_sock(port,0);
//printf("sock for port %d is %d\n",port,sock);
if (sock<0) { // cant create socket - try any other
   fprintf(stderr,"FAIL listen udp_port %d, try random\n",port);
   if (!*host) {
     fprintf(stderr,"FAIL listen udp_port:%d and no HOST defined, Abort\n",port);
     return 1; //error
     }
   sock = udp_sock(0,0);
   if (sock<0) { fprintf(stderr,"FAIL listen random udp_port\n"); return 2;}
   fprintf(stderr,"[radmon port listen started on sock:%d]\n",sock);
   //printf("getsockname ret=%d\n",ret); if (ret<0) perror("getsockname");
   struct sockaddr_in s; unsigned int slen=sizeof(s);
   getsockname(sock, (void*)&s, &slen); sa2data(&s,0,&port);
   //printf("new port=%d\n",port);
   }
//printf("sock=%d host:%s port=%d\n",sock, host,port);
thread_create( lterm_udp_recv_print, (void*) sock ); //print any accepted on screen...
//lterm_udp_recv_print(sock);
while(1) {
  //fgets(buf,sizeof(buf),stdin);
  char *buf = readline(0);
  char *c = trim(buf); //strcat(c,"\r\n");
  int len = strlen(c);
  add_history(buf); // add to history buffer
  if (magma_ready) {
     char obuf[MAX_UDP]; int pad = 0;
     pad = len%8; if (pad) pad=8-pad;
     memset(c+len,0,pad);len+=pad;
     //    unsigned char ctv[sizeof(init_vect_ctr_string)]; memcpy(ctv,init_vect_ctr_string,sizeof(init_vect_ctr_string));
     //CTR_Crypt(ctv, c, obuf, cypher_key, len );
     hex_dump("raw",c,len);
     magma_crypt(c,obuf,len);
     memcpy(buf,obuf,len); c=buf;
     hex_dump("send_enc",c,len);
     }
  int l = sendto(sock,c,len,0,(void*)&udp_target_sa,sizeof(udp_target_sa)); // send without '\r\n'

  free(buf);
  //printf("send res=%d on sock %d\n",l,sock);
  if (l<0) { perror("[sendto error]"); break; }
  }
return 0;
}
