#include "pch.h"
#include "scanner.h"
#include "AVBFile.h"

std::list<std::string> detected;

std::list<AVSignRecord> signlist;
std::list<AVHashRecord> hashlist;

typedef struct pathdata {
	std::string FileName;
	const unsigned char* filedata;
	std::ifstream::pos_type size;
} PATHDATA, *PPATHDATA;


std::list<std::string> signGet(std::string sign)
{
	std::list<std::string> signParts;

	return signParts;
}

std::string ToHex(const std::string& s, bool upper_case)
{
	std::ostringstream ret;

	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		int z = s[i] & 0xff;
		ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << z;
	}

	return ret.str();
}


DWORD WINAPI checkSignFile(LPVOID lpParam)
{
	PPATHDATA pPathData = (PPATHDATA)lpParam;

	std::string filedata = ToHex(std::string((char*)pPathData->filedata, pPathData->size), false);

	for (AVSignRecord& Record : signlist)
	{
		for (int i = 0; i < Record.Sign.size(); )
		{
			std::size_t find = filedata.find(Record.Sign.front(), Record.Offset);
			if (find != std::string::npos)
			{
				if (i == (Record.Sign.size() - 1))
				{
					detected.push_back(Record.Name);
					std::string filename = pPathData->FileName;
					detected.push_back(filename);
				}
			}
			else break;

			Record.Sign.pop_front();
		}
	}

	return 0;
}


DWORD WINAPI checkHashFile(LPVOID lpParam)
{
	PPATHDATA pPathData = (PPATHDATA)lpParam;

	for (AVHashRecord& Record : hashlist)
	{
		if (pPathData->size != Record.FileLen) return 0;

		std::string Hash;

		Hash = reinterpret_cast<char*>(MD5(pPathData->filedata, pPathData->size, NULL));

		std::string HexHash = ToHex(Hash, false);

		if (HexHash == Record.Hash) {
			detected.push_back(Record.Name);
			std::string filename = pPathData->FileName;
			detected.push_back(filename);
		}
	}	

	return 0;
}

void processPath(PCSTR Path)
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
				processPath(folder.c_str());
			}
			else 
			{	
				HANDLE threads[2];

				PPATHDATA Pathdata = (PPATHDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PATHDATA));
				if (Pathdata == NULL)
				{
					ExitProcess(2);
				}

				std::string FileName = filepath + FindData.cFileName;
				std::ifstream::pos_type size;
				const unsigned char* filedata;
				std::ifstream file(FileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
				if (file)
				{
					size = file.tellg();
					file.seekg(0, std::ios::beg);
					filedata = new unsigned char[size];
					file.read((char*)filedata, size);
				}
				else continue;

				Pathdata->FileName = FileName;
				Pathdata->filedata = filedata;
				Pathdata->size = size;

				threads[0] = CreateThread(NULL, 0, checkHashFile, Pathdata, 0, NULL);
				if (threads[0] == NULL)	ExitProcess(3);

				threads[1] = CreateThread(NULL, 0, checkSignFile, Pathdata, 0, NULL);
				if (threads[1] == NULL)	ExitProcess(3);
				
				WaitForMultipleObjects(2, threads, TRUE, INFINITE);
				CloseHandle(threads[0]);
				CloseHandle(threads[1]);
				if (Pathdata != NULL)
				{
					HeapFree(GetProcessHeap(), 0, Pathdata);
					Pathdata = NULL;
				}
			}
		} while (FindNextFileA(h, &FindData));
	}
}

void Scanner::AVstart(PCSTR DBPath, PCSTR Path)
{
	AVSignBFile SignBase;
	AVHashBFile HashBase;

	std::string signDBPath = DBPath;
	signDBPath += "/sign.sdb";
	if (!SignBase.open(signDBPath.c_str()))
	{
		detected.push_back((char*)"sign DB error");
		return;
	}

	std::string hashDBPath = DBPath;
	hashDBPath += "/hash.hdb";
	if (!HashBase.open(hashDBPath.c_str()))
	{
		detected.push_back((char*)"hash DB error");
		return;
	}

	signlist = SignBase.signlist;
	hashlist = HashBase.hashlist;

	SignBase.close();
	HashBase.close();

	processPath(Path);

	if(detected.size() == 0) detected.push_back((char*)"no viruses detected");
}

void Scanner::GetNextString(BYTE* byteString)
{
	if (detected.empty()) return;

	std::string String = detected.front();
	detected.pop_front();
	std::memcpy(byteString, (BYTE*)String.c_str(), sizeof(String));
}


//void Scanner::Delete()
//{
//	if (detected.empty()) return;
//
//	std::list<std::string>::iterator detected_front = detected.begin();
//	++detected_front;
//	for (int i = 0; i < detected.size()/2; i++)
//	{
//		std::string filename = *detected_front;
//		remove(filename.c_str());
//		std::advance(detected_front, 2);
//	}
//}
//
//void Scanner::Encrypt(BYTE* Key, BYTE* IV)
//{
//	if (detected.empty()) return;
//
//	std::list<std::string>::iterator detected_front = detected.begin();
//	++detected_front;
//	for (int i = 0; i < detected.size() / 2; i++)
//	{
//		std::string filename = *detected_front;
//		
//		std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
//		if (file)
//		{
//			std::ifstream::pos_type size = file.tellg();
//			file.seekg(0, std::ios::beg);
//			unsigned char* filedata = new unsigned char[size];
//			file.read((char*)filedata, size);
//
//			// Make a copy of the IV to IVd as it seems to get destroyed when used
//			uint8_t IVd[AES_BLOCK_SIZE];
//			for (int i = 0; i < AES_BLOCK_SIZE; i++) 
//			{
//				IVd[i] = IV[i];
//			}
//
//			/** Setup the AES Key structure required for use in the OpenSSL APIs **/
//			AES_KEY* AesKey = new AES_KEY();
//			AES_set_encrypt_key(Key, 128, AesKey);
//			int blocks = (size / 16) + 1;
//
//			for (int i = 1; i <= blocks; i++)
//			{
//				/** take an input string and pad it so it fits into 16 bytes (AES Block Size) **/
//
//				//const int UserDataSize = (const int)size;   // Get the length pre-padding
//				//int RequiredPadding = (AES_BLOCK_SIZE - (size % AES_BLOCK_SIZE));   // Calculate required padding
//				//std::vector<unsigned char> PaddedTxt(txt.begin(), txt.end());   // Easier to Pad as a vector
//				//for (int i = 0; i < RequiredPadding; i++) {
//				//	PaddedTxt.push_back(0); //  Increase the size of the string by
//				//}                           //  how much padding is necessary
//
//				//unsigned char* Data = &PaddedTxt[0];// Get the padded text as an unsigned char array
//				//const int UserDataSizePadded = (const int)PaddedTxt.size();// and the length (OpenSSl is a C-API)
//
//				/** Peform the encryption **/
//				int blockSize;
//				if (i != (blocks)) blockSize = 16;
//				else blockSize = size % 16;
//
//				unsigned char* Data = new unsigned char[blockSize];
//				unsigned char EncryptedData[16] = { 0 }; // Hard-coded Array for OpenSSL (C++ can't dynamic arrays)
//				
//				for (int j = 0; j < blockSize; j++)
//				{
//					Data[j] = filedata[i * (j)];
//				}
//
//				AES_cbc_encrypt(filedata, EncryptedData, size, (const AES_KEY*)AesKey, IVd, AES_ENCRYPT);
//
//
//				/** Setup an AES Key structure for the decrypt operation **/
//				AES_KEY* AesDecryptKey = new AES_KEY(); // AES Key to be used for Decryption
//				AES_set_decrypt_key(Key, 128, AesDecryptKey);   // We Initialize this so we can use the OpenSSL Encryption API
//
//				/** Decrypt the data. Note that we use the same function call. Only change is the last parameter **/
//				unsigned char DecryptedData[16] = { 0 }; // Hard-coded as C++ doesn't allow for dynamic arrays and OpenSSL requires an array
//				AES_cbc_encrypt(EncryptedData, DecryptedData, size, (const AES_KEY*)AesDecryptKey, IVd, AES_DECRYPT);
//				int kostil = 0;
//			}
//		}
//		std::advance(detected_front, 2);
//	}
//}
