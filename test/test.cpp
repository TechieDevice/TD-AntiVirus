#pragma once

#include <iostream>
#include <fstream>
#include <iostream>
#include <list>
#include <Windows.h>
#include <string>
#include <Wincrypt.h>
#include <sstream>
#include <iomanip>

#include "../core/scanner.h"
#include <vector>
using namespace std;

int char2int(char input)
{
    if (input >= '0' && input <= '9')
        return input - '0';
    if (input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if (input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}

void hex2bin(const char* src, char* target)
{
    while (*src && src[1])
    {
        *(target++) = char2int(*src) * 16 + char2int(src[1]);
        src += 2;
    }
}

string ToHex(const string& s, bool upper_case)
{
    ostringstream ret;

    for (string::size_type i = 0; i < s.length(); ++i)
    {
        int z = s[i] & 0xff;
        ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << z;
    }

    return ret.str();
}


void createFile()
{
    std::string wut = "12f4dd6d70241f674d8fc13e1eb3af731a7b5c43173c1cdd75722fa556c373b65c5275d513147b070077757064080386898ae75c6fb7f717b562ef636f6d6d613f2e0e202f6336c5eed52064f120228e2f6d27c10134";

    std::ifstream datafile1("d:\\temp\\test1.exe", std::ios::binary | ios::ate);
    //std::ifstream datafile2("d:\\temp\\test2.exe", std::ios::binary);

    //std::ofstream datafile1("d:\\temp\\test1.exe", std::ios::binary);
    std::ofstream datafile2("d:\\temp\\test1.exe", std::ios::binary);

    char buf[3];
    buf[2] = 0;

    std::stringstream input(wut);
    input.flags(std::ios::hex);
    while (input)
    {
        input >> buf[0] >> buf[1];
        long val = strtol(buf, nullptr, 16);
        datafile2 << static_cast<unsigned char>(val & 0xff);
    }
    datafile2.close();

    cout << '\n';
    
    //char* tmp = new char[wut.size()];
    //hex2bin(wut.c_str(), tmp);
    //cout << tmp << '\n';
    //cout << wut.size() << "\n";
    ifstream::pos_type size = datafile1.tellg();
    datafile1.seekg(0, ios::beg);
    char* tmp1 = new char[size];
    datafile1.read(tmp1, size);
    cout << tmp1 << '\n';
    std::string tohexed = ToHex(string(tmp1, size), true);
    cout << tohexed << "\n";
    //for (int i = 0; i < )

    //cout << tmp1 << "\n";
    datafile1.close();
}


int main()
{
    createFile();
    Scanner::AVstart("../core", "D:\\temp\\test");

    return 0;
}
