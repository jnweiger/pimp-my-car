# Makefile for project $(NAME)
# Distribute under GPLv2, use with care.
#
# 2011-12-25, jw@suse.de

NAME          = cardrive
CFILES        = drive.c # $(NAME).c eeprom.c i2c_slave_cb.c

CPU           = mega48
#CPU           = mega169
#CPU           = mega644

PROG_HW       = usbtiny		# www.ladyada.com
#PROG_HW       = usbasp		# www.fischl.de
#PROG_HW       = stk200		# www.atmel.com
#PROG_PORT     = /dev/ttyUSB0
#PROG_PORT     = /dev/rfcomm1
#AVRDUDE_OPT   = -b 57600 -P 

PROG_SW       = avrdude
#PROG_SW       = sudo avrdude	# recommended
#PROG_SW       = uisp		# deprecated

## use this, if $(ISP) magic fails
#UPLOAD_CMD    = $(PROG_SW) -p AT$(CPU) -c $(PROG_HW) $(AVRDUDE_OPT) $(addprefix -P,$(PROG_PORT)) -U $(NAME).hex -v

#CFLAGS        += -Os -g -Wall
#LDFLAGS       += -Wl,-u,vfprintf -lprintf_min

TOP_DIR       = .

include $(TOP_DIR)/avr_common.mk
DIST_EXCLUDE += --exclude photos

distclean:: 
	rm -f download* ee_data.* 

## header file dependencies
#############################
include depend.mk
