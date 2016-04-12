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
    void setKeyFile(const std::string& f);
    void setWipeMethod(WipeMethod w);
    void setWipeProgram(const std::string& p);
    void setDecryptionPlace(DecryptionPlace p);
    void setDecryptionFolder(const std::string& f);

    KeyStorage keyStorage() const;
    std::string keyFile() const;
    WipeMethod wipeMethod() const;
    std::string wipeProgram() const;
    DecryptionPlace decryptionPlace() const;
    std::string decryptionFolder() const;

private:
    const std::string iniName = "settings.ini";
    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;
    void updateKeys();

    std::string encryptString(const std::string& src);
    std::string decryptString(const std::string& src);

    KeyStorage m_keyStorage;
    std::string m_keyFile;
    WipeMethod m_wipeMethod;
    std::string m_wipeProgram;

    DecryptionPlace m_decryptionPlace;
    std::string m_decryptionFolder;
};

#endif // OPTIONS_H
