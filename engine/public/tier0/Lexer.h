#pragma once

#include <tier0/types.h>

enum TokenType
{
    TT_INTEGER,
    TT_FLOAT,
    TT_STRING,
    TT_NAME,
};

struct Token
{
    TokenType type;
    str_t strVal;
    int iVal;
    f64 fVal;
};

class CLexer
{
public:
    CLexer(const char* pText);

    bool GetToken(Token& tok);
private:
    bool ReadWhitespace();
private:
    const char* m_pText;
    const char* m_pCur;
    int m_iLine;
};