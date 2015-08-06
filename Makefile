VERSION = 0.1

TARGET	= gvimsurfer
NAME		= gVimSurfer
SRC      = $(wildcard src/*.c) 
HDR		= $(wildcard include/*.h)
OBJ  		= $(addprefix obj/,$(notdir $(SRC:.c=.o)))
CONFIG	= $(wildcard config/*.h)

# libs
GTK_INC = $(shell pkg-config --cflags gtk+-2.0 webkit-1.0 )
GTK_LIB = $(shell pkg-config --libs gtk+-2.0 gthread-2.0 webkit-1.0 )

CC = gcc
LFLAGS = -lc ${GTK_LIB} -lpthread -lm
CFLAGS = -std=c99 -pedantic -Wall -I./ -I/usr/include $(GTK_INC)
CFLAGS += -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -DTARGET=\"$(TARGET)\" -DNAME=\"$(NAME)\"

all: ${TARGET}

obj/%.o : src/%.c
	@echo " CC " $<
	@${CC} -c ${CFLAGS} -o $@ $<

$(OBJ) : $(HDR) $(CONFIG)

${TARGET} : ${OBJ}
	@echo " LD " -o $@
	@${CC} $^ ${LFLAGS} -o $@

clean:
	@echo " RM " ${TARGET}
	@rm -f ${TARGET}
	@echo " RM " ${OBJ}
	@rm -f ${OBJ}

info:
	@echo ${TARGET} build options:
	@echo "CC      = ${CC}"
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LIBS    = ${LFLAGS}"

.PHONY: all clean info
