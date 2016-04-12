#include "Options.h"
#include <QSettings>

#include "../cryptopp/aes.h"
#include "../cryptopp/modes.h"
#include "../cryptopp/filters.h"
#include "CryptoppUtils.h"

const std::string keyStr = "1234567890123456789012345678901212345678901234567890123456789012";
const std::string ivStr = "12345678901234561234567890123456";

Options::Options() {
    m_keyStorage = KeyStorage::Keyboard;
    m_wipeMethod = WipeMethod::Regular;
    m_keyFile = "123";
    m_wipeProgram = "123";
    load();
}

void Options::updateKeys() {
    key.resize(CryptoPP::AES::MAX_KEYLENGTH);
    iv.resize(CryptoPP::AES::BLOCKSIZE);

    memcpy(key.BytePtr(), CryptoPPUtils::HexDecodeString(keyStr).BytePtr(), CryptoPP::AES::MAX_KEYLENGTH);
    memcpy(iv.BytePtr(), CryptoPPUtils::HexDecodeString(ivStr).BytePtr(), CryptoPP::AES::BLOCKSIZE);

    //memset(key.BytePtr(), '0', keyStr.size());
    //memset(iv.BytePtr(), '0', ivStr.size());
}

void Options::load() {
    try {
        updateKeys();

        QSettings s(iniName.c_str(), QSettings::IniFormat);
        s.beginGroup("General");
        fromString(m_keyStorage, s.value("KeyStorage", "").toString().toStdString());

        std::string keyFile = s.value("KeyFile", "").toString().toStdString();

        m_keyFile = decryptString(keyFile);

        fromString(m_wipeMethod, s.value("WipeMethod", "").toString().toStdString());
        std::string wipeProgram = s.value("WipeProgram", "").toString().toStdString();
        m_wipeProgram = decryptString(wipeProgram);
    
        fromString(m_decryptionPlace, s.value("DecryptionPlace", "").toString().toStdString());
        std::string decryptionFolder = s.value("DecryptionFolder", "").toString().toStdString();
    
        m_decryptionFolder = decryptString(decryptionFolder);
    } catch( CryptoPP::Exception& e ) {
        //exit(1);
        //exit(1);
    }
}



std::string Options::encryptString(const std::string& src) {
    CryptoPP::AES::Encryption aesEncryption(key, key.size());
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    std::string ciphertext;
    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
    stfEncryptor.Put(reinterpret_cast<const unsigned char*>(src.c_str()), src.length() + 1);
    stfEncryptor.MessageEnd();

    return CryptoPPUtils::HexEncodeString(ciphertext);
}
std::string Options::decryptString(const std::string& src) {
    using namespace CryptoPP;

    std::string srcDecoded;
    SecByteBlock tmp = CryptoPPUtils::HexDecodeString(src);
    /*
    srcDecoded.resize(tmp.size());
    for (std::size_t i = 0; i < tmp.size(); ++i)
        srcDecoded[i] = tmp[i];
    */
    srcDecoded.assign((char*)tmp.BytePtr(), tmp.size());

    CryptoPP::AES::Decryption aesDecryption(key, key.size());
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
    
    std::string decryptedtext;
    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
    stfDecryptor.Put(reinterpret_cast<const unsigned char*>(srcDecoded.c_str()), srcDecoded.size());
    stfDecryptor.MessageEnd();

    return decryptedtext;
}

void Options::save() {
    try {
        updateKeys();
        QSettings s(iniName.c_str(), QSettings::IniFormat);
        s.beginGroup("General");
        s.setValue("KeyStorage", toString(m_keyStorage).c_str());

        std::string keyFileEncrypted = encryptString(m_keyFile);

        s.setValue("KeyFile", keyFileEncrypted.c_str());
        s.setValue("WipeMethod", toString(m_wipeMethod).c_str());

        std::string wipeProgramEncrypted;

        wipeProgramEncrypted = encryptString(m_wipeProgram);
        s.setValue("WipeProgram", wipeProgramEncrypted.c_str());

        s.setValue("DecryptionPlace", toString(m_decryptionPlace).c_str());
        std::string decryptionFolderEncrypted;

        decryptionFolderEncrypted = encryptString(m_decryptionFolder);
        s.setValue("DecryptionFolder", decryptionFolderEncrypted.c_str());
    }
    catch (CryptoPP::Exception& e) {
        exit(1);
    }
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
