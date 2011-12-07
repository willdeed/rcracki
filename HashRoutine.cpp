/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011 James Nobis <quel@quelrod.net>
 *
 * This file is part of rcracki_mt.
 *
 * rcracki_mt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * rcracki_mt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "HashRoutine.h"
#include "HashAlgorithm.h"

//////////////////////////////////////////////////////////////////////

CHashRoutine::CHashRoutine()
{
	// Notice: MIN_HASH_LEN <= nHashLen <= MAX_HASH_LEN


	AddHashRoutine("lm",   HashLM,   8);
	AddHashRoutine("ntlm", HashNTLM, 16);
//	AddHashRoutine("md2",  HashMD2,  16);
	AddHashRoutine("md4",  HashMD4,  16);
	AddHashRoutine("md5",  HashMD5,  16);
	AddHashRoutine("doublemd5",  HashDoubleMD5,  16);
	AddHashRoutine("sha1", HashSHA1, 20);
//	AddHashRoutine("ripemd160", HashRIPEMD160, 20);
	AddHashRoutine("mysql323", HashMySQL323, 8);
	AddHashRoutine("mysqlsha1", HashMySQLSHA1, 20);
//	AddHashRoutine("ciscopix", HashPIX, 16);
//	AddHashRoutine("mscache", HashMSCACHE, 16);
	AddHashRoutine("halflmchall", HashHALFLMCHALL, 8);

	// Added from mao
	AddHashRoutine("lmchall", HashLMCHALL, 24);
	AddHashRoutine("ntlmchall", HashNTLMCHALL, 24);
//	AddHashRoutine("oracle", HashORACLE, 8);
}

CHashRoutine::~CHashRoutine()
{
}

void CHashRoutine::AddHashRoutine( std::string sHashRoutineName, HASHROUTINE pHashRoutine, int nHashLen )
{
	vHashRoutineName.push_back(sHashRoutineName);
	vHashRoutine.push_back(pHashRoutine);
	vHashLen.push_back(nHashLen);
}

std::string CHashRoutine::GetAllHashRoutineName()
{
	std::string sRet;
	uint32 i;
	for (i = 0; i < vHashRoutineName.size(); i++)
		sRet += vHashRoutineName[i] + " ";

	return sRet;
}

void CHashRoutine::GetHashRoutine( std::string sHashRoutineName, HASHROUTINE& pHashRoutine, int& nHashLen )
{
	uint32 i;
	for (i = 0; i < vHashRoutineName.size(); i++)
	{
		if (sHashRoutineName == vHashRoutineName[i])
		{
			pHashRoutine = vHashRoutine[i];
			nHashLen = vHashLen[i];
			return;
		}
	}

	pHashRoutine = NULL;
	nHashLen = 0;
}
