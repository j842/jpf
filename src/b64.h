#ifndef __B64__H
#define __B64__H

#include <vector>
#include <string>
typedef unsigned char BYTE;

class Base64
{
public:
    static std::wstring encode(const std::vector<BYTE>& buf);
    static std::wstring encode(const BYTE* buf, unsigned int bufLen);
    static std::vector<BYTE> decode(std::wstring encoded_string);

    // for text:
    static std::wstring encodeT(const std::wstring & buf);
    static std::wstring decodeT(std::wstring encoded_string);
};

#endif
