CONTIKI_PROJECT = start-iotorii
all: $(CONTIKI_PROJECT)

CONTIKI = ../..

PROJECT_SOURCEFILES += iotoriicsma.c 
PROJECT_SOURCEFILES += hlmacaddr.c


# use a custom MAC driver: iotorii_mac_driver
MAKE_MAC = MAKE_MAC_OTHER
MAKE_NET = MAKE_NET_NULLNET
MAKE_ROUTING = MAKE_ROUTING_NONE
include $(CONTIKI)/Makefile.include

