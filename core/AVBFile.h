#pragma once

#include "pch.h"
#include "AVRecord.h"

extern bool isFileExist(PCSTR FileName);

class AVHashBFile 
{
public:
	std::ifstream hFile; 
	DWORD RecordCount;
	std::list<AVHashRecord> hashlist;

	AVHashBFile();

	bool open(PCSTR FileName);
	virtual void close();
	virtual bool is_open();
	virtual DWORD getRecordCount();
	AVHashRecord readNextRecord(std::string data);
};


class AVSignBFile 
{
public:
	std::ifstream hFile;
	DWORD RecordCount;
	std::list<AVSignRecord> signlist;

	AVSignBFile();

	bool open(PCSTR FileName);
	virtual void close();
	virtual bool is_open();
	virtual DWORD getRecordCount();
	AVSignRecord readNextRecord(std::string data);
};
