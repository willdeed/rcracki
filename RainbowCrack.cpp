/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010  Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009 James Dickson
 * Copyright 2009, 2010, 2011 James Nobis <quel@quelrod.net>
 * Copyright 2010 uroskn
 *
 * Modified by Martin Westergaard Jørgensen <martinwj2005@gmail.com> to support  * indexed and hybrid tables
 *
 * Modified by neinbrucke to support multi threading and a bunch of other stuff :)
 *
 * 2009-01-04 - <james.dickson@comhem.se> - Slightly modified (or "fulhack" as 
 * we say in sweden)  to support cain .lst files.
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
#include "lm2ntlm.h"
#include <algorithm>

#ifdef _WIN32
	#include <io.h>
#else
	#include <unistd.h>
	#include <dirent.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32) && !defined(__GNUC__)
	#pragma comment(lib, "libeay32.lib")
#endif

//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void GetTableList( std::string sWildCharPathName, std::vector<std::string>& vPathName )
{
	std::string sPath;
	std::string::size_type n = sWildCharPathName.find_last_of('\\');

	if ( n == (sWildCharPathName.size() - 1) )
	{
		sWildCharPathName = sWildCharPathName.substr(0, n);
		n = sWildCharPathName.find_last_of('\\');
	}

	if (n != std::string::npos)
		sPath = sWildCharPathName.substr(0, n + 1);

	_finddata_t fd;

	long handle = _findfirst(sWildCharPathName.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			std::string sName = fd.name;
			if (sName.size()>3) {
				if (sName.substr(sName.size()-3, 3) == ".rt" && !(fd.attrib & _A_SUBDIR))
				{
					std::string sPathName = sPath + sName;
					vPathName.push_back(sPathName);
				}
			}
			if (sName.size()>4) {
				if (sName.substr(sName.size()-4, 4) == ".rti" && !(fd.attrib & _A_SUBDIR))
				{
					std::string sPathName = sPath + sName;
					vPathName.push_back(sPathName);
				}
			}
			if (sName.size()>5) {
				if (sName.substr(sName.size()-5, 5) == ".rti2" && !(fd.attrib & _A_SUBDIR))
				{
					std::string sPathName = sPath + sName;
					vPathName.push_back(sPathName);
				}
			}

			if (sName != "." && sName != ".." && (fd.attrib & _A_SUBDIR))
			{
				std::string sPath_sub = sPath + sName + '\\';
				std::string sWildCharPathName_sub = sPath_sub + '*';
				GetTableList(sWildCharPathName_sub, vPathName);
			}

		} while (_findnext(handle, &fd) == 0);

		_findclose(handle);
	}
	//printf("Found %d rainbowtables (files) in %d sub directories...\n", vPathName.size(), subDir_count);
}
#else
//void GetTableList(int argc, char* argv[], vector<string>& vPathName)
void GetTableList( std::string sWildCharPathName, std::vector<std::string>& vPathName )
{
	struct stat buf;
	if (lstat(sWildCharPathName.c_str(), &buf) == 0)
	{
		if (S_ISDIR(buf.st_mode))
		{
			DIR *dir = opendir(sWildCharPathName.c_str());
			if(dir)
			{
				struct dirent *pDir=NULL;
				while((pDir = readdir(dir)) != NULL)
				{
					std::string filename = "";
					filename += (*pDir).d_name;
					if (filename != "." && filename != "..")
					{
						std::string new_filename = sWildCharPathName + '/' + filename;
						GetTableList(new_filename, vPathName);
					}
				}
				closedir(dir);
			}
		}
		else if (S_ISREG(buf.st_mode))
		{
			if (sWildCharPathName.size()>3)
			{
				if (sWildCharPathName.substr(sWildCharPathName.size()-3, 3) == ".rt")
				{
					vPathName.push_back(sWildCharPathName);
				}
			}
			if (sWildCharPathName.size()>4)
			{
				if (sWildCharPathName.substr(sWildCharPathName.size()-4, 4) == ".rti")
				{
					//string sPathName_sub = sPath_sub + sName_sub;
					vPathName.push_back(sWildCharPathName);
					//printf("sPathName_sub: %s\n", sPathName_sub.c_str());
				}
			}
			if ( sWildCharPathName.size() > 5 )
			{
				if ( sWildCharPathName.substr( sWildCharPathName.size() - 5, 5 ) == ".rti2" )
				{
					vPathName.push_back( sWildCharPathName );
				}
			}
		}
	}
}
#endif

bool NormalizeHash(std::string& sHash)
{
	std::string sNormalizedHash = sHash;

	if (   sNormalizedHash.size() % 2 != 0
		|| sNormalizedHash.size() < MIN_HASH_LEN * 2
		|| sNormalizedHash.size() > MAX_HASH_LEN * 2)
		return false;

	// Make lower
	uint32 i;
	for (i = 0; i < sNormalizedHash.size(); i++)
	{
		if (sNormalizedHash[i] >= 'A' && sNormalizedHash[i] <= 'F')
			sNormalizedHash[i] = (char) sNormalizedHash[i] - 'A' + 'a';
	}

	// Character check
	for (i = 0; i < sNormalizedHash.size(); i++)
	{
		if (   (sNormalizedHash[i] < 'a' || sNormalizedHash[i] > 'f')
			&& (sNormalizedHash[i] < '0' || sNormalizedHash[i] > '9'))
			return false;
	}

	sHash = sNormalizedHash;
	return true;
}

void LoadLMHashFromPwdumpFile( std::string sPathName, std::vector<std::string>& vUserName, std::vector<std::string>& vLMHash, std::vector<std::string>& vNTLMHash )
{
	std::vector<std::string> vLine;
	if (ReadLinesFromFile(sPathName, vLine))
	{
		uint32 i;
		for (i = 0; i < vLine.size(); i++)
		{
			std::vector<std::string> vPart;
			if (SeperateString(vLine[i], "::::", vPart))
			{
				std::string sUserName = vPart[0];
				std::string sLMHash   = vPart[2];
				std::string sNTLMHash = vPart[3];

				if (sLMHash.size() == 32 && sNTLMHash.size() == 32)
				{
					if (NormalizeHash(sLMHash) && NormalizeHash(sNTLMHash))
					{
						vUserName.push_back(sUserName);
						vLMHash.push_back(sLMHash);
						vNTLMHash.push_back(sNTLMHash);
					}
					else
						printf("invalid lm/ntlm hash %s:%s\n", sLMHash.c_str(), sNTLMHash.c_str());
				}
			}
		}
	}
	else
		printf("can't open %s\n", sPathName.c_str());
}

// 2009-01-04 - james.dickson - Added this so we can load hashes from cain .LST files.
void LoadLMHashFromCainLSTFile( std::string sPathName, std::vector<std::string>& vUserName, std::vector<std::string>& vLMHash, std::vector<std::string>& vNTLMHash )
{
	std::vector<std::string> vLine;
	if (ReadLinesFromFile(sPathName, vLine))
	{
		uint32 i;
		for (i = 0; i < vLine.size(); i++)
		{
			std::vector<std::string> vPart;
			if (SeperateString(vLine[i], "\t\t\t\t\t\t", vPart))
			{
				std::string sUserName = vPart[0];
				std::string sLMHash   = vPart[4];
				std::string sNTLMHash = vPart[5];

				if (sLMHash.size() == 32 && sNTLMHash.size() == 32)
				{
					if (NormalizeHash(sLMHash) && NormalizeHash(sNTLMHash))
					{
						vUserName.push_back(sUserName);
						vLMHash.push_back(sLMHash);
						vNTLMHash.push_back(sNTLMHash);
					}
					else
						printf("invalid lm/ntlm hash %s:%s\n", sLMHash.c_str(), sNTLMHash.c_str());
				}
			}
		}
	}
	else
		printf("can't open %s\n", sPathName.c_str());
}

bool NTLMPasswordSeek(unsigned char* pLMPassword, int nLMPasswordLen, int nLMPasswordNext,
					  unsigned char* pNTLMHash, std::string& sNTLMPassword)
{
	if (nLMPasswordNext == nLMPasswordLen)
	{
		unsigned char md[MD4_DIGEST_LENGTH];
		MD4_NEW(pLMPassword, nLMPasswordLen * 2, md);

		if (memcmp(md, pNTLMHash, MD4_DIGEST_LENGTH) == 0)
		{
			sNTLMPassword = "";
			int i;
			for (i = 0; i < nLMPasswordLen; i++)
				sNTLMPassword += char(pLMPassword[i * 2]);
			return true;
		}
		else
			return false;
	}

	if (NTLMPasswordSeek(pLMPassword, nLMPasswordLen, nLMPasswordNext + 1, pNTLMHash, sNTLMPassword))
		return true;

	if (   pLMPassword[nLMPasswordNext * 2] >= 'A'
		&& pLMPassword[nLMPasswordNext * 2] <= 'Z')
	{
		pLMPassword[nLMPasswordNext * 2] = (unsigned char) pLMPassword[nLMPasswordNext * 2] - 'A' + 'a';
		if (NTLMPasswordSeek(pLMPassword, nLMPasswordLen, nLMPasswordNext + 1, pNTLMHash, sNTLMPassword))
			return true;
		pLMPassword[nLMPasswordNext * 2] = (unsigned char) pLMPassword[nLMPasswordNext * 2] - 'a' + 'A';
	}

	return false;
}

bool LMPasswordCorrectCase( std::string sLMPassword, unsigned char* pNTLMHash, std::string& sNTLMPassword )
{
	if (sLMPassword.size() == 0)
	{
		sNTLMPassword = "";
		return true;
	}

	unsigned char* pLMPassword = new unsigned char[sLMPassword.size() * 2];
	uint32 i;
	for (i = 0; i < sLMPassword.size(); i++)
	{
		pLMPassword[i * 2    ] = sLMPassword[i];
		pLMPassword[i * 2 + 1] = 0x00;
	}
	bool fRet = NTLMPasswordSeek(pLMPassword, sLMPassword.size(), 0, pNTLMHash, sNTLMPassword);

	delete pLMPassword;

	return fRet;
}

void Usage()
{
	Logo();

	printf("usage: rcracki_mt -h hash rainbow_table_pathname\n");
	printf("       rcracki_mt -l hash_list_file rainbow_table_pathname\n");
	printf("       rcracki_mt -f pwdump_file rainbow_table_pathname\n");
	printf("       rcracki_mt -c lst_file rainbow_table_pathname\n");
	printf("\n");
	printf("-h hash:                use raw hash as input\n");
	printf("-l hash_list_file:      use hash list file as input, each hash in a line\n");
	printf("-f pwdump_file:         use pwdump file as input, handles lanmanager hash only\n");
	printf("-c lst_file:            use .lst (cain format) file as input\n");
	printf("-r [-s session_name]:   resume from previous session, optional session name\n");
	printf("rainbow_table_pathname: pathname(s) of the rainbow table(s)\n");
	printf("\n");
	printf("Extra options:    -t [nr] use this amount of threads/cores, default is 1\n");
	printf("                  -o [output_file] write (temporary) results to this file\n");
	printf("                  -s [session_name] write session data with this name\n");
	printf("                  -k keep precalculation on disk\n");
	printf("                  -m [megabytes] limit memory usage\n");
	printf("                  -v show debug information\n");
	printf("\n");
#ifdef _WIN32
	printf("example: rcracki_mt -h 5d41402abc4b2a76b9719d911017c592 -t 2 [path]\\MD5\n");
	printf("         rcracki_mt -l hash.txt [path_to_specific_table]\\*\n");
#else
	printf("example: rcracki_mt -h 5d41402abc4b2a76b9719d911017c592 -t 2 [path]/MD5\n");
	printf("         rcracki_mt -l hash.txt [path_to_specific_table]/*\n");
#endif
	printf("         rcracki_mt -f hash.txt -t 4 -o results.txt *.rti\n");
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		Usage();
		return 0;
	}

	std::vector<std::string> vPathName;
	std::vector<std::string> vDefaultRainbowTablePath;
	std::string sWildCharPathName			= "";
	std::string sInputType						= "";
	std::string sInput							= "";
	std::string outputFile						= "";
	std::string sApplicationPath				= "";
	std::string sIniPathName					= "rcracki_mt.ini";
	bool writeOutput						= false;
	std::string sSessionPathName				= "rcracki.session";
	std::string sProgressPathName			= "rcracki.progress";
	std::string sPrecalcPathName				= "rcracki.precalc";
	bool resumeSession					= false;
	bool useDefaultRainbowTablePath	= false;
	bool debug								= false;
	bool keepPrecalcFiles				= false;
	std::string sAlgorithm						= "";
	int maxThreads							= 1;
	uint64 maxMem							= 0;
	CHashSet hs;

	// Read defaults from ini file;
	bool readFromIni = false;
	std::vector<std::string> vLine;
	if (ReadLinesFromFile(sIniPathName, vLine)) {
		readFromIni = true;
	}
	else if (ReadLinesFromFile(GetApplicationPath() + sIniPathName, vLine)) {
		readFromIni = true;
	}
	if (readFromIni)
	{
		uint32 i;
		for (i = 0; i < vLine.size(); i++)
		{
			if (vLine[i].substr(0,1) != "#")
			{
				std::vector<std::string> vPart;
				if (SeperateString(vLine[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "Threads") {
						maxThreads = atoi(sValue.c_str());
					}
					else if (sOption == "MaxMemoryUsage" ) {
						maxMem = atoi(sValue.c_str()) * 1024 *1024;
					}
					else if (sOption == "DefaultResultsFile") {
						outputFile = sValue;
					}
					else if (sOption == "AlwaysStoreResultsToFile") {
						if (sValue == "1")
							writeOutput = true;
					}
					else if (sOption.substr(0,24) == "DefaultRainbowTablePath.") {
						//printf("Default RT path: %s\n", sValue.c_str());
						vDefaultRainbowTablePath.push_back(vLine[i]);
					}
					else if (sOption == "DefaultAlgorithm") {
						useDefaultRainbowTablePath = true;
						sAlgorithm = sValue;
					}
					else if (sOption == "AlwaysDebug") {
						if (sValue == "1")
							debug = true;
					}
					else if (sOption == "AlwaysKeepPrecalcFiles") {
						if (sValue == "1")
							keepPrecalcFiles = true;
					}
					else {
						printf("illegal option %s in ini file %s\n", sOption.c_str(), sIniPathName.c_str());
						return 0;
					}
				}
			}
		}
		if (writeOutput && outputFile == "")
		{
			printf("You need to specify a 'DefaultResultsFile' with 'AlwaysStoreResultsToFile=1'\n");
			writeOutput = false;
		}
	}

	// Parse command line arguments
	int i;
	for (i = 1; i < argc; i++)
	{
		std::string cla = argv[i];
		if (cla == "-h") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-l") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-f") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-c") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-t") {
			i++;
			if (i < argc)
				maxThreads = atoi(argv[i]);
		}
		else if ( cla == "-m" ) {
			i++;
			if ( i < argc )
				maxMem = atoi(argv[i]) * 1024 * 1024;
		}
		else if (cla == "-o") {
			writeOutput = true;
			i++;
			if (i < argc)
				outputFile = argv[i];
		}
		else if (cla == "-r") {
			resumeSession = true;
		}
		else if (cla == "-s") {
			i++;
			if (i < argc)
			{
				sSessionPathName		=  argv[i];
				sSessionPathName		+= ".session";
				sProgressPathName		=  argv[i];
				sProgressPathName		+= ".progress";
				sPrecalcPathName		=  argv[i];
				sPrecalcPathName		+= ".precalc";
			}
		}
		else if (cla == "-v") {
			debug = true;
		}
		else if (cla == "-k") {
			keepPrecalcFiles = true;
		}
		else if (cla == "-a") {
			useDefaultRainbowTablePath = true;
			i++;
			if (i < argc)
				sAlgorithm = argv[i];
		}
		else {
			GetTableList(cla, vPathName);
		}
	}

	if (debug && !readFromIni)
		printf("Debug: Couldn't read rcracki_mt.ini, continuing anyway.\n");

	// Load session data if we are resuming
	if (resumeSession)
	{
		vPathName.clear();
		std::vector<std::string> sSessionData;
		if (ReadLinesFromFile(sSessionPathName.c_str(), sSessionData))
		{
			uint32 i;
			for (i = 0; i < sSessionData.size(); i++)
			{
				std::vector<std::string> vPart;
				if (SeperateString(sSessionData[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "sPathName") {
						vPathName.push_back(sValue);
					}
					else if (sOption == "sInputType") {
						sInputType = sValue;
					}
					else if (sOption == "sInput") {
						sInput = sValue;
					}
					else if (sOption == "outputFile") {
						writeOutput = true;
						outputFile = sValue;
					}
					else if (sOption == "keepPrecalcFiles") {
						if (sValue == "1")
							keepPrecalcFiles = true;
					}
				}
			}
		}
		else {
			printf("Couldn't open session file %s\n", sSessionPathName.c_str());
			return 0;
		}
	}

	if (maxThreads<1)
		maxThreads = 1;

	// don't load these if we are resuming a session that already has a list of tables
	if (useDefaultRainbowTablePath && !resumeSession)
	{
		uint32 i;
		for (i = 0; i < vDefaultRainbowTablePath.size(); i++)
		{
			std::vector<std::string> vPart;
			if (SeperateString(vDefaultRainbowTablePath[i], ".=", vPart))
			{
				std::string lineAlgorithm = vPart[1];
				std::string linePath = vPart[2];

				if (lineAlgorithm == sAlgorithm)
					GetTableList(linePath, vPathName);
			}
		}
	}

	printf("Using %d threads for pre-calculation and false alarm checking...\n", maxThreads);

	setvbuf(stdout, NULL, _IONBF,0);
	if (vPathName.size() == 0)
	{
		printf("no rainbow table found\n");
		return 0;
	}
	printf("Found %lu rainbowtable files...\n\n",
		(unsigned long)vPathName.size());

	bool fCrackerType;			// true: hash cracker, false: lm cracker
	std::vector<std::string> vHash;		// hash cracker
	std::vector<std::string> vUserName;	// lm cracker
	std::vector<std::string> vLMHash;		// lm cracker
	std::vector<std::string> vNTLMHash;	// lm cracker
	if (sInputType == "-h")
	{
		fCrackerType = true;

		std::string sHash = sInput;
		if (NormalizeHash(sHash))
			vHash.push_back(sHash);
		else
			printf("invalid hash: %s\n", sHash.c_str());
	}
	else if (sInputType == "-l")
	{
		fCrackerType = true;

		std::string sPathName = sInput;
		std::vector<std::string> vLine;
		if (ReadLinesFromFile(sPathName, vLine))
		{
			uint32 i;
			for (i = 0; i < vLine.size(); i++)
			{
				std::string sHash = vLine[i];
				if (NormalizeHash(sHash))
					vHash.push_back(sHash);
				else
					printf("invalid hash: %s\n", sHash.c_str());
			}
		}
		else
			printf("can't open %s\n", sPathName.c_str());
	}
	else if (sInputType == "-f")
	{
		fCrackerType = false;

		std::string sPathName = sInput;
		LoadLMHashFromPwdumpFile(sPathName, vUserName, vLMHash, vNTLMHash);
	}
	else if (sInputType == "-c")
	{
		// 2009-01-04 - james.dickson - Added this for cain-files.
		fCrackerType = false;
		std::string sPathName = sInput;
		LoadLMHashFromCainLSTFile(sPathName, vUserName, vLMHash, vNTLMHash);
	}
	else
	{
		Usage();
		return 0;
	}

	if (fCrackerType && vHash.size() == 0)
	{
		printf("no hashes found");
		return 0;
	}
	if (!fCrackerType && vLMHash.size() == 0)
	{
		return 0;
		printf("no hashes found");
	}

	if (fCrackerType)
	{
		uint32 i;
		for (i = 0; i < vHash.size(); i++)
			hs.AddHash(vHash[i]);
	}
	else
	{
		uint32 i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			hs.AddHash(vLMHash[i].substr(0, 16));
			hs.AddHash(vLMHash[i].substr(16, 16));
		}
	}

	// Load found hashes from session file
	if (resumeSession)
	{
		std::vector<std::string> sSessionData;
		if (ReadLinesFromFile(sSessionPathName.c_str(), sSessionData))
		{
			uint32 i;
			for (i = 0; i < sSessionData.size(); i++)
			{
				std::vector<std::string> vPart;
				if (SeperateString(sSessionData[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "sHash") {
						std::vector<std::string> vPartHash;
						if (SeperateString(sValue, "::", vPartHash))
						{
							std::string sHash = vPartHash[0];
							std::string sBinary = vPartHash[1];
							std::string sPlain = vPartHash[2];
							
							hs.SetPlain(sHash, sPlain, sBinary);
						}
					}
				}
			}
		}
	}
	else
	{
		FILE* file;
		
		std::string buffer = "";

		// (Over)write session data if we are not resuming
		if ( ( file = fopen( sSessionPathName.c_str(), "w" ) ) != NULL )
		{
			buffer += "sInputType=" + sInputType + "\n";
			buffer += "sInput=" + sInput + "\n";

			uint32 i;
			for (i = 0; i < vPathName.size(); i++)
			{
				buffer += "sPathName=" + vPathName[i] + "\n";
			}

			if (writeOutput)
				buffer += "outputFile=" + outputFile + "\n";

			if (keepPrecalcFiles)
				buffer += "keepPrecalcFiles=1\n";

			fputs (buffer.c_str(), file);
			fclose (file);
		}

		file = fopen( sProgressPathName.c_str(), "w" );
		fclose( file );
	}

	// Run
	CCrackEngine ce;
	if (writeOutput)
		ce.setOutputFile(outputFile);
	ce.setSession(sSessionPathName, sProgressPathName, sPrecalcPathName, keepPrecalcFiles);
	ce.Run(vPathName, hs, maxThreads, maxMem, resumeSession, debug);

	// Remove session files
	if (debug) printf("Debug: Removing session files.\n");

	if (remove(sSessionPathName.c_str()) == 0)
		remove(sProgressPathName.c_str());
	else
		if (debug) printf("Debug: Failed removing session files.\n");

	// Statistics
	printf("statistics\n");
	printf("-------------------------------------------------------\n");
	printf("plaintext found:            %d of %d (%.2f%%)\n", hs.GetStatHashFound(),
															hs.GetStatHashTotal(),
															100.0f * hs.GetStatHashFound() / hs.GetStatHashTotal());
	printf("total disk access time:     %.2f s\n", ce.GetStatTotalDiskAccessTime());
	printf("total cryptanalysis time:   %.2f s\n", ce.GetStatTotalCryptanalysisTime());
	printf("total pre-calculation time: %.2f s\n", ce.GetStatTotalPrecalculationTime());
	printf("total chain walk step:      %llu\n",     ce.GetStatTotalChainWalkStep());
	printf("total false alarm:          %llu\n",     ce.GetStatTotalFalseAlarm());
	printf("total chain walk step due to false alarm: %llu\n", ce.GetStatTotalChainWalkStepDueToFalseAlarm());
//	printf("total chain walk step skipped due to checkpoints: %d\n", ce.GetStatTotalFalseAlarmSkipped()); // Checkpoints not used - yet
	printf("\n");

	// Result
	printf("result\n");
	printf("-------------------------------------------------------\n");
	if (fCrackerType)
	{
		uint32 i;
		for (i = 0; i < vHash.size(); i++)
		{
			std::string sPlain, sBinary;
			if (!hs.GetPlain(vHash[i], sPlain, sBinary))
			{
				sPlain  = "<notfound>";
				sBinary = "<notfound>";
			}

			printf("%s\t%s\thex:%s\n", vHash[i].c_str(), sPlain.c_str(), sBinary.c_str());
		}
	}
	else
	{
		uint32 i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			std::string sPlain1, sBinary1;
			bool fPart1Found = hs.GetPlain(vLMHash[i].substr(0, 16), sPlain1, sBinary1);
			if (!fPart1Found)
			{
				sPlain1  = "<notfound>";
				sBinary1 = "<notfound>";
			}

			std::string sPlain2, sBinary2;
			bool fPart2Found = hs.GetPlain(vLMHash[i].substr(16, 16), sPlain2, sBinary2);
			if (!fPart2Found)
			{
				sPlain2  = "<notfound>";
				sBinary2 = "<notfound>";
			}

			std::string sPlain = sPlain1 + sPlain2;
			std::string sBinary = sBinary1 + sBinary2;

			// Correct case
			if (fPart1Found && fPart2Found)
			{
				unsigned char NTLMHash[16];
				int nHashLen;
				ParseHash(vNTLMHash[i], NTLMHash, nHashLen);
				if (nHashLen != 16)
					printf("debug: nHashLen mismatch\n");
				std::string sNTLMPassword;
				if (LMPasswordCorrectCase(sPlain, NTLMHash, sNTLMPassword))
				{
					sPlain = sNTLMPassword;
					sBinary = HexToStr((const unsigned char*)sNTLMPassword.c_str(), sNTLMPassword.size());
					if (writeOutput)
					{
						if (!writeResultLineToFile(outputFile, vNTLMHash[i].c_str(), sPlain.c_str(), sBinary.c_str()))
							printf("Couldn't write final result to file!\n");
					}
				}
				else
				{
					printf("%-14s\t%s\thex:%s\n", vUserName[i].c_str(), sPlain.c_str(), sBinary.c_str());
					printf("Failed case correction, trying unicode correction for: %s\n", sPlain.c_str());
					LM2NTLMcorrector corrector;
					if (corrector.LMPasswordCorrectUnicode(sBinary, NTLMHash, sNTLMPassword))
					{
						sPlain = sNTLMPassword;
						sBinary = corrector.getBinary();
						if (writeOutput)
						{
							if (!writeResultLineToFile(outputFile, vNTLMHash[i].c_str(), sPlain.c_str(), sBinary.c_str()))
								printf("Couldn't write final result to file!\n");
						}
					}
					else {
						printf("unicode correction for password %s failed!\n", sPlain.c_str());
					}
				}
			}

			// Display
			printf("%-14s\t%s\thex:%s\n", vUserName[i].c_str(),
										  sPlain.c_str(),
										  sBinary.c_str());
			
		}
	}

	return 0;
}
