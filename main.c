
//#include <linux/ioctls.h>

#include "lterm.h"
#include "magma.h"


int prn_help() {
fprintf(stderr,"lterm version: %s, build: "__DATE__"\n",szVersion);
fprintf(stderr,"usage: </dev/tty*:speed|udp:host:port> <key32byte>\n");
return 1;
}

unsigned char cypher_key[32]; // fffefdfc fbfaf9f8 f7f6f5f4 f3f2f1f0 00112233 44556677 8899aabb ccddeeff  // if have cypher
int magma_ready = 0;
unsigned char init_vect_ctr_string[BLCK_SIZE / 2] = { 0x78, 0x56, 0x34, 0x12 }; // initial vecor - must reset every send/recv data




int main(int npar,char **par)
{
//magma_test(); return 0;

    net_init();
    if (npar<2) return prn_help();
    if (npar>2) { // read cypher
       if (hexstr2bin(cypher_key,par[2],strlen(par[2]))!=sizeof(cypher_key)) {
         fprintf(stderr,"imvalid key_length, expected %d\n",sizeof(cypher_key));
         return prn_help();
         }
       hex_dump("key",cypher_key,sizeof(cypher_key));
       GOST_Magma_Expand_Key(cypher_key);
       magma_ready = 1;
       }
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
