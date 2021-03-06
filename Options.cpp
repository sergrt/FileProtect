#include "Options.h"
#include <QSettings>

#include "../cryptopp/aes.h"
#include "../cryptopp/modes.h"
#include "../cryptopp/filters.h"
#include "CryptoppUtils.h"

// Key and IV for options encryption
// keyStr should be 64 symbols (to be converted to 32 bytes) and ivStr should be 32 symbols
const std::string keyStr = "fa3ead1c0618de10a203a43aef0b19cc"
                           "269a666774eb61d7895fbe10e1496dd7";
const std::string ivStr =  "663e27b75f0795b66af1e58961a4f309";

namespace OptionsText {
    const std::string iniName = "settings.ini";
    namespace Groups {
        const QString general = "General";
    }
    namespace KeyStorage {
        const QString keyStorage = "KeyStorage";
        const std::string keyboard = "keyboard";
        const std::string file = "file";
    }
    const QString keyFile = "KeyFile";
    namespace WipeMethod {
        const QString wipeMethod = "WipeMethod";
        const std::string regular = "regular";
        const std::string external = "external";
    }
    const QString wipeProgram = "WipeProgram";
    namespace DecryptionPlace {
        const QString decryptionPlace = "DecryptionPlace";
        const std::string inplace = "inplace";
        const std::string specified = "specified";
    }
    const QString decryptionFolder = "DecryptionFolder";
    const QString rootPath = "RootPath";
}

Options::Options() {
    // Some defaults
    m_keyStorage = KeyStorage::Keyboard;
    m_wipeMethod = WipeMethod::Regular;
    m_decryptionPlace = DecryptionPlace::Inplace;
    //m_keyFile = "keyfile.key";

    load();
}

void Options::updateKeys() {
    if (key.size() != CryptoPP::AES::MAX_KEYLENGTH && iv.size() != CryptoPP::AES::BLOCKSIZE) {
        key.resize(CryptoPP::AES::MAX_KEYLENGTH);
        iv.resize(CryptoPP::AES::BLOCKSIZE);
    }

    memcpy(key.BytePtr(), CryptoPPUtils::HexDecodeString(keyStr).BytePtr(), CryptoPP::AES::MAX_KEYLENGTH);
    memcpy(iv.BytePtr(), CryptoPPUtils::HexDecodeString(ivStr).BytePtr(), CryptoPP::AES::BLOCKSIZE);
}


std::string Options::encryptString(const std::wstring& src) {
    CryptoPP::AES::Encryption aesEncryption(key, key.size());
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    std::string ciphertext;
    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
    //stfEncryptor.Put(reinterpret_cast<const unsigned char*>(src.c_str()), src.length() + 1);

    const int sz = std::wcstombs(nullptr, &src[0], src.size());
    std::unique_ptr<char[]> c;
    if (sz != -1) {
        c.reset(new char[sz]);
        std::wcstombs(c.get(), &src[0], src.size());
    } else {
        throw std::runtime_error("Error converting string");
    }

    stfEncryptor.Put(reinterpret_cast<const unsigned char*>(c.get()), sz);
    stfEncryptor.MessageEnd();

    return CryptoPPUtils::HexEncodeString(ciphertext);
}

std::wstring arrToWstr(const std::string& in) {
    std::wstring res;
    const int sz = std::mbstowcs(nullptr, in.c_str(), in.size());
    if (sz != -1) {
        std::unique_ptr<wchar_t[]> t(new wchar_t[sz]);
        std::mbstowcs(t.get(), in.c_str(), in.size());
        res = std::wstring(t.get(), sz);
    } else {
        throw std::runtime_error("Error converting string");
    }
    return res;
}


std::wstring Options::decryptString(const std::string& src) {
    CryptoPP::SecByteBlock tmp = CryptoPPUtils::HexDecodeString(src);
    std::string srcDecoded(reinterpret_cast<char*>(tmp.BytePtr()), tmp.size());

    CryptoPP::AES::Decryption aesDecryption(key, key.size());
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

    std::string decryptedtext;
    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
    stfDecryptor.Put(reinterpret_cast<const unsigned char*>(srcDecoded.c_str()), srcDecoded.size());
    stfDecryptor.MessageEnd();

    return arrToWstr(decryptedtext);
}

bool Options::load() {
    bool res = true;
    try {
        updateKeys();
        QSettings s(OptionsText::iniName.c_str(), QSettings::IniFormat);
        s.beginGroup(OptionsText::Groups::general);

        KeyStorage k;
        if (!fromString(k, s.value(OptionsText::KeyStorage::keyStorage, "").toString().toStdString()))
            throw std::runtime_error("Error converting options to string");
        setKeyStorage(k);

        const std::string keyFile = s.value(OptionsText::keyFile, "").toString().toStdString();
        setKeyFile(decryptString(keyFile));

        WipeMethod w;
        if (!fromString(w, s.value(OptionsText::WipeMethod::wipeMethod, "").toString().toStdString()))
            throw std::runtime_error("Error converting options to string");
        setWipeMethod(w);

        const std::string wipeProgram = s.value(OptionsText::wipeProgram, "").toString().toStdString();
        setWipeProgram(decryptString(wipeProgram));
    
        DecryptionPlace d;
        if (!fromString(d, s.value(OptionsText::DecryptionPlace::decryptionPlace, "").toString().toStdString()))
            throw std::runtime_error("Error converting options to string");
        setDecryptionPlace(d);

        const std::string decryptionFolder = s.value(OptionsText::decryptionFolder, "").toString().toStdString();
        setDecryptionFolder(decryptString(decryptionFolder));

        const std::string rootPath = s.value(OptionsText::rootPath, "").toString().toStdString();
        setRootPath(decryptString(rootPath));
    } catch (std::exception&) {
        res = false;
    }

    return res;
}

bool Options::save() {
    bool res = true;
    try {
        updateKeys();
        QSettings s(OptionsText::iniName.c_str(), QSettings::IniFormat);
        s.beginGroup(OptionsText::Groups::general);

        s.setValue(OptionsText::KeyStorage::keyStorage, toString(keyStorage()).c_str());
        const std::string keyFileEncrypted = encryptString(keyFile());
        s.setValue(OptionsText::keyFile, keyFileEncrypted.c_str());

        s.setValue(OptionsText::WipeMethod::wipeMethod, toString(wipeMethod()).c_str());
        const std::string wipeProgramEncrypted = encryptString(wipeProgram());
        s.setValue(OptionsText::wipeProgram, wipeProgramEncrypted.c_str());

        s.setValue(OptionsText::DecryptionPlace::decryptionPlace, toString(decryptionPlace()).c_str());
        const std::string decryptionFolderEncrypted = encryptString(decryptionFolder());
        s.setValue(OptionsText::decryptionFolder, decryptionFolderEncrypted.c_str());

        const std::string rootPathEncrypted = encryptString(rootPath());
        s.setValue(OptionsText::rootPath, rootPathEncrypted.c_str());
    } catch (std::exception&) {
        res = false;
    }
    return res;
}

Options::KeyStorage Options::keyStorage() const {
    return m_keyStorage;
}
Options::WipeMethod Options::wipeMethod() const {
    return m_wipeMethod;
}
Options::DecryptionPlace Options::decryptionPlace() const {
    return m_decryptionPlace;
}

std::wstring Options::keyFile() const {
    return m_keyFile;
}
std::wstring Options::wipeProgram() const {
    return m_wipeProgram;
}
std::wstring Options::decryptionFolder() const {
    return m_decryptionFolder;
}
std::wstring Options::rootPath() const {
    return m_rootPath;
}

void Options::setKeyStorage(KeyStorage k) {
    m_keyStorage = k;
}
void Options::setWipeMethod(WipeMethod w) {
    m_wipeMethod = w;
}
void Options::setDecryptionPlace(DecryptionPlace d) {
    m_decryptionPlace = d;
}

void Options::setKeyFile(const std::wstring& f) {
    m_keyFile = f;
}
void Options::setWipeProgram(const std::wstring& p) {
    m_wipeProgram = p;
}
void Options::setDecryptionFolder(const std::wstring& f) {
    m_decryptionFolder = f;
}

void Options::setRootPath(const std::wstring& p) {
    m_rootPath = p;
}

std::string Options::toString(KeyStorage k) {
    std::string res;
    switch (k) {
    case KeyStorage::Keyboard:
        res = OptionsText::KeyStorage::keyboard;
        break;
    case KeyStorage::File:
        res = OptionsText::KeyStorage::file;
        break;
    default:
        throw std::runtime_error("Not all KeyStorage values covered");
        break;
    }
    return res;
}
bool Options::fromString(KeyStorage& k, const std::string& s) {
    bool res = false;
    if (s == OptionsText::KeyStorage::keyboard) {
        k = KeyStorage::Keyboard;
        res = true;
    } else if (s == OptionsText::KeyStorage::file) {
        k = KeyStorage::File;
        res = true;
    }
    return res;
}
std::string Options::toString(WipeMethod w) {
    std::string res;
    switch (w) {
    case WipeMethod::Regular:
        res = OptionsText::WipeMethod::regular;
        break;
    case WipeMethod::External:
        res = OptionsText::WipeMethod::external;
        break;
    default:
        throw std::runtime_error("Not all WipeMethod values covered");
        break;
    }
    return res;
}
bool Options::fromString(WipeMethod& w, const std::string& s) {
    bool res = false;
    if (s == OptionsText::WipeMethod::regular) {
        w = WipeMethod::Regular;
        res = true;
    } else if (s == OptionsText::WipeMethod::external) {
        w = WipeMethod::External;
        res = true;
    }
    return res;
}
std::string Options::toString(DecryptionPlace d) {
    std::string res;
    switch (d) {
    case DecryptionPlace::Inplace:
        res = OptionsText::DecryptionPlace::inplace;
        break;
    case DecryptionPlace::Specified:
        res = OptionsText::DecryptionPlace::specified;
        break;
    default:
        throw std::runtime_error("Not all DecryptionPlace values covered");
        break;
    }
    return res;
}
bool Options::fromString(DecryptionPlace& d, const std::string& s) {
    bool res = false;
    if (s == OptionsText::DecryptionPlace::inplace) {
        d = DecryptionPlace::Inplace;
        res = true;
    } else if (s == OptionsText::DecryptionPlace::specified) {
        d = DecryptionPlace::Specified;
        res = true;
    }
    return res;
}
