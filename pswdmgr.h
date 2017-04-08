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

        UserPass(const std::string& user, SecureString pswd) :
            username(user) {password.PullFrom(pswd);}
	};

	// Read password-salt and pswds from fileName if entries exist, otherwise generate salt
	// Throws exception if cant open filename for reading, file is not valid, or decryption fails
	pswdmgr(const std::string& fileName, const uint8_t* password);
	pswdmgr() = delete;

	// Add username/password pair mapped by site to pswds
	void AddSite(const std::string& site, const std::string& username, const uint8_t* password);
    void AddSite(const std::string& site, const std::shared_ptr<UserPass>& uPass);

    // Remove the mapping from site to its user/pass (nothing if doesn't exist)
    void RemoveSite(const std::string& site);

	// Truncate fileName and write pswds using key and a random IV
	void WriteOut(const std::string& fileName);

	// Iterator to begin of pswds
  	std::map<std::string, std::shared_ptr<UserPass>>::iterator IterBegin();

	// Iterator to end of pswds
	std::map<std::string, std::shared_ptr<UserPass>>::iterator IterEnd();

	// Index UserPass pairs by site
    std::shared_ptr<UserPass> operator[](const std::string& site);
    std::shared_ptr<UserPass> At(const std::string& site);

	// Create new key from new salt and password
	void CreateKey(const uint8_t* password);

private:
	std::map<std::string, std::shared_ptr<UserPass>> pswds;
	FortunaPRNG fprng;
	std::array<uint8_t, 16> salt;
	std::array<uint8_t, 16> iv;
	SecureArray<32> key;

	void Seed();
};
