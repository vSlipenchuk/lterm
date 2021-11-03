#ifndef LTERM_H
#define LTERM_H

#include <stdio.h>
#include <stdlib.h>
#include "vos.h"
//#include <socket.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include "strutil.h"

#include <termios.h>
#include <stropts.h>

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "magma.h"

#define szVersion "0.2"

// 0.1  - lterm -> serial & udp
// 0.12 - lterm - added magma cypher (second param - optional)
// 0.2 -  lterm_tcp one-client config done

extern int magma_ready; // if we need cyphering/decyphering with magma


int lterm_udp_main(char *host) ;  // host:port
int lterm_tcp_main(char *host) ;  // host:port
int lterm_serial_main(char *dev); // /dev/tty*:speed


// crypter !!!
extern unsigned char init_vect_ctr_string[BLCK_SIZE/2];  // init vector
extern unsigned char cypher_key[32]; // key for magma cypher
void magma_crypt( char *in, char *out,int len) ; // wrapper over CTR_Crypt with  custom init_vect & key


#endif // LTERM_H
