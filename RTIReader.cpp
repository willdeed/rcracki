/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2010, 2011 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2010, 2011 James Nobis <quel@quelrod.net>
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

#include "RTIReader.h"
#include <iostream>

RTIReader::RTIReader( std::string filename )
{
	setChainSizeBytes( 8 );

	setFilename( filename );

	m_pIndex = NULL;
	dataFile = fopen( filename.c_str(), "rb" );
	if(dataFile == NULL) {
		printf("could not open file %s\n", filename.c_str());
		return;		
	}
	std::string sIndex = filename + ".index";
	FILE *pFileIndex = fopen( sIndex.c_str(), "rb" );
	if(pFileIndex == NULL) {
		printf("could not open index file %s\n", sIndex.c_str());
		return;
	}

	// Load the index file
	long nIndexFileLen = GetFileLen( sIndex );
	long nFileLen = GetFileLen( filename );

	unsigned int nTotalChainCount = nFileLen / getChainSizeBytes();

	if (nFileLen % getChainSizeBytes() != 0)
		printf("file length mismatch (%lu bytes)\n", nFileLen);
	else
	{
		// File length check
		if (nIndexFileLen % 11 != 0)
			printf("index file length mismatch (%lu bytes)\n", nIndexFileLen);
		else
		{
			if(m_pIndex != NULL) {
				delete m_pIndex;
				m_pIndex = NULL;
			}
#ifdef _MEMORYDEBUG
			printf("Allocating %u MB memory for RTIReader::m_pIndex", nIndexFileLen / 11 / (1024 * 1024));
#endif
			m_pIndex = new (std::nothrow) IndexChain[nIndexFileLen / 11];
			if(m_pIndex == NULL) {
				printf("\nFailed allocating %ld MB memory.\n", nIndexFileLen / 11 / (1024 * 1024));
				exit(-2);
			}
#ifdef _MEMORYDEBUG
			printf(" - success!\n");
#endif			
			memset(m_pIndex, 0x00, sizeof(IndexChain) * (nIndexFileLen / 11));
			fseek(pFileIndex, 0, SEEK_SET);
			//int nRead = 0;
			uint32 nRows;
			for(nRows = 0; (nRows * 11) < (uint32)nIndexFileLen; nRows++)
			{
				if(fread(&m_pIndex[nRows].nPrefix, 5, 1, pFileIndex) != 1) break;							
				if(fread(&m_pIndex[nRows].nFirstChain, 4, 1, pFileIndex) != 1) break;							
				if(fread(&m_pIndex[nRows].nChainCount, 2, 1, pFileIndex) != 1) break;							
				// Index checking part
				if(nRows != 0 && m_pIndex[nRows].nFirstChain < m_pIndex[nRows-1].nFirstChain)
				{
					printf("Corrupted index detected (FirstChain is lower than previous)\n");
					exit(-1);
				}
				else if(nRows != 0 && m_pIndex[nRows].nFirstChain != m_pIndex[nRows-1].nFirstChain + m_pIndex[nRows-1].nChainCount)
				{
					printf("Corrupted index detected (LastChain + nChainCount != FirstChain)\n");
					exit(-1);
				}
				
			}
			m_nIndexSize = nRows;
			if(m_pIndex[m_nIndexSize - 1].nFirstChain + m_pIndex[m_nIndexSize - 1].nChainCount + 1 <= nTotalChainCount) // +1 is needed.
			{
				printf("Corrupted index detected: Not covering the entire file\n");
				exit(-1);
			}
			if(m_pIndex[m_nIndexSize - 1].nFirstChain + m_pIndex[m_nIndexSize - 1].nChainCount > nTotalChainCount) // +1 is not needed here
			{
				printf("Corrupted index detected: The index is covering more than the file (%i chains of %i chains)\n", m_pIndex[m_nIndexSize - 1].nFirstChain + m_pIndex[m_nIndexSize - 1].nChainCount, nTotalChainCount);
				exit(-1);
			}

	/*					if(nIndexSize != pIndex[i].nFirstChain + pIndex[i].nChainCount)
			{
				printf("Index is not covering the entire tables\n");
			}*/
			fclose(pFileIndex);		
	//					printf("debug: Index loaded successfully (%u entries)\n", nIndexSize);
		}		
	}
}

uint32 RTIReader::getChainsLeft()
{	
	return (GetFileLen( getFilename() ) / 8) - chainPosition;
}

int RTIReader::readChains(uint32 &numChains, RainbowChainO *pData)
{	
	// We HAVE to reset the data to 0x00's or we will get in trouble
	memset(pData, 0x00, sizeof(RainbowChainO) * numChains);
	unsigned int readChains = 0;
	unsigned int chainsleft = getChainsLeft();

	for(uint32 i = 0; i < m_nIndexSize; i++)
	{
		if(chainPosition + readChains > m_pIndex[i].nFirstChain + m_pIndex[i].nChainCount) // We found the matching index
			continue;
		while(chainPosition + readChains < m_pIndex[i].nFirstChain + m_pIndex[i].nChainCount)
		{
			pData[readChains].nIndexE = m_pIndex[i].nPrefix << 16;
			int endpoint = 0; // We have to set it to 0
			// XXX start points may not exceed 6 bytes ( 2^48 )
			fread(&pData[readChains].nIndexS, 6, 1, dataFile);
			fread(&endpoint, 2, 1, dataFile);
			pData[readChains].nIndexE += endpoint;
			readChains++;
			if(readChains == numChains || readChains == chainsleft) break;
		}
		if(readChains == numChains) break;		
	}
	if(readChains != numChains) { 
		numChains = readChains; // Update how many chains we read
	}
	chainPosition += readChains;
	printf("Chain position is now %u\n", chainPosition);
	return 0;
}

void RTIReader::setMinimumStartPoint()
{
	uint64 tmpStartPoint = 0;
	uint16 tmpEndPoint;
	long originalFilePos = ftell( dataFile );

	//fseek( dataFile, 0, SEEK_SET );
	rewind( dataFile );

	while ( !feof( dataFile ) )
	{
		fread( &tmpStartPoint, 6, 1, dataFile );
		fread( &tmpEndPoint, 2, 1, dataFile );

		if ( tmpStartPoint < minimumStartPoint )
			minimumStartPoint = tmpStartPoint;
	}

	fseek( dataFile, originalFilePos, SEEK_SET );
}

RTIReader::~RTIReader()
{
	if( m_pIndex != NULL )
		delete [] m_pIndex;

	if( dataFile != NULL )
		fclose(dataFile);
}

void RTIReader::Dump()
{
	/*
	 m_nIndexSize
	 IndexChain *m_pIndex
	*/

	//std::cout << "minimumStartPoint: " << getMinimumStartPoint() << std::endl;



}
