#ifndef CRYPTOPPUTILS_H
#define CRYPTOPPUTILS_H

#include "../cryptopp/secblock.h"
#include <string>

namespace CryptoPPUtils {
    using namespace CryptoPP;
    SecByteBlock HexDecodeString(const std::string& hexStr);
    std::string HexEncodeString(const std::string& src);
}

#endif // CRYPTOPPUTILS_H
