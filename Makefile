TARGET= BeatBox
DEPENDS = main joystick gpio drum_machine player_wave wave queue bloader mixer_wave UI ui_jstick ui_udp udp 

CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

# Asound process:
# get alibsound2 lib on target:
# 	# apt-get install libasound2
# Copy target's /usr/lib/arm-linux-gnueabihf/libasound.so.2.0.0 
#      to host  $(PROJECT-DIR)/asound_lib_BBB/libasound.so
LINKDIR = asound_lib_BBB
LFLAGS = -L $(LINKDIR)


# -pg for supporting gprof profiling.
#CFLAGS += -pg


build: $(DEPENDS:=.o)
	$(CC_C) $(DEPENDS:=.o) -o $(TARGET) $(LFLAGS) -lpthread -lasound

%.o: %.c 
	$(CC_C) -c $(CFLAGS) $<

clean:
	rm *.o
	rm $(TARGET)