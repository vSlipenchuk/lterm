#include "lterm.h"
#include "sock.h"

int lineReady(char *s) { // zu - to utils???
char *eoh, *r=s;
if (!s) return 0; // no yet
while(*r && *r<=32) r++;
eoh = strstr(r,"\n"); if (!eoh) return 0; // no "end of headers"
return 1+(eoh-s); // return block length
}


int onLineClientPacket(char *data,int len, Socket *sock) {
int l = lineReady(data);
if (l<=0) return 0; // not yet
data[l]=0;
data=trim(data);
  printf("IN:%s\n",data);
  SocketSendf(sock,"crypto_proxy_start_process:%s\n",data);
  if (lcmp(&data,"hello")) { // brpoadcast text to all
    // SocketPoolBroadcast(sock->pool,data);
     }
  if (lcmp(&data,"restart")) {
     SocketSendf(sock,"crypto_proxy_result:restarting...\n");
     sleep(1);
     exit(2); // restart system
  }
  if (lcmp(&data,"mo")) { // Mobile Terminated message [client] [hex]
     int err = -2; // syntax error
     int len = hexstr2bin(data,data,-1); // convert2
     if (len>1) {
      int client_id = (unsigned char)data[0];
      data++; len--; // test - is data
      hex_dump("mo_data_here",data,len); // just dump
  //    err = processUplink(client_id,data,len);
      }
     SocketSendf(sock,"crypto_proxy_result:%d\n",err);
     }
  if (lcmp(&data,"ao")) { // Applcation Originated,  message [client] [hex]
     int err = -2; // syntax error
     int len = hexstr2bin(data,data,-1); // convert2
     if (len>1) {
      int client_id = (unsigned char)data[0];
      data++; len--; // test - is data
      hex_dump("mo_data_here",data,len); // just dump
  //    err = processDownlink(client_id,data,len);
      }
     //SocketSendf(sock,"+mo_result %d\n",err);
     SocketSendf(sock,"crypto_proxy_result:%d\n",err);
     }
  if (lcmp(&data,"exit")) {SocketSendf(sock,"Goodbye"); sock->dieOnSend=1;}
return l;
}

int onEchoClientConnect(Socket *lsock, int handle, int ip) {
char szip[30];
SocketPool  *srv = (void*)lsock->pool; // Here My SocketServer ???
ip2szbuf(ip,szip);
printf("New connect accept to pool %p from %s\n",srv,szip);
Socket *sock = SocketPoolAccept(srv,handle,ip);
if (!sock) { // can be if wrong parameters or no pem.file
   return 0; // killed by SocketPool
   }
//SocketSendf(sock,"+crypto_proxy: %s\n",welcome);
//CLOG(srv,3,"new connect#%d from '%s', accepted\n",sock->N,sock->szip);
fprintf(stderr,"new connect#%d from '%s', accepted\n",sock->N,sock->szip);
sock->checkPacket = onLineClientPacket; // When new packet here...
//if (srv->readLimit.Limit) sock->readPacket = &srv->readLimit; // SetLimiter here (if any?)
return 1; // Everything is done...
}


int SocketLineServer(int port) {
  Socket *s = SocketNew();
  if (SocketListener(s,port,onEchoClientConnect,lineReady,0)<=0) {
     fprintf(stderr,"Fail Listen port %d\n",port); return -1;
  }
fprintf(stderr,"Port %d listened OK pool=%p sock=%p handle=%d\n",port,s->pool,s,s->sock);
while(!aborted) {
  //printf("1 pool=%p\n",s->pool);
  TimeUpdate(); // TimeNow & szTimeNow
//printf("2\n");
  int cnt = SocketPoolRun((SocketPool*)s->pool);
   // cnt+= SocketPoolRun(&srv->srv);
     // cnt+=wsSrvStep(ws);
  //printf("4\n");
     // cnt+=wsSrvStep(ws);
  //printf("SockPoolRun=%d time:%s\n",cnt,szTimeNow); msleep(1000);
  RunSleep(cnt); // Empty socket circle -)))
  //printf("5\n");
//  if (srv->runTill && TimeNow>=srv->runTill) break; // Done???
  }
//printf("YES\n");
TimeUpdate();
//printf("Server done\n");
return 0;
}


int sock_print_read(int sock) { // print everything from com -> to screen
 char buf[1024]; int l;
while(1) {
 int len;
  l = recv( sock,buf,sizeof(buf),0);
  len = l;
 if (l<0) { msleep(10); continue;}; //break;
 if (l==0) break; // EOF?
 buf[l]=0;
  //  char *c=trim(buf); // trim it
  //printf("%s\n",c);   prt_write(com,c,strlen(c)); // prt_write(com,"\r\n",2); // write to com
  //  prt_write(com,buf,l); // as is
  if (magma_ready) {
     char *c=buf;
     char obuf[1024]; int pad = 0;
     //pad = len%8; if (pad) pad=8-pad;
     //memset(c+len,0,pad);len+=pad;
     //    unsigned char ctv[sizeof(init_vect_ctr_string)]; memcpy(ctv,init_vect_ctr_string,sizeof(init_vect_ctr_string));
     //CTR_Crypt(ctv, c, obuf, cypher_key, len );
     hex_dump("raw",c,len);
     magma_crypt(c,obuf,len);
     memcpy(buf,obuf,len); c=buf;
     hex_dump("enc",c,len);
     }
  write(1,buf,len);
 }
fprintf(stderr,"[EOF on sock ret=%d sock=%p]\n",l,sock);
aborted=1; // ZU! - all will stop
return 0;
}

int sock_send_readline(int sock) {
while(!aborted) { // send to comport
    char *buf = readline(0);
    char *c = trim(buf);
    char line[strlen(c)+8]; strcpy(line,buf); strcat(line,"\r\n");
    int len = strlen(line); c=line;
      if (magma_ready) {
      char obuf[1024]; int pad = 0;
      pad = len%8; if (pad) pad=8-pad;
      memset(c+len,0,pad);len+=pad;
     //    unsigned char ctv[sizeof(init_vect_ctr_string)]; memcpy(ctv,init_vect_ctr_string,sizeof(init_vect_ctr_string));
     //CTR_Crypt(ctv, c, obuf, cypher_key, len );
      hex_dump("raw",c,len);
      magma_crypt(c,obuf,len);
      memcpy(buf,obuf,len); c=buf;
      hex_dump("enc",c,len);
     }
    if (send(sock,c,len,MSG_NOSIGNAL)<0) break; // failed
    add_history(buf); free(buf);
    }
}

int do_sock(int sock) {
//thread_create(sock_print_read,sock);
thread_create(sock_send_readline,sock); // term after fail read!
sock_print_read(sock);
/*
while(!aborted) { // send to comport
    char *buf = readline(0);
    char *c = trim(buf);
    char line[strlen(c)+8]; strcpy(line,buf); strcat(line,"\r\n");
    if (send(sock,line,strlen(line),MSG_NOSIGNAL)<0) break; // failed
    add_history(buf); free(buf);
    }
*/
}


int sock_listen_port(int port) {
int lsock = sock_listen(port,0);
if (lsock < 0) { fprintf(stderr,"[fail listen %d port, abort]\n",port); return 1;}
while(1) {
  int ip;
  int sock = sock_accept(lsock,&ip);
  if (sock>0) {
      fprintf(stderr,"[accepted connect %s]\n", ip2sz(ip));
      do_sock(sock);
      fprintf(stderr,"[done connect %s]\n",     ip2sz(ip));
      closesocket(sock);
      }
  }
return 0;
}


int lterm_tcp_main(char *host) {
int port = 0;
char *p = strchr(host,':'); if (p) {*p=0; port=atoi(p+1);} // define host:port
//if (!*host) return SocketLineServer(port); // start listen server
if (!*host) return sock_listen_port(port); // start listen server
printf("Try client connection to %s:%d\n",host,port);
int sock = sock_connect(host,port);
if (sock<0) {  fprintf(stderr,"connection failed to %s:%d err=%d\n",host,port,sock); return 0;  }
fprintf(stderr,"[connected with %s:%d]\n",host,port);
do_sock(sock);
return 0;
}
