#include "lterm.h"


/*

#define FIONREAD	0x541B

int kbhit() {
static const int STDIN = 0;
static int initialized = 0;

if (! initialized) {
    // Use termios to turn off line buffering
    struct termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = 1;
};

int bytesWaiting;
ioctl(STDIN, FIONREAD, &bytesWaiting);
return bytesWaiting;
}
*/
int aborted=0;


int serial_print_read(void *com) { // print everything from com -> to screen
 char buf[1024]; int l;
while(1) {
  l = read((int)com,buf,sizeof(buf));
 if (l<0) { msleep(10); continue;}; //break;
 if (l==0) break; // EOF?
 buf[l]=0;
  //  char *c=trim(buf); // trim it
  //printf("%s\n",c);   prt_write(com,c,strlen(c)); // prt_write(com,"\r\n",2); // write to com
  //  prt_write(com,buf,l); // as is
  write(1,buf,l);
 }
fprintf(stderr,"[EOF on comport ret=%d com=%p]\n",l,com);
aborted=1; // ZU! - all will stop
return 0;
}

int lterm_serial_main(char *dev) {
char buf[200]; int speed=115200;
strNcpy(buf,dev); dev=buf;
char *c = strchr(dev,':'); if (c) {*c=0; speed=atoi(c+1);}
void * com = prt_open(dev,speed);
if (!com) {
  fprintf(stderr,"[open %s failed, abort]\n",dev);
  return 1;
  }
fprintf(stderr,"[comport %s opened speed:%d]\n",dev,speed);
thread_create(serial_print_read,com);
while(!aborted) { // send to comport
    char *buf = readline(0);
    char *c = trim(buf);
    prt_write(com,c,strlen(c));
    add_history(buf); free(buf);
    if (prt_write(com,"\r\n",2)<0) break;  // write to com
    }
prt_close(com);
return 0;
}
