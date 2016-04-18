#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include "../cryptopp/secblock.h"

class Options {
public:
    enum class KeyStorage {
        Keyboard,
        File
    };
    enum class WipeMethod {
        Regular,
        External
    };
    enum class DecryptionPlace {
        Inplace,
        Specified
    };

    static std::string toString(KeyStorage k);
    static bool fromString(KeyStorage& k, const std::string& s);
    static std::string toString(WipeMethod w);
    static bool fromString(WipeMethod& w, const std::string& s);
    static std::string toString(DecryptionPlace d);
    static bool fromString(DecryptionPlace& d, const std::string& s);

    Options();
    bool save();
    bool load();

    void setKeyStorage(KeyStorage k);
    void setKeyFile(const std::wstring& f);
    void setWipeMethod(WipeMethod w);
    void setWipeProgram(const std::wstring& p);
    void setDecryptionPlace(DecryptionPlace p);
    void setDecryptionFolder(const std::wstring& f);

    void setRootPath(const std::wstring& p);

    KeyStorage keyStorage() const;
    std::wstring keyFile() const;
    WipeMethod wipeMethod() const;
    std::wstring wipeProgram() const;
    DecryptionPlace decryptionPlace() const;
    std::wstring decryptionFolder() const;

    std::wstring rootPath() const;
private:
    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;
    void updateKeys();

    std::string encryptString(const std::wstring& src);
    std::wstring decryptString(const std::string& src);

    KeyStorage m_keyStorage;
    std::wstring m_keyFile;
    WipeMethod m_wipeMethod;
    std::wstring m_wipeProgram;
    DecryptionPlace m_decryptionPlace;
    std::wstring m_decryptionFolder;

    std::wstring m_rootPath;
};

#endif // OPTIONS_H
