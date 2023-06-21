#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <variant>
#include <ostream>
#include <string_view>
#include <algorithm>
#include <unordered_map>


constexpr size_t ce_HeapSize = 1 << 14;
constexpr size_t ce_StackSize = 1 << 10;

using namespace std::string_view_literals;

enum class PunctuationToken
{
    Mov, Comma, Semicolon
};
static constexpr std::array sce_Keywords = {
    "mov"sv, ","sv, ";"sv
};
static const std::unordered_map<std::string_view, PunctuationToken> sc_KeywordToTokens = {
    {"mov"sv, PunctuationToken::Mov},
    {","sv, PunctuationToken::Comma},
    {";"sv, PunctuationToken::Semicolon},
};
std::ostream& operator<<(std::ostream& out, PunctuationToken tok)
{
    out << "PunctuationToken(";
    switch (tok)
    {
        case PunctuationToken::Mov: out << "mov"; break;
        case PunctuationToken::Comma: out << ","; break;
        case PunctuationToken::Semicolon: out << ";"; break;
    }
    return out << ')';
}

struct IntegerToken
{
    friend std::ostream& operator<<(std::ostream& out, IntegerToken token);
    uint_fast64_t value;

    constexpr explicit IntegerToken(uint_fast64_t value) : value(value) {}
};
std::ostream& operator<<(std::ostream& out, IntegerToken token)
{
    return out << "IntegerToken(" << token.value << ')';
}

struct UnknownToken
{
    friend std::ostream& operator<<(std::ostream& out, UnknownToken token);
    char symbol;

    constexpr explicit UnknownToken(char symbol) : symbol(symbol) {}
};
std::ostream& operator<<(std::ostream& out, UnknownToken token)
{
    out << "UnknownToken(`";
    switch (token.symbol)
    {
    case '\n':
        out << "\\n";
        break;
    default:
        out << token.symbol;
        break;
    }
    return  out << "`)";
}

struct EOFToken
{
    friend std::ostream& operator<<(std::ostream& out, EOFToken token);
};
std::ostream& operator<<(std::ostream& out, EOFToken token)
{
    (void)token;
    return out << "EOFToken";
}

using Token = std::variant<PunctuationToken, IntegerToken, UnknownToken, EOFToken>;
std::ostream& operator<<(std::ostream& out, const Token& token)
{
    std::visit([&out](auto&& t){ out << t; }, token);
    return out;
}


struct Position
{
    uint_fast64_t line, col;

    constexpr Position() : line(1), col(1) {}
    constexpr explicit Position(uint_fast64_t column) : line(1), col(column) {}
    constexpr Position(uint_fast64_t line, uint_fast64_t column) : line(line), col(column) {}

    constexpr Position operator+(const Position& pos)
    {
        if (!pos.line)
            return Position(line + pos.line, pos.col + 1);

        return Position(line, col + pos.col);
    }
    constexpr Position& operator+=(const Position& pos)
    {
        if (pos.line)
        {
            line += pos.line;
            col = pos.col + 1;
        }
        else
            col += pos.col;

        return *this;
    }
};


class Lexer
{
private:
    Position m_Pos;
    uint64_t m_ReadingPos;
    const std::string_view m_ProgramText;
    std::vector<std::string_view> m_Keywords;
public:
    Lexer(const std::string& program) : m_Pos(), m_ReadingPos(0), m_ProgramText(program)
    {
        m_Keywords.reserve(sce_Keywords.size());
        std::copy(sce_Keywords.begin(), sce_Keywords.end(), std::back_inserter(m_Keywords));
        std::sort(
            m_Keywords.begin(),
            m_Keywords.end(),
            [](const std::string_view& s1, const std::string_view& s2) {
                return s1.length() != s2.length()
                     ? s1.length() > s2.length()
                     : s1.compare(s2);
            }
        );
    }

    void SkipWhitespace()
    {
        while (m_ReadingPos < m_ProgramText.length() && std::isspace(m_ProgramText[m_ReadingPos]))
        {
            m_Pos += m_ProgramText[m_ReadingPos] == '\n' ? Position(1, 0) : Position(1);
            m_ReadingPos++;
        }
    }

    Token ParseToken()
    {
        SkipWhitespace();
        const auto view = std::string_view(&m_ProgramText[m_ReadingPos], m_ProgramText.end());

        if (m_ReadingPos == m_ProgramText.length())
            return EOFToken();

        for (const auto keyword: m_Keywords)
            if (view.starts_with(keyword))
            {
                m_ReadingPos += keyword.length();
                m_Pos += Position(keyword.length());
                return sc_KeywordToTokens.at(keyword);
            }

        m_Pos += Position(1);

        return UnknownToken(m_ProgramText[m_ReadingPos++]);
    }
};


class Interpreter
{
private:
    std::array<uint8_t, ce_HeapSize> m_Heap;
    std::array<uint8_t, ce_StackSize> m_Stack;
    const std::string m_Program;
    Lexer m_Lexer;
public:
    Interpreter(const std::string& program) : m_Program(program), m_Lexer(program) {}
    ~Interpreter() {}

    void Interpret()
    {
        Token tok;
        while (std::get_if<EOFToken>(&(tok = m_Lexer.ParseToken())) == nullptr)
        {
            std::cout << tok << '\n';
        }
        std::cout << tok << std::endl;
    }
};


using namespace std::string_literals;

const std::string program = "mov 0, 69;\n"s;


int main() {
    Interpreter interpreter(program);

    interpreter.Interpret();

    return 0;
}