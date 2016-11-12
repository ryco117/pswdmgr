pswdmgr
=======
A simple utility for storing and retrieving passwords as securely as possible, using StrongHash(master password, randomly generated salt) as the 256 bit AES key used to encrypt the passwords.

Cryptographic operations are handled by my open source project CryptoLibrary (a consolidated [improved] library of the cryptographic functions I've written, still under development).

##Arguments List:
```
-f  --file file_path,      specify path to encrypted password file (default is ~/.pswds)
-a  --add some.site.net,   allows addition of username/password pair (otherwrites any existing entry)
-v  --view some.site.net,  retrieve username and password pair (if exists)
-l  --list,                list all username/password pairs, for each website
-h  --help,                print this list and exit
```
