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

#ifndef _HASHROUTINE_H
#define _HASHROUTINE_H

#include <string>
#include <vector>

#include "global.h"

typedef void (*HASHROUTINE)(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);

class CHashRoutine  
{
public:
	CHashRoutine();
	virtual ~CHashRoutine();

private:
	std::vector<std::string> vHashRoutineName;
	std::vector<HASHROUTINE> vHashRoutine;
	std::vector<int> vHashLen;

	void AddHashRoutine( std::string sHashRoutineName, HASHROUTINE pHashRoutine, int nHashLen);

public:
	std::string GetAllHashRoutineName();
	void GetHashRoutine( std::string sHashRoutineName, HASHROUTINE& pHashRoutine, int& nHashLen );
};

#endif
