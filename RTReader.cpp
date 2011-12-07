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

#include "RTReader.h"

RTReader::RTReader( std::string filename )
{
	setChainSizeBytes( 16 );
	setFilename( filename );
	dataFile = fopen( filename.c_str(), "rb" );
}

uint32 RTReader::getChainsLeft()
{
	return (GetFileLen( getFilename() ) / chainSizeBytes) - chainPosition;
}

int RTReader::readChains(uint32 &numChains, RainbowChainO *pData)
{
	unsigned int numRead = fread(pData, 1, chainSizeBytes * numChains, dataFile);
	numChains = numRead / chainSizeBytes;
	chainPosition += numChains;
	return 0;
}

void RTReader::setMinimumStartPoint()
{
	uint64 tmpStartPoint;
	uint64 tmpEndPoint;
	long originalFilePos = ftell( dataFile );

	//fseek( dataFile, 0, SEEK_SET );
	rewind( dataFile );

	while ( !feof( dataFile ) )
	{
		fread( &tmpStartPoint, 8, 1, dataFile );
		fread( &tmpEndPoint, 8, 1, dataFile );

		if ( tmpStartPoint < minimumStartPoint )
			minimumStartPoint = tmpStartPoint;
	}

	fseek( dataFile, originalFilePos, SEEK_SET );
}
