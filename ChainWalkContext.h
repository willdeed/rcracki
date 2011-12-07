/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011 James Nobis <quel@quelrod.net>
 * Copyright 2008, 2009, 2010, 2011 Steve Thomas (Sc00bz)
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CHAINWALKCONTEXT_H
#define _CHAINWALKCONTEXT_H

#include "HashRoutine.h"
#include "Public.h"

typedef struct 
{
	uint64 m_nPlainSpaceUpToX[MAX_PLAIN_LEN];		// Performance consideration
	//unsigned char m_PlainCharset[255];
	unsigned char m_PlainCharset[MAX_PLAIN_LEN];
	uint64 m_nPlainSpaceTotal;
	uint64 m_nIndexX;
	unsigned int m_nPlainCharsetLen;
	int m_nPlainLenMin;
	int m_nPlainLenMax;
	std::string m_sPlainCharsetName;
	std::string m_sPlainCharsetContent;
} stCharset;
class CChainWalkContext 
{
public:
	CChainWalkContext();
	virtual ~CChainWalkContext();

//private:
	static std::string m_sHashRoutineName;	
	static HASHROUTINE m_pHashRoutine;							// Configuration
	static int m_nHashLen;										// Configuration
	static uint8 RTfileFormatId;

	static std::vector<stCharset> m_vCharset;
	static int m_nPlainLenMinTotal, m_nPlainLenMaxTotal;
	static uint64 m_nPlainSpaceUpToX[MAX_PLAIN_LEN];		// Performance consideration
	static uint64 m_nPlainSpaceTotal;							// Performance consideration
	static int m_nHybridCharset;
	static int m_nRainbowTableIndex;							// Configuration
	static uint64 m_nReduceOffset;								// Performance consideration

	// Context
	uint64 m_nIndex;
	unsigned char m_Plain[MAX_PLAIN_LEN];
	int m_nPlainLen;
	unsigned char m_Hash[MAX_HASH_LEN];
	static unsigned char m_Salt[MAX_SALT_LEN];
	static int m_nSaltLen;
private:
	static bool LoadCharset( std::string sCharset );

public:
	static bool SetHashRoutine( std::string sHashRoutineName );
	static bool SetPlainCharset( std::string sCharsetName, int nPlainLenMin, int nPlainLenMax );
	static bool SetRainbowTableIndex(int nRainbowTableIndex);	
	static bool SetSalt(unsigned char *Salt, int nSaltLength);
	static bool SetupWithPathName( std::string sPathName, int& nRainbowChainLen, int& nRainbowChainCount );	// Wrapper
	static std::string GetHashRoutineName();
	static int GetHashLen();
	static std::string GetPlainCharsetName();
	static std::string GetPlainCharsetContent();
	static int GetPlainLenMin();
	static int GetPlainLenMax();
	static uint64 GetPlainSpaceTotal();
	static int GetRainbowTableIndex();
	static void Dump();
	static uint8 getRTfileFormat();

	void SetIndex(uint64 nIndex);
	void SetHash(unsigned char* pHash);		// The length should be m_nHashLen

	void IndexToPlain();
	int normalIndexToPlain(uint64 index, uint64 *plainSpaceUpToX, unsigned char *charSet, int charSetLen, int min, int max, unsigned char *plain);
	void PlainToHash();
	void HashToIndex(int nPos);

	uint64 GetIndex();
	const uint64* GetIndexPtr();
	std::string GetPlain();
	std::string GetBinary();
	std::string GetHash();
	bool CheckHash(unsigned char* pHash);	// The length should be m_nHashLen
};

#endif
