#include <tier0/Lexer.h>
#include <tier0/log.h>

CLexer::CLexer(const char* pText)
{
    m_pCur = m_pText = pText;
    m_iLine = 0;
}

bool CLexer::ReadWhitespace()
{
    while (true)
    {
        while (*m_pCur <= ' ')
        {
            if (*m_pCur == '\n')
                m_iLine++;
            m_pCur++;
        }

        if (!*m_pCur)
            return false;

        if (*m_pCur == '/' && *(m_pCur+1) == '/') // Comment
        {
            while (*m_pCur && *m_pCur != '\n')
                m_pCur++;
            if (!*m_pCur)
                return false;
            m_iLine++;
            m_pCur++;
        }
        else if (*m_pCur == '/' && *(m_pCur+1) == '*')
        {
            while (*m_pCur && (*m_pCur != '*' || *(m_pCur+1) != '/'))
            {
                if (*m_pCur == '\n')
                    m_iLine++;
                m_pCur++;
            }
            
            if (!*m_pCur)
            {
                LOG_WARN("Encountered EOF inside comment while parsing on line {}!\n", m_iLine);
                return false;
            }

            m_pCur += 2;
        }
        else
            break;
    }

    return true;
}

bool CLexer::GetToken(Token &tok)
{
    if (!ReadWhitespace())
        return false;

    printf("%s\n", m_pCur);

    return true;
}