#include "CryptoppUtils.h"

namespace CryptoPPUtils {
    SecByteBlock HexDecodeString(const std::string& hexStr) {
        StringSource ss(hexStr, true, new HexDecoder);
        SecByteBlock result((size_t)ss.MaxRetrievable());
        ss.Get(result, result.size());
        return result;
    }

    std::string HexEncodeString(const std::string& src) {
        std::string res;
        StringSource ss(src, true, new HexEncoder(new StringSink(res)));

        return res;
    }
}
