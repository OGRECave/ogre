#include "commentstream.h"
#include <cstdio>

namespace utillib {

class commentstreambuf : public std::streambuf {
    std::streambuf * const m_sbuf;
    char m_buf[512];
    const int m_start;
    const int m_end;
    bool m_comment;
public:
    commentstreambuf(std::streambuf *, char start_sb, char end_sb);

    int underflow();
};

commentstreambuf::commentstreambuf(std::streambuf *sb, char start_sb, char end_sb)
    :   m_sbuf(sb), m_start(static_cast<unsigned char>(start_sb)),
        m_end(static_cast<unsigned char>(end_sb)), m_comment(false)
{
    setg(m_buf, m_buf, m_buf);
}

int commentstreambuf::underflow()
{
    char *p = m_buf;
    int ch;
    while (p != m_buf + sizeof m_buf && (ch = m_sbuf->sgetc()) != EOF) {
        m_sbuf->sbumpc();

        if (ch == m_start) m_comment = true;
        else if (ch == m_end) m_comment = false;

        if (!m_comment)
            *p++ = ch;
    }

    if (p == m_buf)
        return EOF;

    setg(m_buf, m_buf, p);
    return *m_buf;
}

commentstream::commentstream(std::istream &s, char start_sb, char end_sb)
    :   std::istream(new commentstreambuf(s.rdbuf(), start_sb, end_sb))
{
}

commentstream::~commentstream()
{
    delete rdbuf();
}

} // namespace utillib
