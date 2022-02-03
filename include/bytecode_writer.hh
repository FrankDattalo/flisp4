#ifndef BYTECODE_WRITER_HH__
#define BYTECODE_WRITER_HH__

#include <string>
#include <fstream>

#include "memory_semantic_macros.hh"
#include "bytecode.hh"
#include "debug.hh"

class BytecodeWriter {
public:
    BytecodeWriter() = delete;

    ~BytecodeWriter() = delete;

    NOT_COPYABLE(BytecodeWriter);

    NOT_MOVEABLE(BytecodeWriter);

    static void Write(const File & file, const std::string & dest) {
        
        std::ofstream output_file;

        DEBUGLN("Writing to " << dest);

        output_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        output_file.open(dest, std::ios::binary|std::ios::out|std::ios::trunc);

        writeU64(output_file, file.GetVersion());

        DEBUGLN("Writing functions");
        std::uint64_t fn_count = file.GetFunctions().size();
        writeU64(output_file, fn_count);

        for (std::uint64_t i = 0; i < fn_count; i++) {
            DEBUGLN("Writing function " << i);

            const Function& fn = file.GetFunctions().at(i);

            writeU64(output_file, fn.GetArity());
            writeU64(output_file, fn.GetLocals());

            std::uint64_t bytecode_count = fn.GetBytecode().size();
            writeU64(output_file, bytecode_count);

            for (std::uint64_t j = 0; j < bytecode_count; j++) {
                DEBUGLN("Writing bytecode " << i << "." << j);

                const Bytecode& bc = fn.GetBytecode().at(j);

                std::uint8_t bytecode = 0;

                struct Visitor : public BytecodeVisitor {
                    std::uint8_t& bytecode;
                public:

                    Visitor(std::uint8_t& _bytecode)
                    : bytecode{_bytecode}
                    {}

                    #define ADD_ENTRY(val) void On##val(const Bytecode&) { bytecode = static_cast<std::uint8_t>(BytecodeType::val); }
                    PER_BYTECODE_TYPE(ADD_ENTRY)
                    #undef ADD_ENTRY

                } visitor(bytecode);

                bc.Visit(visitor);

                // bytecode as int

                writeU8(output_file, bytecode);

                struct ArgVisitor : public BytecodeArgTypeVisitor {
                    const Bytecode& bc;
                    std::ofstream& output_file;
                public:
                    ArgVisitor(const Bytecode& _bc, std::ofstream& _output_file)
                    : bc{_bc}, output_file{_output_file}
                    {}

                    void OnUnsigned(BytecodeArgType) override {
                        writeU64(output_file, bc.GetUnsignedArg());
                    }

                    void OnNone(BytecodeArgType) override {
                        // intentionally empty
                    }

                } argVisitor(bc, output_file);
            }
        }

        DEBUGLN("Writing constants")
        std::uint64_t constant_count = file.GetConstants().size();
        writeU64(output_file, constant_count);
        for (std::uint64_t i = 0; i < constant_count; i++) {
            DEBUGLN("Writing constant " << i);
            const Constant& constant = file.GetConstants().at(i);
            writeConstant(output_file, constant);
            // writeU64(output_file, static_cast<std::uint64_t>(string.size()));
            // for (std::uint64_t j = 0; j < string.size(); j++) {
            //     DEBUGLN("Writing string char " << i << "." << j);
            //     writeChar(output_file, string.at(j));
            // }
        }

        output_file.flush();
        output_file.close();

        DEBUGLN("Written");
    }
private:
    static void writeConstant(std::ofstream& stream, const Constant& constant) {

        struct Visitor : public ConstantVisitor {
            std::ofstream& stream;

            Visitor(std::ofstream& _stream): stream{_stream} {}

            void OnInteger(const Constant& constant) override {
                writeU8(stream, static_cast<std::uint8_t>(ConstantType::Integer));
                writeU64(stream, constant.GetIntegerConstant());
            }

            void OnString(const Constant& constant) override {
                writeU8(stream, static_cast<std::uint8_t>(ConstantType::String));
                writeString(stream, constant.GetStringConstant());
            }
        } visitor(stream);

        constant.Visit(visitor);
    }

    static void writeString(std::ofstream& output_file, const std::string& string) {
        writeU64(output_file, static_cast<std::uint64_t>(string.size()));
        for (std::uint64_t j = 0; j < string.size(); j++) {
            DEBUGLN("Writing string char " << j);
            writeChar(output_file, string.at(j));
        }
    }

    static void writeU64(std::ofstream& stream, std::uint64_t val) {
        stream.write((const char*) &val, sizeof(std::uint64_t));
    }

    static void writeI64(std::ofstream& stream, std::int64_t val) {
        stream.write((const char*) &val, sizeof(std::int64_t));
    }

    static void writeChar(std::ofstream& stream, char val) {
        stream.write(&val, 1);
    }

    static void writeU8(std::ofstream& stream, std::uint8_t val) {
        stream.write((const char*) &val, sizeof(std::uint8_t));
    }
};

#endif // BYTECODE_WRITER_HH__
