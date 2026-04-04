TARGET		:= opentyrian
TITLE_ID	:= TYRIAN001
CONTENT_ID	:= HG0000-$(TITLE_ID)_00-0000111122223333
CC      := ppu-gcc

PS3DEV:=/usr/local/ps3dev
PS3PORTLIBS:=$(PS3DEV)/portlibs/ppu
PATH:=$(PATH):$(PS3DEV)/bin:$(PS3DEV)/ppu/bin

PKG_DIR		:= "package"
DATA_SOURCE	:= "gamefiles"
USR_DIR		:= "$(PKG_DIR)/USRDIR"
EBOOT_DEST	:= "$(USR_DIR)/EBOOT.BIN"

INCLUDES	:= -I$(PSL1GHT)/ppu/include -I$(PS3PORTLIBS)/include -I$(PS3PORTLIBS)/include/SDL

CFLAGS		:= $(INCLUDES) -std=gnu99 \
				-DSDLK_FIRST=0 -DSDLK_LAST=323 -DLSB_FIRST=0 -DTYRIAN_DIR=\"/dev_hdd0/game/TYRIAN001/USRDIR/\" 

CFLAGS   += -Dippu -D__PS3__ -D_THREAD_SAFE

LIBS    := -L$(PS3PORTLIBS)/lib -L$(PSL1GHT)/ppu/lib \
           -lSDL_mixer -lSDL \
           -lio -lrt -lsysutil -lgcm_sys -lrsx -laudio -llv2 -lz -lm
		   
LDFLAGS += -Wl,--stack=0x100000
			   
SOURCES		:= $(wildcard src/*.c)
OBJS		:= $(SOURCES:.c=.o)

include $(PSL1GHT)/ppu_rules

all: $(TARGET).self

$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@
	rm -rf $(PKG_DIR)
	mkdir -p $(USR_DIR)
	cp "PARAM.SFO" "$(PKG_DIR)/PARAM.SFO"
	cp "PIC1.PNG" "$(PKG_DIR)/PIC1.PNG"
	cp "ICON0.PNG" "$(PKG_DIR)/ICON0.PNG"
	cp -r "$(DATA_SOURCE)"/* "$(USR_DIR)/"
	bash fixnames.sh
	
$(TARGET).self: $(TARGET).elf
	ppu-strip $< -o $<.strip
	sprxlinker $<.strip
	make_self $<.strip $@
	fself -n $<.strip EBOOT.BIN $(CONTENT_ID)
	cp EBOOT.BIN "$(EBOOT_DEST)"
	pkg.py --contentid $(CONTENT_ID) "$(PKG_DIR)/" opentyrian_ps3.pkg
	
clean:
	rm -rf "$PKG_DIR"
	mkdir -p "$USR_DIR"
	rm -f $(OBJS) $(TARGET).elf $(TARGET).elf.strip $(TARGET).self EBOOT.BIN opentyrian_ps3.pkg