/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2010, 2011 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
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

#ifndef _BASERTREADER_H
#define _BASERTREADER_H

#include "Public.h"
#include <string>

#if defined(_WIN32) && !defined(__GNUC__)
	#include <io.h>
#endif

class BaseRTReader
{
protected:
	FILE *dataFile;
	uint32 chainLength;
	uint32 chainPosition;
	uint32 chainSizeBytes;
	uint64 minimumStartPoint;
	std::string filename;
	std::string salt;
	
	virtual void setChainSizeBytes( uint32 chainSizeBytes );
	virtual void setChainLength( uint32 chainLength );
	virtual void setFilename( std::string filename );
	virtual void setSalt( std::string salt );

public:
	BaseRTReader();
	virtual ~BaseRTReader() { };

	virtual uint32 getChainsLeft() = 0;
	virtual int readChains(uint32 &numChains, RainbowChainO *pData) = 0;
	virtual void setMinimumStartPoint() = 0;

	virtual uint32 getChainLength();
	virtual uint32 getChainSizeBytes();
	virtual std::string getFilename();
	virtual std::string getSalt();
	virtual uint64 getMinimumStartPoint();

	virtual void Dump();
};

#endif
