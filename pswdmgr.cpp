#include "pswdmgr.h"
#include <crypto/AES.h>
#include <crypto/SecureString.h>
#include <crypto/Base64.h>

extern "C"
{
	#include <libscrypt.h>
}
#include <string.h>

#include <fstream>
#include <stdexcept>

#ifndef SCRYPT_WORK_VALUE
	#define SCRYPT_WORK_VALUE 1048576
#endif

using namespace std;

pswdmgr::pswdmgr(const std::string& fileName, const uint8_t* password)
{
	ifstream file(fileName, ios::binary | ios::in);
	if(!file.is_open())
		throw runtime_error("Cannot open " + fileName);

	unsigned int fileSize;
	file.seekg(0, ios::end);
	fileSize = file.tellg();
	file.seekg(0, ios::beg);

	// Filesize check
	if(fileSize % 16 != 0)
		throw runtime_error("File is not valid");

	// Read or generate salt and IV
	if(fileSize < 32)
	{
		Seed();
		fprng.GenerateBlocks(salt.data(), 1);
		fprng.GenerateBlocks(iv.data(), 1);
		fileSize = 0;
	}
	else
	{
		file.read((char*)salt.data(), 16);
		file.read((char*)iv.data(), 16);
		fileSize -= 32;
	}

	// Read contents into self clearing array
	SecureString pswdsStr;
	pswdsStr.ResizeToFit(fileSize);
	file.read((char*)pswdsStr.GetStr(), fileSize);
	file.close();

	// Hash password with salt
	libscrypt_scrypt(password, strlen((const char*)password), salt.data(), salt.size(), SCRYPT_WORK_VALUE, 8, 1, key.Get(), 32);

	// Decrypt data if there's any
	unsigned int pswdDataSize = 0;
	if(fileSize != 0)
	{
		try
		{
			pswdDataSize = AES::Decrypt(pswdsStr.GetStr(), fileSize, iv, key, pswdsStr.GetStr());
		}
		catch(const std::exception&)
		{
			throw runtime_error("Incorrect password or corrupted data");
		}
	}

	// Read all data
	unsigned int pos = 0;
	while(pos < pswdDataSize)
	{
		string site = (const char*)&pswdsStr[pos];
		pos += site.size() + 1;

		string username = (const char*)&pswdsStr[pos];
		pos += username.size() + 1;

		SecureString sitePassword((char*)&pswdsStr[pos]);
		pos += sitePassword.GetLength() + 1;

		UserPass* upassPtr = new UserPass;
		upassPtr->username = username;
		upassPtr->password.PullFrom(sitePassword);
		pswds[site] = shared_ptr<UserPass>(upassPtr);
	}
}

void pswdmgr::AddSite(const string& site, const string& username, const uint8_t* password)
{
	UserPass* upassPtr = new UserPass({username, move(SecureString((char*)password))});
	pswds[site] = shared_ptr<UserPass>(upassPtr);
}

void pswdmgr::WriteOut(const string& fileName)
{
	ofstream file(fileName, ios::trunc | ios::binary | ios::out);
	if(file.is_open())
	{
		file.write((char*)salt.data(), 16);
		file.write((char*)iv.data(), 16);

		unsigned int bufferSize = 0;
		for(const pair<string, shared_ptr<UserPass>>& line : pswds)
		{
			string site = line.first;
			bufferSize += site.size() + 1;

			string username = line.second->username;
			bufferSize += username.size() + 1;

			const SecureString& password = line.second->password;
			bufferSize += password.GetLength() + 1;
		}

		// Create buffer to store encrypted data
		SecureString buffer;
		bufferSize = AES::PaddedSize(bufferSize);
		buffer.ResizeToFit(bufferSize);

		unsigned int pos = 0;
		for(const pair<string, shared_ptr<UserPass>>& line : pswds)
		{
			// Store site
			string site = line.first;
			memcpy(&buffer[pos], site.c_str(), site.size() + 1);
			pos += site.size() + 1;

			// ... username
			string username = line.second->username;
			memcpy(&buffer[pos], username.c_str(), username.size() + 1);
			pos += username.size() + 1;

			// ... password
			const SecureString& password = line.second->password;
			memcpy(&buffer[pos], (char*)password.GetStr(), password.GetLength() + 1);
			pos += password.GetLength() + 1;
		}

		AES::Encrypt(buffer.GetStr(), pos, iv, key, buffer.GetStr());
		file.write((char*)buffer.GetStr(), bufferSize);
	}
	else
	{
		throw runtime_error("Could not write to " + fileName);
	}
}

map<std::string, std::shared_ptr<pswdmgr::UserPass>>::iterator pswdmgr::IterBegin()
{
	return pswds.begin();
}

map<std::string, std::shared_ptr<pswdmgr::UserPass>>::iterator pswdmgr::IterEnd()
{
	return pswds.end();
}

std::shared_ptr<pswdmgr::UserPass> pswdmgr::operator[](const std::string& site)
{
	return pswds.at(site);
}

void pswdmgr::Seed()
{
	ifstream rand("/dev/urandom", ios::in | ios::binary);
	if(rand.is_open())
	{
		SecureArray<1024> seed;
		rand.read((char*)seed.Get(), 1024);
		fprng.Seed(seed.Get(), 1024);
	}
	else
	{
		throw runtime_error("Could not open default seed /dev/urandom");
	}
}
