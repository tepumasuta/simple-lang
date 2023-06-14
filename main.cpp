#include <iostream>
#include <string>
#include <array>


constexpr size_t ce_HeapSize = 1 << 14;
constexpr size_t ce_StackSize = 1 << 10;


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

const std::string program = ""s;


int main() {
    Interpreter interpreter(program);

    interpreter.Interpret();

    return 0;
}