#include <crypto/SecureString.h>
#include <crypto/SecureArray.h>
#include <crypto/Fortuna.h>

#include <map>
#include <array>
#include <string>
#pragma once

class pswdmgr
{
public:
	struct UserPass
	{
		std::string username;
		SecureString password;
	};

	pswdmgr(const std::string& fileName, const uint8_t* password);
	void AddSite(const std::string& site, const std::string& username, const uint8_t* password);
	void WriteOut(const std::string& fileName);

  	std::map<std::string, std::shared_ptr<UserPass>>::iterator IterBegin();
	std::map<std::string, std::shared_ptr<UserPass>>::iterator IterEnd();

	std::shared_ptr<UserPass> operator[](const std::string&);

private:
	std::map<std::string, std::shared_ptr<UserPass>> pswds;
	FortunaPRNG fprng;
	std::array<uint8_t, 16> salt;
	std::array<uint8_t, 16> iv;
	SecureArray<32> key;

	void Seed();
};
