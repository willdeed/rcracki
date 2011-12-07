/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
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

#include "rcrackiThread.h"

// create job for pre-calculation
rcrackiThread::rcrackiThread(unsigned char* TargetHash, int thread_id, int nRainbowChainLen, int thread_count, uint64* pStartPosIndexE)
{
	t_TargetHash = TargetHash;
	t_nRainbowChainLen = nRainbowChainLen;
	t_ID = thread_id;
	t_count = thread_count;
	t_pStartPosIndexE = pStartPosIndexE;
	t_nChainWalkStep = 0;
	falseAlarmChecker = false;
	falseAlarmCheckerO = false;
}

// create job for false alarm checking
rcrackiThread::rcrackiThread(unsigned char* pHash, bool oldFormat)
{
	falseAlarmChecker = true;
	falseAlarmCheckerO = oldFormat;
	t_pChainsFound.clear();
	t_nGuessedPoss.clear();
	t_pHash = pHash;
	t_nChainWalkStepDueToFalseAlarm = 0;
	t_nFalseAlarm = 0;
	foundHash = false;
}

void rcrackiThread::AddAlarmCheck(RainbowChain* pChain, int nGuessedPos)
{
	t_pChainsFound.push_back(pChain);
	t_nGuessedPoss.push_back(nGuessedPos);
}

void rcrackiThread::AddAlarmCheckO(RainbowChainO* pChain, int nGuessedPos)
{
	t_pChainsFoundO.push_back(pChain);
	t_nGuessedPoss.push_back(nGuessedPos);
}

// Windows (beginthreadex) way of threads
//unsigned __stdcall rcrackiThread::rcrackiThreadStaticEntryPoint(void * pThis)
//{
//	rcrackiThread* pTT = (rcrackiThread*)pThis;
//	pTT->rcrackiThreadEntryPoint();
//	_endthreadex( 2 );
//	return 2;
//}

// entry point for the posix thread
void * rcrackiThread::rcrackiThreadStaticEntryPointPthread(void * pThis)
{
	rcrackiThread* pTT = (rcrackiThread*)pThis;
	pTT->rcrackiThreadEntryPoint();
	pthread_exit(NULL);
	return NULL;
}

// start processing of jobs
void rcrackiThread::rcrackiThreadEntryPoint()
{
	if (falseAlarmChecker) {
		if (falseAlarmCheckerO) {
			CheckAlarmO();
		}
		else {
			CheckAlarm();
		}
	}
	else {
		PreCalculate();
	}
}

uint64 rcrackiThread::GetIndex(int nPos)
{
	uint64 t_index = t_vStartPosIndexE[nPos - t_ID];
	return t_index;
}

int rcrackiThread::GetChainWalkStep()
{
	return t_nChainWalkStep;
}

int rcrackiThread::GetIndexCount()
{
	return t_vStartPosIndexE.size();
}

rcrackiThread::~rcrackiThread(void)
{
}

void rcrackiThread::PreCalculate()
{
	//XXX is this correct for multiple threads?
	for (t_nPos = t_nRainbowChainLen - 2 - t_ID; t_nPos >= 0; t_nPos -= t_count)
	{
		t_cwc.SetHash(t_TargetHash);
		t_cwc.HashToIndex(t_nPos);
		int i;
		for (i = t_nPos + 1; i <= t_nRainbowChainLen - 2; i++)
		{
			t_cwc.IndexToPlain();
			t_cwc.PlainToHash();
			t_cwc.HashToIndex(i);
		}
		t_pStartPosIndexE[t_nPos] = t_cwc.GetIndex();
		t_nChainWalkStep += t_nRainbowChainLen - 2 - t_nPos;
	}
}

void rcrackiThread::CheckAlarm()
{
	uint32 i;
	for (i = 0; i < t_pChainsFound.size(); i++)
	{
		RainbowChain* t_pChain = t_pChainsFound[i];
		int t_nGuessedPos = t_nGuessedPoss[i];		
		
		CChainWalkContext cwc;
		//uint64 nIndexS = t_pChain->nIndexS & 0x0000FFFFFFFFFFFF; // for first 6 bytes
		//uint64 nIndexS = t_pChain->nIndexS >> 16;
		uint64 nIndexS = t_pChain->nIndexS & 0x0000FFFFFFFFFFFFULL; // for first 6 bytes
		cwc.SetIndex(nIndexS);
		//cwc.SetIndex(t_pChain->nIndexS);	
		int nPos;
		for (nPos = 0; nPos < t_nGuessedPos; nPos++)
		{
			cwc.IndexToPlain();
			cwc.PlainToHash();
			cwc.HashToIndex(nPos);
		}
		cwc.IndexToPlain();
		cwc.PlainToHash();
		if (cwc.CheckHash(t_pHash))
		{
			t_Hash = cwc.GetHash();
			t_Plain = cwc.GetPlain();
			t_Binary = cwc.GetBinary();

			foundHash = true;
			break;
		}
		else {
			foundHash = false;
			t_nChainWalkStepDueToFalseAlarm += t_nGuessedPos + 1;
			t_nFalseAlarm++;
		}
	}
}

void rcrackiThread::CheckAlarmO()
{
	uint32 i;
	for (i = 0; i < t_pChainsFoundO.size(); i++)
	{
		RainbowChainO* t_pChain = t_pChainsFoundO[i];
		int t_nGuessedPos = t_nGuessedPoss[i];		
		
		CChainWalkContext cwc;

		uint64 nIndexS = t_pChain->nIndexS;
		cwc.SetIndex(nIndexS);

		int nPos;
		for (nPos = 0; nPos < t_nGuessedPos; nPos++)
		{
			cwc.IndexToPlain();
			cwc.PlainToHash();
			cwc.HashToIndex(nPos);
		}
		cwc.IndexToPlain();
		cwc.PlainToHash();
		if (cwc.CheckHash(t_pHash))
		{
			t_Hash = cwc.GetHash();
			t_Plain = cwc.GetPlain();
			t_Binary = cwc.GetBinary();

			foundHash = true;
			break;
		}
		else {
			foundHash = false;
			t_nChainWalkStepDueToFalseAlarm += t_nGuessedPos + 1;
			t_nFalseAlarm++;
		}
	}
}

bool rcrackiThread::FoundHash()
{
	return foundHash;
}

int rcrackiThread::GetChainWalkStepDueToFalseAlarm()
{
	return t_nChainWalkStepDueToFalseAlarm;
}

int rcrackiThread::GetnFalseAlarm()
{
	return t_nFalseAlarm;
}

std::string rcrackiThread::GetHash()
{
	return t_Hash;
}

std::string rcrackiThread::GetPlain()
{
	return t_Plain;
}

std::string rcrackiThread::GetBinary()
{
	return t_Binary;
}
