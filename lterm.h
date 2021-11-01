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

#define szVersion "0.12"

// 0.1  - lterm -> serial & udp
// 0.12 - lterm - added magma cypher (second param - optional)

extern int magma_ready; // if we need cyphering/decyphering with magma


int lterm_udp_main(char *host) ;  // host:port
int lterm_serial_main(char *dev); // /dev/tty*:speed

extern unsigned char cypher_key[32];

#endif // LTERM_H
