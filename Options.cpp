#include "Options.h"
#include <QSettings>

//#include "../cryptopp/aes.h"
#include "../cryptopp/modes.h"
#include "../cryptopp/hex.h"

const std::string keyStr = "12345678901234567890123456789012";
const std::string ivStr = "1234567890123456";

CryptoPP::SecByteBlock hexDecode(const std::string& s) {
    using namespace CryptoPP;
    SecByteBlock key;
    {
        HexDecoder decoder;

        decoder.Put((byte*)s.data(), s.size());
        decoder.MessageEnd();

        word64 size = decoder.MaxRetrievable();
        if(size && size <= SIZE_MAX) {
            key.resize(size);
            decoder.Get(key, key.size());
        }
    }
    return key;
}

Options::Options()
    : key(hexDecode(keyStr)), iv(hexDecode(ivStr)) {

    m_keyStorage = KeyStorage::Keyboard;
    m_wipeMethod = WipeMethod::Regular;
    m_keyFile = "123";
    m_wipeProgram = "123";

    load();

}
void Options::load() {
    QSettings s(iniName.c_str(), QSettings::IniFormat);
    s.beginGroup("General");

    fromString(m_keyStorage, s.value("KeyStorage", "").toString().toStdString());

    std::string keyFile = s.value("KeyFile", "").toString().toStdString();

    try {
        m_keyFile = decryptString(keyFile);
    } catch( CryptoPP::Exception& e ) {
        exit(1);
    }

    fromString(m_wipeMethod, s.value("WipeMethod", "").toString().toStdString());
    std::string wipeProgram = s.value("WipeProgram", "").toString().toStdString();
    try {
        m_wipeProgram = decryptString(wipeProgram);
    } catch( CryptoPP::Exception& e ) {
        exit(1);
    }
}

std::string Options::encryptString(const std::string& src) {
    return src;
    using namespace CryptoPP;
    CTR_Mode<AES>::Encryption e(key, key.size(), iv);

    std::string cipher;
    StringSource ss1(src, true, new StreamTransformationFilter(e, new StringSink(cipher)));

    std::string hexStr;
    StringSource ss2(cipher, true, new HexEncoder(new StringSink(hexStr)));
    return hexStr;
}
std::string Options::decryptString(const std::string& src) {
    return src;
    using namespace CryptoPP;
    CTR_Mode<AES>::Decryption e(key, key.size(), iv);

    std::string source = src, destination;
    HexDecoder decoder(new StringSink(destination));
    decoder.Put(reinterpret_cast<const byte*>(source.data()), source.size());

    std::string plain;
    StringSource ss1(destination, true, new StreamTransformationFilter(e, new StringSink(plain)));



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
    QSettings s(iniName.c_str(), QSettings::IniFormat);
    s.beginGroup("General");
    s.setValue("KeyStorage", toString(m_keyStorage).c_str());

    std::string keyFileEncrypted;
    try {
        keyFileEncrypted = encryptString(m_keyFile);
    } catch( CryptoPP::Exception& e ) {
        //cerr << e.what() << endl;
        exit(1);
    }
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

}

Options::KeyStorage Options::keyStorage() const {
    return m_keyStorage;
}
Options::WipeMethod Options::wipeMethod() const {
    return m_wipeMethod;
}

std::string Options::keyFile() const {
    return m_keyFile;
}
std::string Options::wipeProgram() const {
    return m_wipeProgram;
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

bool Options::validate() const {
    const KeyStorage k = keyStorage();
    const WipeMethod w = wipeMethod();
    return (
        (k == KeyStorage::Keyboard || (k == KeyStorage::File && keyFile().size() != 0))
        && (w == WipeMethod::Regular || (w == WipeMethod::External && wipeProgram().size() != 0)));
}
