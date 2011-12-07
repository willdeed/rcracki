/*
 * rcracki_mt is a multithreaded implementation and fork of the original
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright 2009, 2010 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011 James Nobis <quel@quelrod.net>
 * Copyright 2010 uroskn
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

#include "CrackEngine.h"
#include "RTReader.h"
#include "RTIReader.h"
#include "RTI2Reader.h"

#ifndef _WIN32
	#include <sys/resource.h>
#endif

CCrackEngine::CCrackEngine()
{
	ResetStatistics();
	writeOutput = false;
	resumeSession = false;
	debug = false;
	keepPrecalcFiles = false;

	sSessionPathName = "";
	sProgressPathName = "";
}

CCrackEngine::~CCrackEngine()
{
}

//////////////////////////////////////////////////////////////////////

void CCrackEngine::ResetStatistics()
{
	m_fTotalDiskAccessTime               = 0.0f;
	m_fTotalCryptanalysisTime            = 0.0f;
	m_fTotalPrecalculationTime           = 0.0f;
	m_nTotalChainWalkStep                = 0;
	m_nTotalFalseAlarm                   = 0;
	m_nTotalChainWalkStepDueToFalseAlarm = 0;
//	m_nTotalFalseAlarmSkipped			 = 0;
}

int CCrackEngine::BinarySearchOld(RainbowChainO* pChain, int nRainbowChainCount, uint64 nIndex)
{
	int nLow = 0;
	int nHigh = nRainbowChainCount - 1;
	while (nLow <= nHigh)
	{
		int nMid = (nLow + nHigh) / 2;
		if (nIndex == pChain[nMid].nIndexE)
			return nMid;
		else if (nIndex < pChain[nMid].nIndexE)
			nHigh = nMid - 1;
		else
			nLow = nMid + 1;
	}

	return -1;
}

RainbowChain *CCrackEngine::BinarySearch(RainbowChain *pChain, int nChainCountRead, uint64 nIndex, RTIrcrackiIndexChain *pIndex, int nIndexSize, int nIndexStart)
{
	uint64 nPrefix = nIndex >> 16;
	int nLow, nHigh;
	bool found = false;
	//int nChains = 0;

	if(nPrefix > (pIndex[nIndexSize-1].nPrefix & 0x000000FFFFFFFFFFULL)) // check if its in the index file
	{
		return NULL;
	}

	int nBLow = 0;
	int nBHigh = nIndexSize - 1;
	while (nBLow <= nBHigh)
	{
		int nBMid = (nBLow + nBHigh) / 2;
		if (nPrefix == (pIndex[nBMid].nPrefix & 0x000000FFFFFFFFFFULL))
		{
			//nLow = nChains;
			//int nChains = 0;

			nLow = pIndex[nBMid].nFirstChain;
			nHigh = nLow + pIndex[nBMid].nChainCount;
			if(nLow >= nIndexStart && nLow <= nIndexStart + nChainCountRead)
			{
				if(nHigh > nIndexStart + nChainCountRead)
					nHigh = nIndexStart + nChainCountRead;
			}
			else if(nLow < nIndexStart && nHigh >= nIndexStart)
			{
				nLow = nIndexStart;
			}
			else break;
			found = true;
			break;
		}
		else if (nPrefix < (pIndex[nBMid].nPrefix & 0x000000FFFFFFFFFFULL))
			nBHigh = nBMid - 1;
		else
			nBLow = nBMid + 1;
	}
	if(found == true)
	{
		for(int i = nLow - nIndexStart; i < nHigh - nIndexStart; i++)
		{
			int nSIndex = ((int)nIndex) & 0x0000FFFF;

			if (nSIndex == pChain[i].nIndexE)
			{
				return &pChain[i];
			}
			else if(pChain[i].nIndexE > nSIndex)
				break;
		}
	}
	return NULL;
}

// not used currently, leaving code for future checkpoints
//bool CCrackEngine::CheckAlarm(RainbowChain* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs)
//{
//	CChainWalkContext cwc;
//	//uint64 nIndexS = pChain->nIndexS >> 16;
//	uint64 nIndexS = pChain->nIndexS & 0x0000FFFFFFFFFFFFULL; // for first 6 bytes
//	cwc.SetIndex(nIndexS);
//	int nPos;
//	for (nPos = 0; nPos < nGuessedPos; nPos++)
//	{
//		cwc.IndexToPlain();
//		cwc.PlainToHash();
//		cwc.HashToIndex(nPos);
//		// Not using checkpoints atm
//		/*
//		switch(nPos)
//		{
//		case 5000:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000080) >> 7)
//				{
//					m_nTotalFalseAlarmSkipped += 10000 - 5000;
////					printf("CheckPoint caught false alarm at position 7600\n");
//					return false;
//				}
//				break;
//		case 6000:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000040) >> 6)
//				{
////					printf("CheckPoint caught false alarm at position 8200\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 6000;
//					return false;
//				}
//				break;
//
//		case 7600:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000020) >> 5)
//				{
////					printf("CheckPoint caught false alarm at position 8700\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 7600;
//					return false;
//				}
//				break;
//
//		case 8200:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000010) >> 4)
//				{
////					printf("CheckPoint caught false alarm at position 9000\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 8200;
//					return false;
//				}
//				break;
//
//			case 8700:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000008) >> 3)
//				{
////					printf("CheckPoint caught false alarm at position 9300\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 8700;
//					return false;
//				}
//
//				break;
//			case 9000:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000004) >> 2)
//				{
////					printf("CheckPoint caught false alarm at position 9600\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 9000;
//					return false;
//				}
//
//				break;
//			case 9300:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000002) >> 1)
//				{
////					printf("CheckPoint caught false alarm at position 9600\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 9300;
//					return false;
//				}
//				break;
//			case 9600:
//				if((cwc.GetIndex() & 0x00000001) != (pChain->nCheckPoint & 0x00000001))
//				{
////					printf("CheckPoint caught false alarm at position 9600\n");
//					m_nTotalFalseAlarmSkipped += 10000 - 9600;
//					return false;
//				}
//				break;
//
//		}*/
//	}
//	cwc.IndexToPlain();
//	cwc.PlainToHash();
//	if (cwc.CheckHash(pHash))
//	{
//		printf("plaintext of %s is %s\n", cwc.GetHash().c_str(), cwc.GetPlain().c_str());
//		hs.SetPlain(cwc.GetHash(), cwc.GetPlain(), cwc.GetBinary());
//		return true;
//	}
//
//	return false;
//}

//bool CCrackEngine::CheckAlarmOld(RainbowChainO* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs)
//{
//	CChainWalkContext cwc;
//	cwc.SetIndex(pChain->nIndexS);
//	int nPos;
//	for (nPos = 0; nPos < nGuessedPos; nPos++)
//	{
//		cwc.IndexToPlain();
//		cwc.PlainToHash();
//		cwc.HashToIndex(nPos);
//	}
//	cwc.IndexToPlain();
//	cwc.PlainToHash();
//	if (cwc.CheckHash(pHash))
//	{
//		printf("plaintext of %s is %s\n", cwc.GetHash().c_str(), cwc.GetPlain().c_str());
//		hs.SetPlain(cwc.GetHash(), cwc.GetPlain(), cwc.GetBinary());
//		return true;
//	}
//
//	return false;
//}

void CCrackEngine::GetChainIndexRangeWithSameEndpoint(RainbowChainO* pChain,
													  int nRainbowChainCount,
													  int nMatchingIndexE,
													  int& nMatchingIndexEFrom,
													  int& nMatchingIndexETo)
{
	nMatchingIndexEFrom = nMatchingIndexE;
	nMatchingIndexETo   = nMatchingIndexE;
	while (nMatchingIndexEFrom > 0)
	{
		if (pChain[nMatchingIndexEFrom - 1].nIndexE == pChain[nMatchingIndexE].nIndexE)
			nMatchingIndexEFrom--;
		else
			break;
	}
	while (nMatchingIndexETo < nRainbowChainCount - 1)
	{
		if (pChain[nMatchingIndexETo + 1].nIndexE == pChain[nMatchingIndexE].nIndexE)
			nMatchingIndexETo++;
		else
			break;
	}
}

void CCrackEngine::SearchTableChunkOld(RainbowChainO* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs)
{
	std::vector<std::string> vHash;
	hs.GetLeftHashWithLen(vHash, CChainWalkContext::GetHashLen());
	printf("searching for %lu hash%s...\n", (unsigned long)vHash.size(),
										   vHash.size() > 1 ? "es" : "");

	int nChainWalkStep = 0;
	int nFalseAlarm = 0;
	int nChainWalkStepDueToFalseAlarm = 0;

	std::vector<rcrackiThread*> threadPool;
	std::vector<pthread_t> pThreads;

	#ifndef _WIN32
		/*
		 * On linux you cannot set the priority of a thread in the non real time
		 * scheduling groups.  You can set the priority of the process.  In
		 * windows BELOW_NORMAL represents a 1/8th drop in priority and this would
		 * be 20 * 1/8 on linux or about 2.5
		 */
		setpriority( PRIO_PROCESS, 0, 2 );
	#endif

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	#ifdef _WIN32
	sched_param param;
	/*
	 * windows scheduling is 0 to 32 (low to high) with 8 as normal and 7 as
	 * BELOW_NORMAL
	 */
	param.sched_priority = THREAD_PRIORITY_BELOW_NORMAL;
	pthread_attr_setschedparam (&attr, &param);
	#endif
	// else set it to 5 or something (for linux)?

	bool pausing = false;

	uint32 nHashIndex;
	for (nHashIndex = 0; nHashIndex < vHash.size(); nHashIndex++)
	{
		#ifdef _WIN32
		if (_kbhit())
		{
			int ch = _getch();
			ch = toupper(ch);
			if (ch == 'P')
			{
				pausing = true;
				printf( "\nPausing, press P again to continue... ");

				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if (_kbhit())
					{
						ch = _getch();
						ch = toupper(ch);
						if (ch == 'P')
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					Sleep(500);
				}
			}
			else
			{
				printf( "\nPress 'P' to pause...\n");
			}
		}
		#else
		int c = tty_getchar();
		if (c >= 0) {
			tty_flush();
			if (c==112) { // = p
				pausing = true;
				printf( "\nPausing, press 'p' again to continue... ");

				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if ((c = tty_getchar()) >= 0)
					{
						tty_flush();
						if (c == 112)
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					usleep(500*1000);
				}
			}
			else {
				printf( "\nPress 'p' to pause...\n");
			}
		}
		#endif
		unsigned char TargetHash[MAX_HASH_LEN];
		int nHashLen;
		ParseHash(vHash[nHashIndex], TargetHash, nHashLen);
		if (nHashLen != CChainWalkContext::GetHashLen())
			printf("Debug: nHashLen mismatch\n");

		// Request ChainWalk
		bool fNewlyGenerated;
		uint64* pStartPosIndexE = m_cws.RequestWalk(TargetHash,
													nHashLen,
													CChainWalkContext::GetHashRoutineName(),
													CChainWalkContext::GetPlainCharsetName(),
													CChainWalkContext::GetPlainLenMin(),
													CChainWalkContext::GetPlainLenMax(),
													CChainWalkContext::GetRainbowTableIndex(),
													nRainbowChainLen,
													fNewlyGenerated,
													debug,
													sPrecalcPathName);
//		printf("Debug: using %s walk for %s\n", fNewlyGenerated ? "newly generated" : "existing",
//					vHash[nHashIndex].c_str());

		if (fNewlyGenerated)
		{
			timeval tv;
			timeval tv2;
			timeval final;

			gettimeofday( &tv, NULL );

			printf("Pre-calculating hash %lu of %lu.%-20s\r",
				(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");
			threadPool.clear();
			pThreads.clear();

			uint32 thread_ID;
			for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
			{
				rcrackiThread* r_Thread = new rcrackiThread(TargetHash, thread_ID, nRainbowChainLen, maxThreads, pStartPosIndexE);
				if (r_Thread)
				{
					pthread_t pThread;
					int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

					if( returnValue != 0 )
					{
						printf("pThread creation failed, returnValue: %d\n", returnValue);
					}
					else
					{
						pThreads.push_back(pThread);
					}

					threadPool.push_back(r_Thread);
				}
				else
				{
					printf("r_Thread creation failed!\n");
				}
			}

			//printf("%d r_Threads created\t\t\n", threadPool.size());

			for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
			{
				pthread_t pThread = pThreads[thread_ID];
				int returnValue = pthread_join(pThread, NULL);
				if( returnValue != 0 )
				{
					printf("pThread join failed, returnValue: %d\n", returnValue);
				}

				rcrackiThread* rThread = threadPool[thread_ID];
				nChainWalkStep += rThread->GetChainWalkStep();
				delete rThread;
			}

			m_cws.StoreToFile(pStartPosIndexE, TargetHash, nHashLen);
			gettimeofday( &tv2, NULL );
			final = sub_timeofday( tv2, tv );

			float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;

			m_fTotalPrecalculationTime += fTime;
			m_fTotalCryptanalysisTime -= fTime;

			printf("%-50s\r", "");

			if ( debug )
				printf("Debug: pre-calculation time: %.2f s\n", fTime);
		}

		threadPool.clear();
		pThreads.clear();

		printf("Checking false alarms for hash %lu of %lu.%-20s\r",
			(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");

		int i;
		for (i = 0; i < maxThreads; i++)
		{
			rcrackiThread* r_Thread = new rcrackiThread(TargetHash, true);
			threadPool.push_back(r_Thread);
		}

		uint32 thread_ID = 0;
		int nPos;
		for (nPos = nRainbowChainLen - 2; nPos >= 0; nPos--)
		{
			uint64 nIndexEOfCurPos = pStartPosIndexE[nPos];

			// Search matching nIndexE
			int nMatchingIndexE = BinarySearchOld(pChain, nRainbowChainCount, nIndexEOfCurPos);
			if (nMatchingIndexE != -1)
			{
				int nMatchingIndexEFrom, nMatchingIndexETo;
				GetChainIndexRangeWithSameEndpoint(pChain, nRainbowChainCount,
												   nMatchingIndexE,
												   nMatchingIndexEFrom, nMatchingIndexETo);
				int i;
				for (i = nMatchingIndexEFrom; i <= nMatchingIndexETo; i++)
				{
					rcrackiThread* rThread = threadPool[thread_ID];
					rThread->AddAlarmCheckO(pChain + i, nPos);
					if (thread_ID < (unsigned long)maxThreads - 1 ) {
						thread_ID++;
					} else {
						thread_ID = 0;
					}
				}
			}
		}

		for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
		{
			rcrackiThread* r_Thread = threadPool[thread_ID];
			pthread_t pThread;

			int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

			if( returnValue != 0 )
			{
				printf("pThread creation failed, returnValue: %d\n", returnValue);
			}
			else
			{
				pThreads.push_back(pThread);
			}
		}

		//printf("%d r_Threads created\t\t\n", threadPool.size());

		bool foundHashInThread = false;
		for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
		{
			rcrackiThread* rThread = threadPool[thread_ID];
			pthread_t pThread = pThreads[thread_ID];

			int returnValue = pthread_join(pThread, NULL);
			if( returnValue != 0 )
			{
				printf("pThread join failed, returnValue: %d\n", returnValue);
			}

			nChainWalkStepDueToFalseAlarm += rThread->GetChainWalkStepDueToFalseAlarm();
			nFalseAlarm += rThread->GetnFalseAlarm();

			if (rThread->FoundHash() && !foundHashInThread) {
				printf("%-50s\r", "");

				printf("plaintext of %s is %s\n", rThread->GetHash().c_str(), rThread->GetPlain().c_str());
				if (writeOutput)
				{
					if (!writeResultLineToFile(outputFile, rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary()))
						printf("Couldn't write this result to file!\n");
				}
				hs.SetPlain(rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary());

				FILE* file;
				
				if ( ( file = fopen( sSessionPathName.c_str(), "a" ) ) != NULL )
				{
					std::string buffer = "sHash=" + rThread->GetHash() + ":" + rThread->GetBinary() + ":" + rThread->GetPlain() + "\n";
					fputs (buffer.c_str(), file);
					fclose (file);
				}

				m_cws.DiscardWalk(pStartPosIndexE);
				foundHashInThread = true;
			}
			//pthread
			delete rThread;
		}

		pThreads.clear();
		threadPool.clear();
	}

	printf("%-50s\r", "");
	pThreads.clear();
	threadPool.clear();
	pthread_attr_destroy(&attr);

	if ( debug )
	{
		std::cout << "Debug: chain walk step: " << nChainWalkStep << std::endl;
		std::cout << "Debug: false alarm: " << nFalseAlarm << std::endl;
		std::cout << "Debug: chain walk step due to false alarm: "
			<< nChainWalkStepDueToFalseAlarm << std::endl;
	}

	m_nTotalChainWalkStep += nChainWalkStep;
	m_nTotalFalseAlarm += nFalseAlarm;
	m_nTotalChainWalkStepDueToFalseAlarm += nChainWalkStepDueToFalseAlarm;
}

void CCrackEngine::SearchTableChunk(RainbowChain* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs, RTIrcrackiIndexChain *pIndex, int nIndexSize, int nChainStart)
{
	std::vector<std::string> vHash;
	hs.GetLeftHashWithLen(vHash, CChainWalkContext::GetHashLen());
	printf("searching for %lu hash%s...\n", (unsigned long)vHash.size(),
										   vHash.size() > 1 ? "es" : "");

	int nChainWalkStep = 0;
	int nFalseAlarm = 0;
	int nChainWalkStepDueToFalseAlarm = 0;

	std::vector<rcrackiThread*> threadPool;
	std::vector<pthread_t> pThreads;

	#ifndef _WIN32
		/*
		 * On linux you cannot set the priority of a thread in the non real time
		 * scheduling groups.  You can set the priority of the process.  In
		 * windows BELOW_NORMAL represents a 1/8th drop in priority and this would
		 * be 20 * 1/8 on linux or about 2.5
		 */
		setpriority( PRIO_PROCESS, 0, 2 );
	#endif

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	#ifdef _WIN32
	sched_param param;
	/*
	 * windows scheduling is 0 to 32 (low to high) with 8 as normal and 7 as
	 * BELOW_NORMAL
	 */
	param.sched_priority = THREAD_PRIORITY_BELOW_NORMAL;
	pthread_attr_setschedparam (&attr, &param);
	#endif
	// else set it to 5 or something (for linux)?

	bool pausing = false;

	uint32 nHashIndex;
	for (nHashIndex = 0; nHashIndex < vHash.size(); nHashIndex++)
	{
		#ifdef _WIN32
		if (_kbhit())
		{
			int ch = _getch();
			ch = toupper(ch);
			if (ch == 'P')
			{
				pausing = true;
				printf( "\nPausing, press P again to continue... ");

				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if (_kbhit())
					{
						ch = _getch();
						ch = toupper(ch);
						if (ch == 'P')
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					Sleep(500);
				}
			}
			else
			{
				printf( "\nPress 'P' to pause...\n");
			}
		}
		#else
		int c = tty_getchar();
		if (c >= 0) {
			tty_flush();
			if (c==112) { // = p
				pausing = true;
				printf( "\nPausing, press 'p' again to continue... ");

				timeval tv;
				timeval tv2;
				timeval final;
				gettimeofday( &tv, NULL );

				while (pausing)
				{
					if ((c = tty_getchar()) >= 0)
					{
						tty_flush();
						if (c == 112)
						{
							printf( " [Continuing]\n");
							pausing = false;
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );
							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							m_fTotalCryptanalysisTime -= fTime;
						}
					}
					usleep(500*1000);
				}
			}
			else {
				printf( "\nPress 'p' to pause...\n");
			}
		}
		#endif
		unsigned char TargetHash[MAX_HASH_LEN];
		int nHashLen;
		ParseHash(vHash[nHashIndex], TargetHash, nHashLen);
		if (nHashLen != CChainWalkContext::GetHashLen())
			printf("Debug: nHashLen mismatch\n");

		// Request ChainWalk
		bool fNewlyGenerated;
		uint64* pStartPosIndexE = m_cws.RequestWalk(TargetHash,
													nHashLen,
													CChainWalkContext::GetHashRoutineName(),
													CChainWalkContext::GetPlainCharsetName(),
													CChainWalkContext::GetPlainLenMin(),
													CChainWalkContext::GetPlainLenMax(),
													CChainWalkContext::GetRainbowTableIndex(),
													nRainbowChainLen,
													fNewlyGenerated,
													debug,
													sPrecalcPathName);
//		printf("Debug: using %s walk for %s\n", fNewlyGenerated ? "newly generated" : "existing",
//					vHash[nHashIndex].c_str());

		if (fNewlyGenerated)
		{
			timeval tv;
			timeval tv2;
			timeval final;

			gettimeofday( &tv, NULL );

			printf("Pre-calculating hash %lu of %lu.%-20s\r",
				(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");
			threadPool.clear();
			pThreads.clear();

			uint32 thread_ID;
			for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
			{
				rcrackiThread* r_Thread = new rcrackiThread(TargetHash, thread_ID, nRainbowChainLen, maxThreads, pStartPosIndexE);
				if (r_Thread)
				{
					pthread_t pThread;
					int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

					if( returnValue != 0 )
					{
						printf("pThread creation failed, returnValue: %d\n", returnValue);
					}
					else
					{
						pThreads.push_back(pThread);
					}

					threadPool.push_back(r_Thread);
				}
				else
				{
					printf("r_Thread creation failed!\n");
				}
			}

			//printf("%d r_Threads created\t\t\n", threadPool.size());

			for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
			{
				pthread_t pThread = pThreads[thread_ID];
				int returnValue = pthread_join(pThread, NULL);
				if( returnValue != 0 )
				{
					printf("pThread join failed, returnValue: %d\n", returnValue);
				}

				rcrackiThread* rThread = threadPool[thread_ID];
				nChainWalkStep += rThread->GetChainWalkStep();
				delete rThread;
			}

			m_cws.StoreToFile(pStartPosIndexE, TargetHash, nHashLen);
			gettimeofday( &tv2, NULL );
			final = sub_timeofday( tv2, tv );

			float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;

			m_fTotalPrecalculationTime += fTime;
			m_fTotalCryptanalysisTime -= fTime;

			printf("%-50s\r", "");

			if ( debug )
				printf("Debug: pre-calculation time: %.2f s\n", fTime);
		}

		threadPool.clear();
		pThreads.clear();

		printf("Checking false alarms for hash %lu of %lu.%-20s\r",
			(unsigned long)nHashIndex+1, (unsigned long)vHash.size(), "");

		int i;
		for (i = 0; i < maxThreads; i++)
		{
			rcrackiThread* r_Thread = new rcrackiThread(TargetHash);
			threadPool.push_back(r_Thread);
		}

		uint32 thread_ID = 0;
		int nPos;
		for (nPos = nRainbowChainLen - 2; nPos >= 0; nPos--)
		{
			uint64 nIndexEOfCurPos = pStartPosIndexE[nPos];

			// Search matching nIndexE
			RainbowChain *pChainFound = BinarySearch(pChain, nRainbowChainCount, nIndexEOfCurPos, pIndex, nIndexSize, nChainStart);
			if (pChainFound != NULL) // For perfected indexed tables we only recieve 1 result (huge speed increase!)
			{
				rcrackiThread* rThread = threadPool[thread_ID];
				rThread->AddAlarmCheck(pChainFound, nPos);
				if (thread_ID < (unsigned long)maxThreads - 1 ) {
					thread_ID++;
				} else {
					thread_ID = 0;
				}
			}
		}

		for (thread_ID = 0; thread_ID < (unsigned long)maxThreads; thread_ID++)
		{
			rcrackiThread* r_Thread = threadPool[thread_ID];
			pthread_t pThread;

			int returnValue = pthread_create( &pThread, &attr, rcrackiThread::rcrackiThreadStaticEntryPointPthread, (void *) r_Thread);

			if( returnValue != 0 )
			{
				printf("pThread creation failed, returnValue: %d\n", returnValue);
			}
			else
			{
				pThreads.push_back(pThread);
			}
		}

		//printf("%d r_Threads created\t\t\n", threadPool.size());

		bool foundHashInThread = false;
		for (thread_ID = 0; thread_ID < threadPool.size(); thread_ID++)
		{
			rcrackiThread* rThread = threadPool[thread_ID];
			pthread_t pThread = pThreads[thread_ID];

			int returnValue = pthread_join(pThread, NULL);
			if( returnValue != 0 )
			{
				printf("pThread join failed, returnValue: %d\n", returnValue);
			}

			nChainWalkStepDueToFalseAlarm += rThread->GetChainWalkStepDueToFalseAlarm();
			nFalseAlarm += rThread->GetnFalseAlarm();

			if (rThread->FoundHash() && !foundHashInThread) {
				printf("%-50s\r", "");

				printf("plaintext of %s is %s\n", rThread->GetHash().c_str(), rThread->GetPlain().c_str());
				if (writeOutput)
				{
					if (!writeResultLineToFile(outputFile, rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary()))
						printf("Couldn't write this result to file!\n");
				}
				hs.SetPlain(rThread->GetHash(), rThread->GetPlain(), rThread->GetBinary());

				FILE* file;
				
				if ( ( file = fopen( sSessionPathName.c_str(), "a" ) ) != NULL )
				{
					std::string buffer = "sHash=" + rThread->GetHash() + ":" + rThread->GetBinary() + ":" + rThread->GetPlain() + "\n";
					fputs (buffer.c_str(), file);
					fclose (file);
				}

				m_cws.DiscardWalk(pStartPosIndexE);
				foundHashInThread = true;
			}
			//pthread
			delete rThread;
		}

		pThreads.clear();
		threadPool.clear();
	}

	printf("%-50s\r", "");
	pThreads.clear();
	threadPool.clear();
	pthread_attr_destroy(&attr);

	if ( debug )
	{
		std::cout << "Debug: chain walk step: " << nChainWalkStep << std::endl;
		std::cout << "Debug: false alarm: " << nFalseAlarm << std::endl;
		std::cout << "Debug: chain walk step due to false alarm: "
			<< nChainWalkStepDueToFalseAlarm << std::endl;
	}

	m_nTotalChainWalkStep += nChainWalkStep;
	m_nTotalFalseAlarm += nFalseAlarm;
	m_nTotalChainWalkStepDueToFalseAlarm += nChainWalkStepDueToFalseAlarm;
}

void CCrackEngine::SearchRainbowTable( std::string pathName, CHashSet& hs )
{
	// Did we already go through this file in this session?
	if (resumeSession)
	{
		std::vector<std::string> sessionFinishedPathNames;

		if (ReadLinesFromFile(sProgressPathName.c_str(), sessionFinishedPathNames))
		{
			uint32 i;
			for (i = 0; i < sessionFinishedPathNames.size(); i++)
			{
				if (sessionFinishedPathNames[i] == pathName)
				{
					printf("Skipping %s\n", pathName.c_str());
					return;
				}
			}
		}
	}

	// FileName
#ifdef _WIN32
	std::string::size_type nIndex = pathName.find_last_of('\\');
#else
	std::string::size_type nIndex = pathName.find_last_of('/');
#endif

	std::string sFileName;
	if (nIndex != std::string::npos)
		sFileName = pathName.substr(nIndex + 1);
	else
		sFileName = pathName;

	// Info
	printf("%s:\n", sFileName.c_str());

	// Setup
	int nRainbowChainLen, nRainbowChainCount;
	if (!CChainWalkContext::SetupWithPathName( pathName, nRainbowChainLen, nRainbowChainCount) )
		return;

	// Already finished?
	if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
	{
		printf("this table contains hashes with length %d only\n", CChainWalkContext::GetHashLen());
		return;
	}

	// Open
	FILE* file;

	if ( ( file = fopen( pathName.c_str(), "rb" ) ) != NULL )
	{
		// File length check
		uint32 sizeOfChain = 0;
		bool fVerified = false;
		long nFileLen = GetFileLen( pathName );

		if ( CChainWalkContext::getRTfileFormat() == getRTfileFormatId("RT") )
			sizeOfChain = 16;
		else if ( CChainWalkContext::getRTfileFormat()
			== getRTfileFormatId("RTI" ) )
		{
			sizeOfChain = 8;
		}

		if ( debug &&
				( CChainWalkContext::getRTfileFormat() != getRTfileFormatId("RTI2")
					&&
					(
						(unsigned long)nFileLen % sizeOfChain != 0
						|| nRainbowChainCount * sizeOfChain != (unsigned long)nFileLen
					)
				)
			)
		{
			printf("file length mismatch\n");
			exit( 10 );
		}

		//fseek(file, 0, SEEK_SET);
		timeval tv;
		timeval tv2;
		timeval final;

		unsigned int bytesForChainWalkSet = hs.GetStatHashTotal() * (nRainbowChainLen-1) * 8;
		if (debug) printf("Debug: Saving %u bytes of memory for chainwalkset.\n", bytesForChainWalkSet);

		uint64 nAllocatedSize;

		if ( CChainWalkContext::getRTfileFormat() != getRTfileFormatId("RTI" ))
		{
			// XXX fix this up as this verified the file exists and is readable
			// only RTI is directly reading here instead of using a reader
			fclose( file );

			BaseRTReader *reader = NULL;

			if ( CChainWalkContext::getRTfileFormat() == getRTfileFormatId("RTI2") )
			{
				reader = (BaseRTReader*)new RTI2Reader( pathName );
				sizeOfChain = reader->getChainSizeBytes();

				if ( debug )
				{
					std::cout << "Debug: This is a table in .rti2 format."
						<< std::endl;
				}
			}
			else if ( CChainWalkContext::getRTfileFormat() == getRTfileFormatId("RT") )
			{
				reader = (BaseRTReader*)new RTReader( pathName );

				if ( debug )
				{
					std::cout << "Debug: This is a table in .rt format."
						<< std::endl;
				}
			}

			static CMemoryPool mp(bytesForChainWalkSet, debug, maxMem);

			uint64 size = reader->getChainsLeft() * sizeof(RainbowChainO);
			RainbowChainO* pChain = (RainbowChainO*)mp.Allocate( size, nAllocatedSize );

			if ( debug )
			{
				std::cout << "Debug: Allocated " << nAllocatedSize
					<< " bytes, filelen " << nFileLen << std::endl;
			}

			if (pChain != NULL)
			{
				// Round to sizeOfChain boundary
				nAllocatedSize = nAllocatedSize / sizeof(RainbowChainO) * sizeof(RainbowChainO);
				// XXX safe for now...fix to use uint64 throughout
				uint32 nChains = nAllocatedSize / sizeof(RainbowChainO);

				while ( reader->getChainsLeft() > 0 )
				{
					// Load table chunk
					if ( debug )
						printf("Debug: reading...\n");

					gettimeofday( &tv, NULL );

					reader->readChains( nChains, pChain );

					gettimeofday( &tv2, NULL );
					final = sub_timeofday( tv2, tv );

					float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
					printf("%u bytes read, disk access time: %.2f s\n", nChains * sizeOfChain, fTime);
					m_fTotalDiskAccessTime += fTime;

					// Verify table chunk
					if ( debug && !fVerified )
					{
						printf("Debug: verifying the file...\n");

						// Chain length test
						int nIndexToVerify = nChains / 2;
						CChainWalkContext cwc;
						cwc.SetIndex(pChain[nIndexToVerify].nIndexS);
						int nPos;
						for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
						{
							cwc.IndexToPlain();
							cwc.PlainToHash();
							cwc.HashToIndex(nPos);
						}
						if (cwc.GetIndex() != pChain[nIndexToVerify].nIndexE)
						{
							printf("rainbow chain length verify fail\n");
							break;
						}

						// Chain sort test
						uint32 i;
						for (i = 0; i < nChains - 1; i++)
						{
							if (pChain[i].nIndexE > pChain[i + 1].nIndexE)
								break;
						}
						if (i != nChains - 1)
						{
							printf("this file is not sorted\n");
							break;
						}

						fVerified = true;
					}

					// Search table chunk
					gettimeofday( &tv, NULL );
					SearchTableChunkOld(pChain, nRainbowChainLen, nChains, hs);
					gettimeofday( &tv2, NULL );
					final = sub_timeofday( tv2, tv );
					fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
					printf("cryptanalysis time: %.2f s\n", fTime);
					m_fTotalCryptanalysisTime += fTime;

					// Already finished?
					if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
						break;
				}
			}
			else
				printf("memory allocation fail\n");

			// XXX
			//delete pChain;

			if ( reader != NULL )
				delete reader;
		}
		else
		{
			static CMemoryPool mpIndex(bytesForChainWalkSet, debug, maxMem);
			uint64 nAllocatedSizeIndex;

			std::string indexPathName = pathName + std::string(".index");

			FILE* fIndex;
			
			if( ( fIndex = fopen( indexPathName.c_str(), "rb" ) ) != NULL )
			{
				// File length check
				long nFileLenIndex = GetFileLen( indexPathName );
				//unsigned int nRows = nFileLenIndex / 11;
				//unsigned int nSize = nRows * sizeof(RTIrcrackiIndexChain);
				//printf("Debug: 8\n");
				if (nFileLenIndex % 11 != 0)
					printf("index file length mismatch (%ld bytes)\n", nFileLenIndex);
				else
				{
					//printf("index nSize: %d\n", nSize);
					//pIndex = (RTIrcrackiIndexChain*)new unsigned char[nSize];
					RTIrcrackiIndexChain *pIndex = (RTIrcrackiIndexChain*)mpIndex.Allocate(nFileLenIndex, nAllocatedSizeIndex);
					if ( debug )
					{
						std::cout << "Debug: Allocated " << nAllocatedSizeIndex
							<< " bytes for index with filelen " << nFileLenIndex
							<< std::endl;
					}

					static CMemoryPool mp(bytesForChainWalkSet + nAllocatedSizeIndex, debug, maxMem);

					if (pIndex != NULL && nAllocatedSizeIndex > 0)
					{
						nAllocatedSizeIndex = nAllocatedSizeIndex / sizeof(RTIrcrackiIndexChain) * sizeof(RTIrcrackiIndexChain);		// Round to sizeOfIndexChain boundary

						fseek(fIndex, 0, SEEK_SET);

						while ( ftell(fIndex) != nFileLenIndex )	// Index chunk read loop
						{
							// Load index chunk
							memset(pIndex, 0x00, nAllocatedSizeIndex);
							printf("reading index... ");
							gettimeofday( &tv, NULL );
							unsigned int nDataRead = fread(pIndex, 1, nAllocatedSizeIndex, fIndex);
							gettimeofday( &tv2, NULL );
							final = sub_timeofday( tv2, tv );

							float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
							printf("%u bytes read, disk access time: %.2f s\n", nDataRead, fTime);
							m_fTotalDiskAccessTime += fTime;

							//nIndexSize = nFileLenIndex / 11;
							int nIndexChainCountRead = nDataRead / sizeof(RTIrcrackiIndexChain);
							//fclose(fIndex);
							unsigned int nCoveredRainbowTableChains = 0;
							for(int i = 0; i < nIndexChainCountRead; i++)
							{
								nCoveredRainbowTableChains += pIndex[i].nChainCount;
							}

							//RainbowChain* pChain = (RainbowChain*)mp.Allocate(nFileLen, nAllocatedSize);
							RainbowChain* pChain = (RainbowChain*)mp.Allocate(nCoveredRainbowTableChains * sizeOfChain, nAllocatedSize);
							if ( debug )
							{
								std::cout << "Debug: Allocated " << nAllocatedSize
									<< " for " << nCoveredRainbowTableChains
									<< " chains, filelen " << nFileLen
									<< std::endl;
							}

							if (pChain != NULL && nAllocatedSize > 0)
							{
								nAllocatedSize = nAllocatedSize / sizeOfChain * sizeOfChain;		// Round to sizeOfChain boundary

								uint32 nProcessedChains = 0;
								while ( ftell(file) != nFileLen
									&& nProcessedChains < nCoveredRainbowTableChains )	// Chunk read loop
								{
									// Load table chunk
									memset(pChain, 0x00, nAllocatedSize);
									printf("reading table... ");
									gettimeofday( &tv, NULL );
									unsigned int nDataRead = fread(pChain, 1, nAllocatedSize, file);
									gettimeofday( &tv2, NULL );
									final = sub_timeofday( tv2, tv );

									float fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
									printf("%u bytes read, disk access time: %.2f s\n", nDataRead, fTime);
									m_fTotalDiskAccessTime += fTime;
									int nRainbowChainCountRead = nDataRead / sizeOfChain;
									// Verify table chunk (Too lazy to implement this)

									if ( debug && !fVerified )
									{
										printf("Debug: verifying the file... ");

										// Chain length test
										unsigned int nIndexToVerify = nRainbowChainCountRead / 2;
										CChainWalkContext cwc;
										uint64 nIndexS;
										nIndexS = pChain[nIndexToVerify].nIndexS & 0x0000FFFFFFFFFFFFULL; // for first 6 bytes

										//printf("nIndexS: %s\n", uint64tostr(nIndexS).c_str());
										cwc.SetIndex(nIndexS);

										int nPos;
										for (nPos = 0; nPos < nRainbowChainLen - 1; nPos++)
										{
											cwc.IndexToPlain();
											cwc.PlainToHash();
											cwc.HashToIndex(nPos);
										}

										uint64 nEndPoint = 0;

										//for(int i = 0; i < nIndexSize; i++)
										for(int i = 0; i < nIndexChainCountRead; i++)
										{
											if(nIndexToVerify >= pIndex[i].nFirstChain && nIndexToVerify < pIndex[i].nFirstChain + pIndex[i].nChainCount) // We found the matching index
											{ // Now we need to seek nIndexToVerify into the chains
												nEndPoint += (pIndex[i].nPrefix & 0x000000FFFFFFFFFFULL) << 16; // & 0x000000FFFFFFFFFFULL for first 5 bytes
												//printf("nPrefix: %s\n", uint64tostr(pIndex[i].nPrefix & 0x000000FFFFFFFFFF).c_str());
												//printf("nFirstChain: %d\n", pIndex[i].nFirstChain);
												//printf("nChainCount: %d\n", pIndex[i].nChainCount);
												nEndPoint += pChain[nIndexToVerify].nIndexE;
												break;
											}
										}

										if (cwc.GetIndex() != nEndPoint)
										{
											printf("rainbow chain length verify fail\n");
											break;
										}

										fVerified = true;
										printf("ok\n");
									}

									// Search table chunk
									gettimeofday( &tv, NULL );
									float preTime = m_fTotalCryptanalysisTime;

									SearchTableChunk(pChain, nRainbowChainLen, nRainbowChainCountRead, hs, pIndex, nIndexChainCountRead, nProcessedChains);
									float postTime = m_fTotalCryptanalysisTime;
									gettimeofday( &tv2, NULL );
									final = sub_timeofday( tv2, tv );

									fTime = 1.0f * final.tv_sec + 1.0f * final.tv_usec / 1000000;
									printf("cryptanalysis time: %.2f s\n", fTime + postTime - preTime);
									m_fTotalCryptanalysisTime += fTime;
									nProcessedChains += nRainbowChainCountRead;
									// Already finished?
									if (!hs.AnyHashLeftWithLen(CChainWalkContext::GetHashLen()))
										break;
								}
							}
							else
								printf("memory allocation failed for rainbow table\n");

							// XXX
							//delete pChain;
						}
					}
					else printf("memory allocation failed for index\n");
				}
			}
			else
			{
				printf("Can't load index\n");
				return;
			}

			if ( fIndex != NULL )
				fclose(fIndex);

			//delete pIndex;

			if ( file != NULL )
				fclose(file);
		}

		if (debug)
			printf("Debug: writing progress to %s\n", sProgressPathName.c_str());

		FILE* file;
		
		if ( ( file = fopen( sProgressPathName.c_str(), "a" ) ) != NULL )
		{
			std::string buffer = pathName + "\n";
			fputs (buffer.c_str(), file);
			fclose (file);
		}
	}
	else
		printf("can't open file\n");
}

void CCrackEngine::Run(std::vector<std::string> vPathName, CHashSet& hs, int i_maxThreads, uint64 i_maxMem, bool resume, bool bDebug)
{
#ifndef _WIN32
	tty_init();
#endif
	resumeSession = resume;
	debug = bDebug;

	maxThreads = i_maxThreads;
	maxMem = i_maxMem;
	// Reset statistics
	ResetStatistics();

	// XXX it's not like the STL has a sort method...
	// Sort vPathName (CChainWalkSet need it)
	uint32 i, j;
	for (i = 0; i < vPathName.size() - 1; i++)
		for (j = 0; j < vPathName.size() - i - 1; j++)
		{
			if (vPathName[j] > vPathName[j + 1])
			{
				std::string sTemp;
				sTemp = vPathName[j];
				vPathName[j] = vPathName[j + 1];
				vPathName[j + 1] = sTemp;
			}
		}

	// Run
	for (i = 0; i < vPathName.size() && hs.AnyhashLeft(); i++)
	{
		SearchRainbowTable(vPathName[i], hs);
		printf("\n");
	}

	// delete precalc files
	if (!keepPrecalcFiles)
		m_cws.removePrecalcFiles();

#ifndef _WIN32
	tty_done();
#endif
}

void CCrackEngine::setOutputFile(std::string pathName)
{
	writeOutput = true;
	outputFile = pathName;
}

void CCrackEngine::setSession(std::string sSession, std::string sProgress, std::string sPrecalc, bool keepPrecalc)
{
	sSessionPathName = sSession;
	sProgressPathName = sProgress;
	sPrecalcPathName = sPrecalc;
	keepPrecalcFiles = keepPrecalc;
}

float CCrackEngine::GetStatTotalDiskAccessTime()
{
	return m_fTotalDiskAccessTime;
}
/*float CCrackEngine::GetWastedTime()
{
	return m_fIndexTime;
}*/
float CCrackEngine::GetStatTotalCryptanalysisTime()
{
	return m_fTotalCryptanalysisTime;
}

float CCrackEngine::GetStatTotalPrecalculationTime()
{
	return m_fTotalPrecalculationTime;
}

uint64 CCrackEngine::GetStatTotalChainWalkStep()
{
	return m_nTotalChainWalkStep;
}

uint64 CCrackEngine::GetStatTotalFalseAlarm()
{
	return m_nTotalFalseAlarm;
}

uint64 CCrackEngine::GetStatTotalChainWalkStepDueToFalseAlarm()
{
	return m_nTotalChainWalkStepDueToFalseAlarm;
}
