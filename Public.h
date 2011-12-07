/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011 James Nobis <quel@quelrod.net>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PUBLIC_H
#define _PUBLIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <list>

#include "global.h"

struct RainbowChainO
{
	uint64 nIndexS;
	uint64 nIndexE;
};

#if defined(_WIN32) && !defined(__GNUC__)
	#pragma warning(disable: 4201)
#endif
union RainbowChain
{
	uint64 nIndexS;
	struct
	{
		unsigned short foo[3];
		unsigned short nIndexE;
	};
	//int nCheckPoint;
};
#if defined(_WIN32) && !defined(__GNUC__)
	#pragma warning(default : 4201) 
#endif

struct RainbowChainCP
{
	uint64 nIndexS;
	uint64 nIndexE;
	unsigned short nCheckPoint;
};

struct IndexChain
{
	uint64 nPrefix;
	uint32 nFirstChain;
	uint32 nChainCount;
};

/* rcracki_mt IndexChain
#pragma pack(1)
union IndexChain
{
	uint64 nPrefix; //5
	struct
	{
		unsigned char foo[5];
		unsigned int nFirstChain; //4
		unsigned short nChainCount; //2
	};
	//unsigned short nChainCount; (maybe union with nPrefix, 1 byte spoiled, no pack(1) needed)
};
#pragma pack()
*/

struct IndexRow
{
	uint64 prefix;
	unsigned int prefixstart, numchains;
};

typedef struct
{
	std::string sName;
	int nPlainLenMin;
	int nPlainLenMax;
} tCharset;

#define MAX_PLAIN_LEN 256
#define MIN_HASH_LEN  8
#define MAX_HASH_LEN  256
#define MAX_SALT_LEN  256

// XXX nmap is GPL2, will check newer releases regarding license
// Code comes from nmap, used for the linux implementation of kbhit()
#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

int tty_getchar();
void tty_done();
void tty_init();
void tty_flush(void);
// end nmap code

#endif

#if defined(_WIN32) && !defined(__GNUC__)
	int gettimeofday( struct timeval *tv, struct timezone *tz );
#else
	#include <sys/time.h>
#endif

timeval sub_timeofday( timeval tv2, timeval tv );

long GetFileLen(FILE* file);
long GetFileLen(char* file);
long GetFileLen( std::string file );
uint8 getRTfileFormatId( std::string RTfileFormatName );
std::string TrimString( std::string s );
bool boinc_ReadLinesFromFile( std::string sPathName, std::vector<std::string>& vLine );
bool ReadLinesFromFile( std::string sPathName, std::vector<std::string>& vLine);
bool SeperateString( std::string s, std::string sSeperator, std::vector<std::string>& vPart);
std::string uint64tostr(uint64 n);
std::string uint64tohexstr(uint64 n);
std::string HexToStr(const unsigned char* pData, int nLen);
unsigned long GetAvailPhysMemorySize();
std::string GetApplicationPath();
void ParseHash( std::string sHash, unsigned char* pHash, int& nHashLen );
bool GetHybridCharsets( std::string sCharset, std::vector<tCharset>& vCharset );
void Logo();
bool writeResultLineToFile( std::string sOutputFile, std::string sHash, std::string sPlain, std::string sBinary );

#endif
