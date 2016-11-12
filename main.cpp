//Local includes
#include "myconio.h"
#include "pswdmgr.h"

// Crypto
#include <libscrypt.h>
#include <crypto/AES.h>
#include <crypto/SecureString.h>

// Nix
#include <unistd.h>

// Stds
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

const string helpStr = "pswdmgr written by ryco117@gmail.com\n\n"
"Arguments List:\n"
"===============\n"
"-f  --file file_path,      specify path to encrypted password file (default is ~/.pswds)\n"
"-a  --add some.site.net,   allows addition of username/password pair (otherwrites any existing entry)\n"
"-v  --view some.site.net,  retrieve username and password pair (if exists)\n"
"-l  --list,                list all username/password pairs, for each website\n"
"-h  --help,                print this list and exit\n\n";

void ReadPassword(SecureString& password);
void PrintNSpaces(unsigned int n);
void ContByteOrThrow(const unsigned char& c);
void KBHitOrThrow();

int main(int argc, char** argv)
{
	string pswdsFileName = string(getenv("HOME")) + string("/.pswds");
	string addSite, viewSite;
	bool listAll = false;

	// Read command line args
	for(unsigned int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if((arg == "-f" || arg == "--file") && ++i < argc)
			pswdsFileName = argv[i];
		else if((arg == "-a" || arg == "--add") && ++i < argc)
			addSite = argv[i];
		else if((arg == "-v" || arg == "--view") && ++i < argc)
			viewSite = argv[i];
		else if(arg == "-l" || arg == "--list")
			listAll = true;
		else if(arg == "-h" || arg == "--help")
		{
			cout << helpStr;
			return -1;
		}
		else
			cerr << "Unknown argument \"" << argv[i] << "\"";
	}

	try
	{
		// Read file password and retrieve data
		SecureString masterPass;
		cout << "Master Password: ";
		ReadPassword(masterPass);

		pswdmgr mgr(pswdsFileName, masterPass.GetStr());

		// Check if adding a site
		if(!addSite.empty())
		{
			string username;
			cout << "Username/ID for " << addSite << ": ";
			getline(cin, username);

			// Read Password
			SecureString sitePass;
			cout << "Password: ";
			ReadPassword(sitePass);

			mgr.AddSite(addSite, username, sitePass.GetStr());
			mgr.WriteOut(pswdsFileName);
		}

		//... viewing a site
		if(!viewSite.empty())
		{
			try
			{
				shared_ptr<pswdmgr::UserPass> userPass = mgr[viewSite];
				cout << "Username/ID                     | Password\n"
					<< "========================================\n";

				cout << userPass->username;
				PrintNSpaces(32 - userPass->username.length());
				cout << "| " << userPass->password.GetStr() << endl;
			}
			catch(const exception&)
			{
				cerr << "Cannot find " << viewSite << endl;
			}
			fflush(stdout);
		}

		// ... list all
		if(listAll)
		{
			cout << "Site                            | Username/ID                     | Password\n"
				<< "============================================================================\n";

			for(map<string, shared_ptr<pswdmgr::UserPass>>::iterator it = mgr.IterBegin(); it != mgr.IterEnd(); ++it)
			{
				cout << it->first;
				PrintNSpaces(32 - it->first.length());
				cout << "| " << it->second->username;
				PrintNSpaces(32 - it->second->username.length());
				cout << "| " << it->second->password.GetStr() << endl;
				fflush(stdout);
			}
		}
	}
	catch(const std::runtime_error& e)
	{
		nonblock(false, true);
		cerr << e.what() << endl;
		fflush(stderr);
		return -100;
	}
	return 0;
}

void PrintNSpaces(unsigned int n)
{
	for(int i = 0; i < n; i++)
	{
		cout << " ";
	}
}

void ReadPassword(SecureString& password)
{
	fflush(stdout);
        nonblock(true, false);
        password.Clear();
        while(true)
        {
                if(kbhit())
                {
                        unsigned char c = getch();

			// Assume UTF-8, check byte length
			if(c >= 0x20 && c <= 0x7E)
			{
				// Single char
				password.Append((char&)c);
			}
			else if(c >= 0xC0 && c <= 0xDF)
			{
				// Double char
				unsigned char c_1 = getch(); ContByteOrThrow(c_1);
				password.Append((char&)c);
				password.Append((char&)c_1);
			}
			else if(c >= 0xE0 && c <= 0xEF)
			{
				// Triple char
				unsigned char c_1 = getch(); ContByteOrThrow(c_1);
				unsigned char c_2 = getch(); ContByteOrThrow(c_2);
				password.Append((char&)c);
				password.Append((char&)c_1);
				password.Append((char&)c_2);
			}
			else if(c >= 0xF0 && c <= 0xF7)
			{
				// Quad char
				unsigned char c_1 = getch(); ContByteOrThrow(c_1);
				unsigned char c_2 = getch(); ContByteOrThrow(c_2);
				unsigned char c_3 = getch(); ContByteOrThrow(c_3);
				password.Append((char&)c);
				password.Append((char&)c_1);
				password.Append((char&)c_2);
				password.Append((char&)c_3);
			}
                        else if(c == '\n')
                        {
                                nonblock(false, true);
                                cout << endl;
                                fflush(stdout);

                                if(password.Empty())
					throw runtime_error("Empty password not accepted");

                                return;
                        }
			else if(c == 127 || c == 8)       //Backspace
                        {
                        	unsigned int len = password.GetLength();
                                if(len > 0)
                                {
                                	len -= 1;
                                	password[len] = 0;
                                        password.SetLength(len);
				}
                        }
                        else
                        {
                                throw runtime_error("Cannot read input as password");
                        }
                }
        }
}

void ContByteOrThrow(const unsigned char& c)
{
	if(c < 0x80 || c > 0xBF)
		throw runtime_error("Not a valid UTF-8 character");
}

void KBHitOrThrow()
{
	if(kbhit() == 0)
	{
		throw runtime_error("Invalid UTF-8 input");
	}
}
