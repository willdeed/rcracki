# rcracki_mt is a multithreaded implementation and fork of the original 
# RainbowCrack
#
# Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
# Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
# Copyright 2009, 2010, 2011 James Nobis <quel@quelrod.net>
#
# This file is part of rcracki_mt.
#
# rcracki_mt is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# rcracki_mt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.

SHELL = /bin/sh
BIN = $(DESTDIR)/usr/bin
CC = g++
# -mtune=native doesn't work on NetBSD 5.1 with gcc 4.1.3 and make (fine with gmake)
#  -mtune=native doesn't work on OSX either
#  add -mtune=native to OPTIMIZATION for better performance (if it works)
OPTIMIZATION = -O3
INCLUDES = -I.
CFLAGS = -Wall -ansi $(INCLUDES) $(OPTIMIZATION) -c $(DEBUG)
LFLAGS = -Wall -ansi -filt $(INCLUDES) $(OPTIMIZATION) $(DEBUG)
LIBS = -lcrypto -lpthread
OBJS = BaseRTReader.o ChainWalkContext.o ChainWalkSet.o CrackEngine.o fast_md5.o HashAlgorithm.o HashRoutine.o HashSet.o lm2ntlm.o fast_md4.o MemoryPool.o Public.o RainbowCrack.o rcrackiThread.o RTReader.o RTIReader.o RTI2Reader.o
COMMON_API_PATH = .
#sha1.o
WARNALL = -Wextra -Wunused-macros -Wunsafe-loop-optimizations -Wundef -Woverlength-strings -Wdisabled-optimization -Wformat-extra-args -Wformat-security -Winline

OSNAME = $(shell uname -s)
# apparently --string-debug works on Linux, OpenBSD, NetBSD, and FreeBSD
# but not on OSX so to the short name -S
STRIP = $(shell which strip) -S

#LIBS_Darwin = -lcrypto
LIBS_NetBSD = -ldes
#LIBS_OpenBSD = -lcrypto

LIBS += ${LIBS_$(OSNAME)}

RELEASE_DIR = rcracki_mt_0.6.6_src

all: rcracki_mt strip

rcracki_mt: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o rcracki_mt

clean:
	rm -f core *.o *.gcno *.gcda *.gcov rcracki_mt rcracki.session \
		rcracki.progress  rcracki.precalc.0 rcracki.precalc.0.index

debug: DEBUG += -DDEBUG -g
debug: rcracki_mt

debugall: DEBUG += -DDEBUG -g $(WARNALL)
debugall: rcracki_mt

dumpasmall: DEBUG += -S -fverbose-asm
dumpasmall: rcracki_mt

dumpasmprofileuseall: DEBUG += -S -fverbose-asm
dumpasmprofileuseall: profileuseall

debugprofileuseall: DEBUG += -fprofile-use
debugprofileuseall: debugall

m32: DEBUG += -m32
m32: rcracki_mt

m32debug: DEBUG += -m32
m32debug: debug

m32debugall: DEBUG += -m32
m32debugall: debugll

m32dumpasmall: DEBUG += -m32
m32dumpasmall: dumpasmall

m32dumpasmprofileuseall: DEBUG += -S -fverbose-asm -m32
m32dumpasmprofileuseall: profileuseall

m32profilegenall: DEBUG += -m32
m32profilegenall: profilegenall

m32profileuseall: DEBUG += -m32
m32profileuseall: profileuseall

profilegenall: DEBUG += -fprofile-generate $(WARNALL)
profilegenall: rcracki_mt

profileuseall: DEBUG += -fprofile-use $(WARNALL)
profileuseall: rcracki_mt

gcovall: DEBUG += -DDEBUG -g -fprofile-arcs -ftest-coverage
gcovall: rcracki_mt

gprofall: DEBUG += -DDEBUG -g -pg
gprofall: rcracki_mt

install:
	install -d $(BIN)
	install --group=root --owner=root --mode=755 rcracki_mt $(BIN)

release: clean
	mkdir $(RELEASE_DIR)
	cp $(COMMON_API_PATH)/BaseRTReader.* $(COMMON_API_PATH)/ChainWalkContext.* \
		$(COMMON_API_PATH)/ChainWalkSet.* CrackEngine.* \
		$(COMMON_API_PATH)/fast_md5.* \
		$(COMMON_API_PATH)/HashAlgorithm.h HashAlgorithm.cpp HashRoutine.cpp \
		$(COMMON_API_PATH)/HashRoutine.h HashSet.* $(COMMON_API_PATH)/global.h \
		lm2ntlm.* $(COMMON_API_PATH)/fast_md4.* $(COMMON_API_PATH)/MemoryPool.* \
		$(COMMON_API_PATH)/Public.* RainbowCrack.* rcrackiThread.* \
		$(COMMON_API_PATH)/RTI2Common.* \
		$(COMMON_API_PATH)/RT*Reader.* sha1.* ChangeLog.txt charset.txt COPYING \
		INSTALLING.txt README.txt TODO rcracki_mt.ini $(RELEASE_DIR)/
	cat Makefile | sed 's/..\/..\/common\/rt_api/./' > $(RELEASE_DIR)/Makefile

rebuild: clean all

static: DEBUG += -static -static-libgcc
static: all

strip:
	$(STRIP) rcracki_mt

uninstall:
	rm -f $(BIN)/rcracki_mt

BaseRTReader.o: $(COMMON_API_PATH)/BaseRTReader.h \
	$(COMMON_API_PATH)/BaseRTReader.cpp
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/BaseRTReader.cpp

ChainWalkContext.o: $(COMMON_API_PATH)/ChainWalkContext.h \
	$(COMMON_API_PATH)/ChainWalkContext.cpp $(COMMON_API_PATH)/HashRoutine.h \
	$(COMMON_API_PATH)/Public.h
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/ChainWalkContext.cpp

ChainWalkSet.o: $(COMMON_API_PATH)/Public.h $(COMMON_API_PATH)/ChainWalkSet.h \
	$(COMMON_API_PATH)/ChainWalkSet.cpp
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/ChainWalkSet.cpp

CrackEngine.o: $(COMMON_API_PATH)/Public.h HashSet.h \
	$(COMMON_API_PATH)/ChainWalkContext.h $(COMMON_API_PATH)/MemoryPool.h \
	$(COMMON_API_PATH)/ChainWalkSet.h rcrackiThread.h \
	$(COMMON_API_PATH)/RTReader.h $(COMMON_API_PATH)/RTIReader.h \
	$(COMMON_API_PATH)/RTI2Reader.h CrackEngine.h CrackEngine.cpp
	$(CC) $(CFLAGS) CrackEngine.cpp

fast_md5.o: $(COMMON_API_PATH)/fast_md5.h $(COMMON_API_PATH)/fast_md5.cpp \
	$(COMMON_API_PATH)/global.h
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/fast_md5.cpp

#HashAlgorithm.o: $(COMMON_API_PATH)/HashAlgorithm.h HashAlgorithm.cpp \
	$(COMMON_API_PATH)/Public.h $(COMMON_API_PATH)/fast_md5.h \
	$(COMMON_API_PATH)/fast_md4.h sha1.h
HashAlgorithm.o: $(COMMON_API_PATH)/HashAlgorithm.h HashAlgorithm.cpp \
	$(COMMON_API_PATH)/Public.h $(COMMON_API_PATH)/fast_md5.h \
	$(COMMON_API_PATH)/fast_md4.h
	$(CC) $(CFLAGS) HashAlgorithm.cpp

HashRoutine.o: $(COMMON_API_PATH)/HashRoutine.h HashRoutine.cpp \
	$(COMMON_API_PATH)/global.h $(COMMON_API_PATH)/HashAlgorithm.h
	$(CC) $(CFLAGS) HashRoutine.cpp

HashSet.o: HashSet.h HashSet.cpp $(COMMON_API_PATH)/Public.h
	$(CC) $(CFLAGS) HashSet.cpp

lm2ntlm.o: lm2ntlm.h lm2ntlm.cpp $(COMMON_API_PATH)/Public.h \
	$(COMMON_API_PATH)/fast_md4.h
	$(CC) $(CFLAGS) lm2ntlm.cpp

fast_md4.o: $(COMMON_API_PATH)/fast_md4.h $(COMMON_API_PATH)/fast_md4.cpp \
	$(COMMON_API_PATH)/global.h
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/fast_md4.cpp

MemoryPool.o: $(COMMON_API_PATH)/MemoryPool.h \
	$(COMMON_API_PATH)/MemoryPool.cpp $(COMMON_API_PATH)/Public.h \
	$(COMMON_API_PATH)/global.h
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/MemoryPool.cpp

Public.o: $(COMMON_API_PATH)/Public.h $(COMMON_API_PATH)/Public.cpp \
	$(COMMON_API_PATH)/global.h
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/Public.cpp

RainbowCrack.o: RainbowCrack.cpp CrackEngine.h lm2ntlm.h
	$(CC) $(CFLAGS) RainbowCrack.cpp

rcrackiThread.o: rcrackiThread.h rcrackiThread.cpp \
	$(COMMON_API_PATH)/ChainWalkContext.h $(COMMON_API_PATH)/Public.h HashSet.h
	$(CC) $(CFLAGS) rcrackiThread.cpp

RTReader.o: $(COMMON_API_PATH)/RTReader.h $(COMMON_API_PATH)/RTReader.cpp
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/RTReader.cpp

RTIReader.o: $(COMMON_API_PATH)/RTIReader.h $(COMMON_API_PATH)/RTIReader.cpp
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/RTIReader.cpp

RTI2Reader.o: $(COMMON_API_PATH)/RTI2Reader.h \
	$(COMMON_API_PATH)/RTI2Reader.cpp $(COMMON_API_PATH)/BaseRTReader.h
	$(CC) $(CFLAGS) $(COMMON_API_PATH)/RTI2Reader.cpp

#sha1.o: sha1.h sha1.cpp $(COMMON_API_PATH)/global.h
#	 $(CC) $(CFLAGS) sha1.cpp
