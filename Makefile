all: 
		gcc  main.c common.c lterm_udp.c lterm_serial.c \
		  -I ../vos -lpthread \
		-lreadline -ltermcap -o lterm 
	

clear:
		rm lterm
    