#include <iostream>
#include <string>
#include <array>
#include <variant>
#include <ostream>
#include <string_view>


constexpr size_t ce_HeapSize = 1 << 14;
constexpr size_t ce_StackSize = 1 << 10;


enum class PunctuationToken
{
    Mov, Comma, Semicolon
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
    const std::string& m_ProgramText;
public:
    Lexer(const std::string& program) : m_Pos(), m_ReadingPos(0), m_ProgramText(program) {}

    Token ParseToken()
    {
        if (m_ReadingPos == m_ProgramText.length())
            return EOFToken();

        if (m_ProgramText[m_ReadingPos] == '\n')
            m_Pos += Position(1, 0);
        else
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