VERSION = 0.4b

TARGET	= gvimsurfer
NAME		= gVimSurfer
PREFIX  ?= /usr
SRC      = $(wildcard src/*.c) 
HDR		= $(wildcard include/*.h)
OBJ  		= $(addprefix obj/,$(notdir $(SRC:.c=.o)))
CONFIG	= $(wildcard config/*.h)

# libs
GTK_INC = $(shell pkg-config --cflags gtk+-2.0 webkit-1.0 )
GTK_LIB = $(shell pkg-config --libs gtk+-2.0 webkit-1.0 )

CC = gcc
LFLAGS = -lc ${GTK_LIB} -lm
CFLAGS = -std=c99 -pedantic -Wall -I./ -I/usr/include ${GTK_INC}
CFLAGS += -DVERSION=\"${VERSION}\" -D_XOPEN_SOURCE=600 -DTARGET=\"${TARGET}\" -DNAME=\"${NAME}\"

all: ${TARGET}

obj/%.o : src/%.c
	@if [ ! -d obj/ ]; then mkdir -p obj/; fi
	@echo " CC " $<
	@${CC} -c ${CFLAGS} -o $@ $<

${OBJ} : ${HDR} ${CONFIG}

${TARGET} : ${OBJ}
	@echo " LD " -o $@
	@${CC} $^ ${LFLAGS} -o $@

clean:
	@echo " RM " ${TARGET}
	@rm -f ${TARGET}
	@echo " RM " ${OBJ}
	@rm -f ${OBJ}
	@rm -rf obj/

info:
	@echo ${TARGET}-${VERSION} build options:
	@echo "CC      = ${CC}"
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LIBS    = ${LFLAGS}"

install: all
	@echo installing to ${DESTDIR}${PREFIX}...
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${TARGET} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${TARGET}
	@mkdir -p ${DESTDIR}${PREFIX}/share/${TARGET}
	@cp -f setup.sh configrc script.js ${DESTDIR}${PREFIX}/share/${TARGET}
	@chmod 755 ${DESTDIR}${PREFIX}/share/${TARGET}/setup.sh

uninstall:
	@echo uninstalling...
	@rm -f ${DESTDIR}${PREFIX}/bin/${TARGET}
	@rm -f ${DESTDIR}${PREFIX}/share/${TARGET}/*
	@rm -rf ${DESTDIR}${REFIX}/share/${TARGET}

.PHONY: all clean info
