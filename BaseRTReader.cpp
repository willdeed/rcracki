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

#include "BaseRTReader.h"

BaseRTReader::BaseRTReader()
{
	chainPosition = 0;

	// set it to the maximum possible value
	minimumStartPoint = (uint64)-1;
}

uint32 BaseRTReader::getChainLength()
{
	return chainLength;
}

uint32 BaseRTReader::getChainSizeBytes()
{
	return chainSizeBytes;
}

uint64 BaseRTReader::getMinimumStartPoint()
{
	return minimumStartPoint;
}

std::string BaseRTReader::getFilename()
{
	return filename;
}

std::string BaseRTReader::getSalt()
{
	return salt;
}

void BaseRTReader::setChainLength( uint32 chainLength )
{
	this->chainLength = chainLength;
}

void BaseRTReader::setChainSizeBytes( uint32 chainSizeBytes )
{
	this->chainSizeBytes = chainSizeBytes;
}

void BaseRTReader::setFilename( std::string filename )
{
	this->filename = filename;
}

void BaseRTReader::setSalt( std::string salt )
{
	this->salt = salt;
}

void BaseRTReader::Dump()
{
}
