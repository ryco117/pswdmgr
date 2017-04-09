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

#ifdef WINDOWS
    #include <Wincrypt.h>
#endif

#ifndef SCRYPT_WORK_VALUE
	#define SCRYPT_WORK_VALUE 1048576
    #define SCRYPT_CPU_VALUE 1
    #define SCRYPT_RAM_VALUE 8
#endif

using namespace std;

pswdmgr::pswdmgr(const std::string& fileName, const uint8_t* password)
{
	// Open file for reading
	ifstream file(fileName, ios::binary | ios::in);
	if(!file.is_open())
		throw runtime_error("Cannot open " + fileName);

	// Get filesize
	unsigned int fileSize;
	file.seekg(0, ios::end);
	fileSize = file.tellg();
	file.seekg(0, ios::beg);

	// Filesize check
	if(fileSize % 16 != 0)
		throw runtime_error("File is not valid");

	// Ensure fprng is seeded
	Seed();

	// Read salt and IV for decrypting pswds, or generate new salt
	if(fileSize < 32)
	{
		fprng.GenerateBlocks(salt.data(), 1);
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
    libscrypt_scrypt(password, strlen((const char*)password), salt.data(), salt.size(), SCRYPT_WORK_VALUE, SCRYPT_RAM_VALUE, SCRYPT_CPU_VALUE, key.Get(), 32);

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
        pswds[site] = make_shared<UserPass>(username, std::move(sitePassword));
	}
}

void pswdmgr::AddSite(const string& site, const string& username, const uint8_t* password)
{
	// Allocate memory for user/pass and store as shared ptr, add to map
    pswds[site] = make_shared<UserPass>(username, std::move(SecureString((char*)password)));
}
void pswdmgr::AddSite(const string& site, const std::shared_ptr<UserPass>& uPass)
{
    // Allocate memory for user/pass and store as shared ptr, add to map
    pswds[site] = uPass;
}

void pswdmgr::RemoveSite(const std::string& site)
{
    pswds.erase(site);
}

void pswdmgr::WriteOut(const string& fileName)
{
	ofstream file(fileName, ios::trunc | ios::binary | ios::out);
	if(file.is_open())
	{
		// Write same salt to file, so password can generate correct key next time
		file.write((char*)salt.data(), 16);

		// Generate new IV
		fprng.GenerateBlocks(iv.data(), 1);
		file.write((char*)iv.data(), 16);

		// Determine size needed to store all triplets in encrypted buffer
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
		bufferSize = AES::PaddedSize(bufferSize);

		// Create buffer to store encrypted data
		SecureString buffer;
		buffer.ResizeToFit(bufferSize);

		// Write triplets to buffer
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

		// Encrypt buffer and write to file
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
    return pswds[site];
}
std::shared_ptr<pswdmgr::UserPass> pswdmgr::At(const std::string& site)
{
    return pswds[site];
}

void pswdmgr::CreateKey(const uint8_t* password)
{
	fprng.GenerateBlocks(salt.data(), 1);
	libscrypt_scrypt(password, strlen((const char*)password), salt.data(), salt.size(), SCRYPT_WORK_VALUE, 8, 1, key.Get(), key.Size());
}

void pswdmgr::Seed()
{
    SecureArray <1024> seed;

    #ifdef WINDOWS
        HCRYPTPROV cspHndl;
        CryptAcquireContext(&cspHndl, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
        CryptGenRandom(cspHndl, 1024, seed);
        CryptReleaseContext(cspHndl, 0);
    #else
        ifstream rand("/dev/random", ios::in | ios::binary);
        if(rand.is_open())
        {
            rand.read((char*)seed.Get(), 1024);
        }
        else
        {
            throw runtime_error("Could not open default seed /dev/random");
        }
    #endif

    fprng.Seed(seed.Get(), 1024);
}
