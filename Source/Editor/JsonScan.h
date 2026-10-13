#pragma once
#include "Base/Types.h"

// 텍스트 위치를 보존하는 최소 JSON 스캐너
struct JsonNode
{
    char           type  = 0;
    int            begin = 0;
    int            end   = 0;
    String         str   = {};
    Vector<String> keys  = {};
    Vector<int>    kids  = {};
};

class JsonScanner
{
public:
    explicit JsonScanner(StringView _text);

    int Parse();

    Vector<JsonNode> nodes = {};

private:
    void SkipWs_();
    bool ReadString_(String& _out);
    int  Value_(int _depth);

    StringView m_text  = {};
    size_t     m_pos   = 0;
    bool       m_bFail = false;
};
