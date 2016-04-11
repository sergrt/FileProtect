#include "Options.h"
#include <QSettings>

//#include "../cryptopp/aes.h"
#include "../cryptopp/modes.h"
#include "CryptoppUtils.h"

const std::string keyStr = "12345678901234567890123456789012";
const std::string ivStr = "1234567890123456";



Options::Options()
    /*: key(CryptoPPUtils::HexDecodeString(keyStr)), iv(CryptoPPUtils::HexDecodeString(ivStr))*/ {

    m_keyStorage = KeyStorage::Keyboard;
    m_wipeMethod = WipeMethod::Regular;
    m_keyFile = "123";
    m_wipeProgram = "123";
/*
    const std::string keyStr = "12345678901234567890123456789012";
    const std::string ivStr = "1234567890123456";


    key = CryptoPPUtils::HexDecodeString(keyStr);
    iv = CryptoPPUtils::HexDecodeString(ivStr);
    */



    load();

}

void Options::updateKeys() {
    std::string hexKey(CryptoPP::AES::DEFAULT_KEYLENGTH,'6');
    std::string hexIv(CryptoPP::AES::BLOCKSIZE, '3');
    key = CryptoPPUtils::HexDecodeString(hexKey);
    iv = CryptoPPUtils::HexDecodeString(hexIv);
}

void Options::load() {

    updateKeys();


    QSettings s(iniName.c_str(), QSettings::IniFormat);
    s.beginGroup("General");

    fromString(m_keyStorage, s.value("KeyStorage", "").toString().toStdString());

    std::string keyFile = s.value("KeyFile", "").toString().toStdString();

    m_keyFile = decryptString(keyFile);

    fromString(m_wipeMethod, s.value("WipeMethod", "").toString().toStdString());
    std::string wipeProgram = s.value("WipeProgram", "").toString().toStdString();
    try {
        m_wipeProgram = decryptString(wipeProgram);
    } catch( CryptoPP::Exception& e ) {
        exit(1);
    }

    fromString(m_decryptionPlace, s.value("DecryptionPlace", "").toString().toStdString());
    std::string decryptionFolder = s.value("DecryptionFolder", "").toString().toStdString();
    try {
        m_decryptionFolder = decryptString(decryptionFolder);
    } catch( CryptoPP::Exception& e ) {
        exit(1);
    }
}

#include <sstream>
#include <iomanip>
#include "../cryptopp/base64.h"
std::string Options::encryptString(const std::string& src) {

    using namespace CryptoPP;
    CTR_Mode<AES>::Encryption e(key, key.size(), iv);

    byte* b = new byte[src.size()];
    for (int i = 0; i < src.size(); ++i)
        b[i] = src[i];

    std::string cipher;
    StringSource ss(b, true, new StreamTransformationFilter(e, new StringSink(cipher)));

    byte




    //return src;
    using namespace CryptoPP;

    CTR_Mode<AES>::Encryption e(key, key.size(), iv);

    std::string cipher;
    StringSource ss(src, true, new StreamTransformationFilter(e, new StringSink(cipher)));

    //std::string hexStr = CryptoPPUtils::HexEncodeString(cipher);
    std::string hexStr = cipher;

    return hexStr;
}
std::string Options::decryptString(const std::string& src) {
    //return src;

    using namespace CryptoPP;
    CTR_Mode<AES>::Decryption d(key, key.size(), iv);

    //SecByteBlock b = CryptoPPUtils::HexDecodeString(src);
    std::string b = src;

    std::string plain;
    StringSource ss(b, true, new StreamTransformationFilter(d, new StringSink(plain)));

    return plain;
/*
    std::string plain;
    StringSource ss1(src, true, new StreamTransformationFilter(e, new StringSink(cipher)));

    std::string hexStr;
    StringSource ss2(cipher, true, new HexEncoder(new StringSink(hexStr)));
    return hexStr;
    */
}

void Options::save() {
    updateKeys();
    QSettings s(iniName.c_str(), QSettings::IniFormat);
    s.beginGroup("General");
    s.setValue("KeyStorage", toString(m_keyStorage).c_str());


    std::string keyFileEncrypted = encryptString(m_keyFile);

    s.setValue("KeyFile", keyFileEncrypted.c_str());
    s.setValue("WipeMethod", toString(m_wipeMethod).c_str());

    std::string wipeProgramEncrypted;
    try {
        wipeProgramEncrypted = encryptString(m_wipeProgram);
    } catch( CryptoPP::Exception& e ) {
        //cerr << e.what() << endl;
        exit(1);
    }
    s.setValue("WipeProgram", wipeProgramEncrypted.c_str());

    s.setValue("DecryptionPlace", toString(m_decryptionPlace).c_str());
    std::string decryptionFolderEncrypted;
    try {
        decryptionFolderEncrypted = encryptString(m_decryptionFolder);
    } catch( CryptoPP::Exception& e ) {
        //cerr << e.what() << endl;
        exit(1);
    }
    s.setValue("DecryptionFolder", decryptionFolderEncrypted.c_str());

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

std::string Options::keyFile() const {
    return m_keyFile;
}
std::string Options::wipeProgram() const {
    return m_wipeProgram;
}
std::string Options::decryptionFolder() const {
    return m_decryptionFolder;
}

void Options::setKeyStorage(KeyStorage k) {
    m_keyStorage = k;
}

void Options::setKeyFile(const std::string& f) {
    m_keyFile = f;
}

void Options::setWipeMethod(WipeMethod w) {
    m_wipeMethod = w;
}

void Options::setWipeProgram(const std::string& p) {
    m_wipeProgram = p;
}

void Options::setDecryptionPlace(DecryptionPlace d) {
    m_decryptionPlace = d;
}

void Options::setDecryptionFolder(const std::string &f) {
    m_decryptionFolder = f;
}



bool Options::validate() const {
    const KeyStorage k = keyStorage();
    const WipeMethod w = wipeMethod();
    const DecryptionPlace d = decryptionPlace();
    return (
        (k == KeyStorage::Keyboard || (k == KeyStorage::File && keyFile().size() != 0))
        && (w == WipeMethod::Regular || (w == WipeMethod::External && wipeProgram().size() != 0))
        && (d == DecryptionPlace::Inplace || (d == DecryptionPlace::Specified && decryptionFolder().size() != 0))
                );
}
