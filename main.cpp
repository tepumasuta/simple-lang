#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <variant>
#include <ostream>
#include <string_view>
#include <algorithm>
#include <unordered_map>
#include <functional>


constexpr size_t ce_HeapSize = 1 << 14;
constexpr size_t ce_StackSize = 1 << 10;

using namespace std::string_literals;
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
    int_fast64_t value;

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

using TokenValue = std::variant<PunctuationToken, IntegerToken, UnknownToken, EOFToken>;
std::ostream& operator<<(std::ostream& out, const TokenValue& token)
{
    std::visit([&out](auto&& t){ out << t; }, token);
    return out;
}


struct Position
{
    uint_fast64_t line, col;

    friend std::ostream& operator<<(std::ostream& out, const Position pos);

    constexpr Position() : line(1), col(1) {}
    constexpr explicit Position(uint_fast64_t column) : line(0), col(column) {}
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
std::ostream& operator<<(std::ostream& out, const Position pos)
{
    return out << "Position(line=" << pos.line << ", col=" << pos.col << ')';
}


struct Token
{
    Position pos;
    TokenValue value;

    friend std::ostream& operator<<(std::ostream& out, const Token token);
};
std::ostream& operator<<(std::ostream& out, const Token token)
{
    return out << "Token[" << token.pos.line << ':' << token.pos.col << "](" << token.value << ')';
}


class Lexer
{
private:
    Position m_Pos;
    uint64_t m_ReadingPos;
    const std::string& m_ProgramText;
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

    Token LexToken()
    {
        SkipWhitespace();
        const auto view = std::string_view(&m_ProgramText[m_ReadingPos], &m_ProgramText.back());

        if (m_ReadingPos == m_ProgramText.length())
            return Token(m_Pos, EOFToken());

        for (const auto keyword: m_Keywords)
            if (view.starts_with(keyword))
            {
                m_ReadingPos += keyword.length();
                const auto token = Token(m_Pos, sc_KeywordToTokens.at(keyword));
                m_Pos += Position(keyword.length());
                return token;
            }

        if (std::isdigit(m_ProgramText[m_ReadingPos]) || (m_ProgramText[m_ReadingPos] == '-' && m_ProgramText.length() - m_ReadingPos >= 2 && std::isdigit(m_ProgramText[m_ReadingPos + 1])))
        {
            uint_fast32_t begin = m_ReadingPos;
            while (std::isdigit(m_ProgramText[++m_ReadingPos]));
            const auto token = Token(m_Pos, IntegerToken(std::stol(&m_ProgramText[begin])));
            m_Pos += Position(m_ReadingPos - begin);
            return token;
        }

        const auto token = Token(m_Pos, UnknownToken(m_ProgramText[m_ReadingPos++]));
        m_Pos += Position(1);

        return token;
    }
};


struct MovInstruction
{
    friend std::ostream& operator<<(std::ostream& out, const MovInstruction mov);
    uint_fast64_t value, heapAddress;
};
std::ostream& operator<<(std::ostream& out, const MovInstruction mov)
{
    return out << "Mov(value=" << mov.value << ", addr=" << mov.heapAddress << ')';
}
using Instruction = std::variant<MovInstruction>;
std::ostream& operator<<(std::ostream& out, const Instruction& instruction)
{
    std::visit([&out](auto&& i) { out << i; }, instruction);
    return out;
}

class Interpreter
{
private:
    std::array<uint8_t, ce_HeapSize> m_Heap;
    std::array<uint8_t, ce_StackSize> m_Stack;
    const std::string& m_Program;
public:
    Interpreter(const std::string& program) : m_Program(program) {}
    ~Interpreter() {}

    void Interpret() {}
};


const std::string program = "mov 0, 69;\n"s;


int main() {
    Lexer lexer(program);

    Token tok;
    while (std::get_if<EOFToken>(&(tok = lexer.LexToken()).value) == nullptr)
    {
        std::cout << tok << '\n';
    }
    std::cout << tok << std::endl;

    return 0;
}