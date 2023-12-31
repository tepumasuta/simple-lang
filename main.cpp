#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <variant>
#include <ostream>
#include <string_view>
#include <algorithm>
#include <unordered_map>
#include <optional>
#include <iomanip>
#include <fstream>


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

    std::vector<Token> LexTokens()
    {
        std::vector<Token> tokens;
        Token tok;
        while (std::get_if<EOFToken>(&(tok = LexToken()).value) == nullptr)
        {
            tokens.emplace_back(tok);
        }
        tokens.emplace_back(tok);
        return tokens;
    }
};

struct Instruction
{
    virtual std::ostream& _dump(std::ostream& out) const
    {
        return out << "Instruction()";
    }
    friend std::ostream& operator<<(std::ostream& out, const Instruction& instruction);

    virtual ~Instruction() {}
};
std::ostream& operator<<(std::ostream& out, const Instruction& instruction)
{
    return instruction._dump(out);
}


struct MovInstruction : public Instruction
{
    uint8_t value;
    uint_fast64_t heapAddress;

    MovInstruction(uint8_t value, uint_fast64_t heapAddress) : value(value), heapAddress(heapAddress) {}

    virtual std::ostream& _dump(std::ostream& out) const override
    {
        return out << "Mov(value=" << static_cast<uint16_t>(value) << ", addr=" << heapAddress << ')';
    }
    friend std::ostream& operator<<(std::ostream& out, const MovInstruction mov);
};
std::ostream& operator<<(std::ostream& out, const MovInstruction mov)
{
    return mov._dump(out);
}

struct ScopeInstruction : public Instruction
{
    std::vector<Instruction*> instructions;

    virtual std::ostream& _dump(std::ostream& out) const override
    {
        out << "Scope([";
        if (instructions.size())
        {
            auto start = instructions.begin();
            out << **start++;
            for (; start != instructions.end(); start++)
                out << ", " << **start;
        }
    return out << "])";
    }
    friend std::ostream& operator<<(std::ostream& out, const ScopeInstruction& scope);
    virtual ~ScopeInstruction()
    {
        for (const auto& instruction: instructions)
            delete instruction;
    }
};
std::ostream& operator<<(std::ostream& out, const ScopeInstruction& scope)
{
    return scope._dump(out);
}


struct AST
{
    Instruction* root;

    friend std::ostream& operator<<(std::ostream& out, const AST& tree);

    AST() : root(new ScopeInstruction()) {}
    virtual ~AST() { delete root; }
};
std::ostream& operator<<(std::ostream& out, const AST& tree)
{
    out << "AST(";

    ScopeInstruction* node = dynamic_cast<ScopeInstruction*>(tree.root);
    if (node)
    {
        out << *node;
    }

    return out << ')';
}


class Parser
{
private:
    const std::vector<Token>& m_Program;
    uint_fast64_t m_ReadingPos;

public:
    Parser(const std::vector<Token>& tokens) : m_Program(tokens), m_ReadingPos(0) {}

    Instruction* ParseInstruction()
    {
        if (LeftAtLeast(5)
            && FollowingTokenOfType<PunctuationToken>(0) && FollowingTokenIs<PunctuationToken>(0, PunctuationToken::Mov)
            && FollowingTokenOfType<IntegerToken>(1)
            && FollowingTokenOfType<PunctuationToken>(2) && FollowingTokenIs<PunctuationToken>(2, PunctuationToken::Comma)
            && FollowingTokenOfType<IntegerToken>(3)
            && FollowingTokenOfType<PunctuationToken>(4) && FollowingTokenIs<PunctuationToken>(4, PunctuationToken::Semicolon))
        {
            const auto node = new MovInstruction(
                std::get<IntegerToken>(m_Program[m_ReadingPos + 1].value).value,
                std::get<IntegerToken>(m_Program[m_ReadingPos + 3].value).value
            );
            m_ReadingPos += 5;
            return node;
        }

        throw std::runtime_error("Failed to parse program");
    };

    AST ParseProgram()
    {
        AST tree;

        while (m_ReadingPos < m_Program.size() && !FollowingTokenOfType<EOFToken>(0))
        {
            const auto& node = ParseInstruction();
            if (dynamic_cast<ScopeInstruction*>(node) == nullptr)
                dynamic_cast<ScopeInstruction*>(tree.root)->instructions.push_back(node);
        }

        return tree;
    }

private:
    bool LeftAtLeast(uint_fast64_t length) const
    {
        return m_Program.size() - m_ReadingPos >= length;
    }
    template<typename T>
    bool FollowingTokenOfType(uint_fast64_t index) const
    {
        return std::holds_alternative<T>(m_Program.at(m_ReadingPos + index).value);
    }
    template<typename T>
    bool FollowingTokenIs(uint_fast64_t index, const T value) const
    {
        return std::get<T>(m_Program.at(m_ReadingPos + index).value) == value;
    }
};


class Interpreter
{
private:
    std::array<uint8_t, ce_HeapSize> m_Heap;
    std::array<uint8_t, ce_StackSize> m_Stack;
    const AST& m_Program;
public:
    Interpreter(const AST& program) : m_Program(program) {}
    ~Interpreter() {}

    void Interpret()
    {
        Interpret(m_Program.root);
    }
    void DumpHeap(uint_fast64_t limit = 16)
    {
        if (!limit)
            return;

        std::cout << std::hex << std::setfill('0');
        std::cout << std::hex << std::setw(2) << static_cast<int>(m_Heap[0]);
        for (uint_fast64_t i = 1; i < limit; i++)
            std::cout << ' ' << std::hex << std::setw(2) << static_cast<int>(m_Heap[i]);
        std::cout << std::endl;
    }
    void DumpHeap(uint_fast64_t from, uint_fast64_t to)
    {
        if (to < from)
            return;

        std::cout << std::hex << std::setfill('0');
        std::cout << std::hex << std::setw(2) << static_cast<int>(m_Heap[from]);
        for (uint_fast64_t i = from + 1; i < to; i++)
            std::cout << ' ' << std::hex << std::setw(2) << static_cast<int>(m_Heap[i]);
        std::cout << std::endl;
    }
private:
    void Interpret(Instruction* instruction)
    {
        if (ScopeInstruction* op = dynamic_cast<ScopeInstruction*>(instruction))
            for (const auto nextOp: op->instructions)
                Interpret(nextOp);
        if (MovInstruction* op = dynamic_cast<MovInstruction*>(instruction))
        {
            m_Heap[op->heapAddress] = op->value;
        }
    }
};


int main(int argc, char** argv) {
    if (argc != 2)
    {
        std::cout << "Interpreter expects a filepath to be read and interpreted." << std::endl;
        return -1;
    }

    const std::string filename(argv[1]);
    std::ifstream file(filename);
    std::string program((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Lexer lexer(program);
    const std::vector<Token>& tokens = lexer.LexTokens();
    Parser parser(tokens);
    const AST ast = parser.ParseProgram();
    Interpreter interpreter(ast);
    interpreter.Interpret();
    interpreter.DumpHeap(80);

    return 0;
}