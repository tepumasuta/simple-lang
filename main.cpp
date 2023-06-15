#include <iostream>
#include <string>
#include <array>
#include <variant>
#include <ostream>


constexpr size_t ce_HeapSize = 1 << 14;
constexpr size_t ce_StackSize = 1 << 10;


enum class PunctuationToken
{
    Mov, Comma, Semicolon
};

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
    return out << "UnknownToken(`" << token.symbol << "`)";
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


class Interpreter
{
private:
    std::array<uint8_t, ce_HeapSize> m_Heap;
    std::array<uint8_t, ce_StackSize> m_Stack;
    const std::string m_Program;
public:
    Interpreter(const std::string& program) : m_Program(program) {}
    ~Interpreter() {}

    void Interpret()
    {
        std::cout << "Hello from interpreter!\n"
                  << "The program received: \n`" << m_Program << '`' << std::endl;
    }
};


using namespace std::string_literals;

const std::string program = "mov 0, 69;\n"s;


int main() {
    Interpreter interpreter(program);

    interpreter.Interpret();

    return 0;
}