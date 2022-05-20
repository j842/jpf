#ifndef __B64__H
#define __B64__H

#include <vector>
#include <string>
typedef unsigned char BYTE;

class Base64
{
public:
    static std::string encode(const std::vector<BYTE>& buf);
    static std::string encode(const BYTE* buf, unsigned int bufLen);
    static std::vector<BYTE> decode(std::string encoded_string);

    // for text:
    static std::string encodeT(const std::string & buf);
    static std::string decodeT(std::string encoded_string);
};

#endif
