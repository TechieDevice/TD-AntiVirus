#include "pch.h"
#include "AVBFile.h"


bool isFileExist(PCSTR FileName)
{
	return GetFileAttributesA(FileName) != DWORD(-1);
}


AVHashBFile::AVHashBFile() {
	this->RecordCount = 0;
}

void AVHashBFile::close() {
	if (hFile.is_open()) hFile.close();
}

bool AVHashBFile::is_open() 
{
	return hFile.is_open();
}

DWORD AVHashBFile::getRecordCount() 
{
	return this->RecordCount;
}

bool AVHashBFile::open(PCSTR FileName) 
{
	if (FileName == NULL) return false;
	if (isFileExist(FileName)) {
		hFile.open(FileName, std::ios::in | std::ios::binary | std::ios::ate);
		if (!hFile.is_open()) return false;

		std::ifstream::pos_type size = hFile.tellg();
		hFile.seekg(0, hFile.beg);

		CHAR Sign[4];
		hFile.read((PSTR)Sign, 4);
		if (memcmp(Sign, "AVHB", 4)) {
			hFile.close(); 
			return false;
		}
		std::string count;
		getline(hFile, count);
		this->RecordCount = std::stoi(count);

		size = (size - hFile.tellg());
		char* tmp = new char[size];
		hFile.read(tmp, size);
		std::string str(tmp);

		std::string delimiter = "\n";
		size_t pos = 0;
		std::string token;

		std::stringstream x;
		x << tmp;
		while (x >> token)
		{
			AVHashRecord rec = readNextRecord(token);
			hashlist.push_back(rec);
		}
	}
	else { return false; }
	return true;
}

AVHashRecord AVHashBFile::readNextRecord(std::string data) 
{
	if (!hFile.is_open()) return NULL;

	std::string delimiter = ":";
	std::string token[4];

	char* ptr = 0;
	ptr = strtok((char*)data.c_str(), delimiter.c_str());

	for (int i = 0; i < 3; i++)
	{
		std::string str(ptr);
		token[i] = str;
		ptr = strtok(0, delimiter.c_str());
	}
	token[3] = std::to_string(token[2].length());
	
	return AVHashRecord(token);
}



AVSignBFile::AVSignBFile() 
{
	this->RecordCount = 0;
}

void AVSignBFile::close() 
{
	if (hFile.is_open()) hFile.close();
}

bool AVSignBFile::is_open() {
	return hFile.is_open();
}

DWORD AVSignBFile::getRecordCount() 
{
	return this->RecordCount;
}

bool AVSignBFile::open(PCSTR FileName) 
{
	if (FileName == NULL) return false;
	if (isFileExist(FileName)) {
		hFile.open(FileName, std::ios::in | std::ios::binary | std::ios::ate);
		if (!hFile.is_open()) return false;

		hFile.seekg(0, hFile.end);
		std::ifstream::pos_type size = hFile.tellg();
		hFile.seekg(0, hFile.beg);

		CHAR Sign[4];
		hFile.read((PSTR)Sign, 4);
		if (memcmp(Sign, "AVSB", 4)) {
			hFile.close();
			return false;
		}
		std::string count;
		getline(hFile, count);
		this->RecordCount = std::stoi(count);

		size = (size - hFile.tellg());
		char* tmp = new char[size];
		hFile.read(tmp, size);
		std::string str(tmp);

		std::string delimiter = "\n";
		size_t pos = 0;
		std::string token;

		std::stringstream x;       
		x << tmp;                
		while (x >> token)
		{
			AVSignRecord rec = readNextRecord(token);
			signlist.push_back(rec);
		}
	}
	else { return false; }
	return true;
}

AVSignRecord AVSignBFile::readNextRecord(std::string data) 
{
	if (!hFile.is_open()) return NULL;

	std::string delimiter = ":";
	std::string token[5];

	char* ptr = 0;     
	ptr = strtok((char*)data.c_str(), delimiter.c_str());
	
	for (int i = 0; i < 4; i++) 
	{
		std::string str(ptr);
		token[i] = str;
		ptr = strtok(0, delimiter.c_str()); 
	}
	if (token[2] == "*") token[2] = "0";
	token[4] = std::to_string(token[0].length());

	return AVSignRecord(token);
}
