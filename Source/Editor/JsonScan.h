#pragma once
#include "Base/Types.h"

// 텍스트 위치(begin/end)를 보존하는 최소 JSON 스캐너. 에디터가 원본 서식을 유지한 채 숫자만 제자리 교체하는 용도.
struct JsonNode
{
    char           type  = 0;   // 'o' 객체, 'a' 배열, 's' 문자열, 'n' 숫자, 'l' 리터럴
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

    // 루트 노드 인덱스, 실패하면 -1
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
