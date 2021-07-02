#include "pch.h"
#include "scanner.h"
#include "AVBFile.h"

void checkHashFile(PCSTR FileName, PCSTR DBPath, std::list<std::string>& detected)
{
	AVHashBFile HashBase;
	HashBase.open(DBPath);
	
	DWORD RecordCount = HashBase.getRecordCount();
	std::ifstream file(FileName, std::ios::in | std::ios::binary | std::ios::ate);
	if (file)
	{
		std::ifstream::pos_type size = file.tellg();
		file.seekg(0, std::ios::beg);
		char* filedata = new char[size];
		file.read(filedata, size);
		for (DWORD RecID = 0; RecID < RecordCount; RecID++)
		{
			std::string hashdata;
			getline(HashBase.hFile, hashdata);
			AVHashRecord Record = HashBase.readNextRecord(hashdata);

			if (size != Record.FileLen) continue;

			std::string Hash;

			Hash = reinterpret_cast<char*>(MD5(NULL));

			std::vector<unsigned char> buffer({});

			if (!(Hash == Record.Hash)) {
				detected.push_back(Record.Name);
				std::string filename = FileName;
				detected.push_back(filename);
			}
		}
	}	
	HashBase.close();
}

void processPath(PCSTR Path, PCSTR DBPath, std::list<std::string>& detected)
{
	std::string filepath;
	filepath = Path;
	filepath += "\\";

	std::string target;
	target = filepath + "*";
	WIN32_FIND_DATAA FindData;
	HANDLE h = FindFirstFileA(target.c_str(), &FindData);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!strcmp(FindData.cFileName, ".") || !strcmp(FindData.cFileName, "..")) continue;

			if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string folder = filepath + FindData.cFileName;
				processPath(folder.c_str(), DBPath, detected);
			}
			else
			{
				std::string file = filepath + FindData.cFileName;
				checkFile(file.c_str(), DBPath, detected);
			}
		} while (FindNextFileA(h, &FindData));
	}
}


void Scanner::AVstart(PCSTR DBPath, PCSTR Path)
{
	AVSignBFile SignBase;
	if (!SignBase.open(DBPath))
	{
		detected.push_back((char*)"error");
		return;
	}
	SignBase.close();
	processPath(Path, DBPath, detected);
}

void Scanner::GetNextString(BYTE* byteString)
{
	if (detected.empty()) return;

	std::string String = detected.front();
	detected.pop_front();
	std::memcpy(byteString, (BYTE*)String.c_str(), sizeof(String));
}

void Scanner::Delete()
{
	if (detected.empty()) return;

	std::list<std::string>::iterator detected_front = detected.begin();
	++detected_front;
	for (int i = 0; i < detected.size() / 2; i++)
	{
		std::string filename = *detected_front;
		remove(filename.c_str());
		std::advance(detected_front, 2);
	}
}
