#pragma once

#include "pch.h"


struct AVHashRecord 
{
	AVHashRecord(std::string token[4]) {
		this->Name = token[2];
		this->NameLen = stoi(token[3]);
		this->FileLen = stoi(token[1]);
		this->Hash = token[0];
	}
	~AVHashRecord() {}

	std::string Name;
	int NameLen;
	int FileLen;
	std::string Hash;
};


struct AVSignRecord 
{
	AVSignRecord(std::string token[5]) {
		this->Name = token[0];
		this->NameLen = stoi(token[4]);
		this->Type = token[1];
		try
		{
			this->Offset = stoi(token[2]);
		}
		catch (std::invalid_argument)
		{
			this->Offset = 0;
		}

		std::string delimiterBeg = "{";
		std::string delimiterEnd = "}";
		std::string delimiter = "??";
		std::list<std::string> newToken3;

		size_t f = token[3].find(delimiter);

		char* ptr = 0;
		ptr = strtok((char*)token[3].c_str(), delimiter.c_str());
		while (ptr)
		{
			std::string str(ptr);
			if (str != "") newToken3.push_back(str);
			ptr = strtok(0, delimiter.c_str());
		}

		for (int i = 0; i < newToken3.size(); )
		{
			ptr = strtok((char*)newToken3.front().c_str(), delimiterBeg.c_str());
			while (ptr)
			{
				std::string str(ptr);
				this->Sign.push_back(str);
				ptr = strtok(0, delimiterEnd.c_str());
				ptr = strtok(0, delimiterBeg.c_str());
			}
			newToken3.pop_front();
		}
	}
	~AVSignRecord() {}

	std::string Name;
	int NameLen;
	std::string Type;
	int Offset;
	std::list<std::string> Sign;
};
