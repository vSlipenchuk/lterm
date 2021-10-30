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

#define szVersion "0.1"


int lterm_udp_main(char *host) ;  // host:port
int lterm_serial_main(char *dev); // /dev/tty*:speed

#endif // LTERM_H
