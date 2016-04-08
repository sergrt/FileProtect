#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include "../cryptopp/aes.h"

class Options {
private:
    const std::string iniName = "settings.ini";
    const CryptoPP::SecByteBlock key;
    const CryptoPP::SecByteBlock iv;
    std::string encryptString(const std::string& src);
    std::string decryptString(const std::string& src);
public:
    enum class KeyStorage {
        Keyboard,
        File
    } m_keyStorage;
    std::string m_keyFile;

    enum class WipeMethod {
        Regular,
        External
    } m_wipeMethod;
    std::string m_wipeProgram;

    static std::string toString(KeyStorage k) {
        std::string res;
        switch (k) {
        case KeyStorage::Keyboard:
            res = "keyboard";
            break;
        case KeyStorage::File:
            res = "file";
            break;
        default:
            throw("Not all KeyStorage values covered");
            break;
        }

        return res;
    }
    static bool fromString(KeyStorage& k, const std::string& s) {
        bool res = false;
        if (s == "Keyboard") {
            k = KeyStorage::Keyboard;
            res = true;
        } else if (s == "file") {
            k = KeyStorage::File;
            res = true;
        }
        return res;
    }
    static std::string toString(WipeMethod w) {
        std::string res;
        switch (w) {
        case WipeMethod::Regular:
            res = "regular";
            break;
        case WipeMethod::External:
            res = "external";
            break;
        default:
            throw("Not all WipeMethod values covered");
            break;
        }

        return res;
    }
    static bool fromString(WipeMethod& w, const std::string& s) {
        bool res = false;
        if (s == "regular") {
            w = WipeMethod::Regular;
            res = true;
        } else if (s == "external") {
            w = WipeMethod::External;
            res = true;
        }

        return res;
    }

    Options();
    void save();
    void load();
    bool validate() const;

    void setKeyStorage(KeyStorage k);
    void setKeyFile(const std::string& f);
    void setWipeMethod(WipeMethod w);
    void setWipeProgram(const std::string& p);

    KeyStorage keyStorage() const;
    std::string keyFile() const;
    WipeMethod wipeMethod() const;
    std::string wipeProgram() const;
};

#endif // OPTIONS_H
