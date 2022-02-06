#ifndef BYTECODE_WRITER_HH__
#define BYTECODE_WRITER_HH__

#include <string>
#include <fstream>

#include "util/memory_semantic_macros.hh"
#include "util/debug.hh"

#include "bytecode.hh"

namespace bytecode {

class BytecodeWriter {
public:
    BytecodeWriter() = delete;

    ~BytecodeWriter() = delete;

    NOT_COPYABLE(BytecodeWriter);

    NOT_MOVEABLE(BytecodeWriter);

    static void Write(const bytecode::File & file, const std::string & dest) {
        
        std::ofstream output_file;

        DEBUGLN("Writing to " << dest);

        output_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        output_file.open(dest, std::ios::binary|std::ios::out|std::ios::trunc);

        writeU64(output_file, file.GetVersion());

        DEBUGLN("Writing module name");
        writeString(output_file, file.GetModuleName());

        DEBUGLN("Writing imports");
        writeStrings(output_file, file.GetImportNames());

        DEBUGLN("Writing exports");
        writeStrings(output_file, file.GetExportNames());

        DEBUGLN("Writing functions");
        std::uint64_t fn_count = file.GetFunctions().size();
        writeU64(output_file, fn_count);

        for (std::uint64_t i = 0; i < fn_count; i++) {
            DEBUGLN("Writing function " << i);

            const bytecode::Function& fn = file.GetFunctions().at(i);

            writeString(output_file, fn.GetName());
            writeU64(output_file, fn.GetArity());
            writeU64(output_file, fn.GetLocals());

            std::uint64_t bytecode_count = fn.GetBytecode().size();
            writeU64(output_file, bytecode_count);

            for (std::uint64_t j = 0; j < bytecode_count; j++) {
                DEBUGLN("Writing bytecode " << i << "." << j);

                const bytecode::Bytecode& bc = fn.GetBytecode().at(j);

                std::uint8_t bytecode = 0;

                struct Visitor : public bytecode::BytecodeVisitor {
                    std::uint8_t& bytecode;
                public:

                    Visitor(std::uint8_t& _bytecode)
                    : bytecode{_bytecode}
                    {}

                    #define ADD_ENTRY(val) \
                        void On##val(const bytecode::Bytecode&) override { \
                            DEBUGLN("Writing " << #val); \
                            bytecode = static_cast<std::uint8_t>(bytecode::BytecodeType::val); \
                        }
                    PER_BYTECODE_TYPE(ADD_ENTRY)
                    #undef ADD_ENTRY

                } visitor(bytecode);

                bc.Visit(visitor);

                DEBUGLN("Writing bytecode tag " << static_cast<int>(bytecode));

                // bytecode as int

                writeU8(output_file, bytecode);

                struct ArgVisitor : public bytecode::BytecodeArgVisitor {
                    const bytecode::Bytecode& bc;
                    std::ofstream& output_file;
                public:
                    ArgVisitor(const bytecode::Bytecode& _bc, std::ofstream& _output_file)
                    : bc{_bc}, output_file{_output_file}
                    {}

                    void OnUnsigned(const bytecode::BytecodeArg&) override {
                        DEBUGLN("Writing bytecode u64 arg");
                        writeU64(output_file, bc.GetUnsignedArg());
                    }

                    void OnNone(const bytecode::BytecodeArg&) override {
                        // intentionally empty
                        DEBUGLN("No arg for bytecode");
                    }

                } argVisitor(bc, output_file);

                bc.VisitArg(argVisitor);
            }
        }

        DEBUGLN("Writing constants")
        std::uint64_t constant_count = file.GetConstants().size();
        writeU64(output_file, constant_count);
        for (std::uint64_t i = 0; i < constant_count; i++) {
            DEBUGLN("Writing constant " << i);
            const bytecode::Constant& constant = file.GetConstants().at(i);
            writeConstant(output_file, constant);
        }

        output_file.flush();
        output_file.close();

        DEBUGLN("Written");
    }
private:
    static void writeStrings(std::ofstream& stream, const std::vector<std::string>& strs) {
        writeU64(stream, strs.size());
        for (std::uint64_t i = 0; i < strs.size(); i++) {
            writeString(stream, strs.at(i));
        }
    }

    static void writeConstant(std::ofstream& stream, const bytecode::Constant& constant) {

        struct Visitor : public bytecode::ConstantVisitor {
            std::ofstream& stream;

            Visitor(std::ofstream& _stream): stream{_stream} {}

            void OnInteger(const bytecode::Constant& constant) override {
                writeU8(stream, static_cast<std::uint8_t>(bytecode::ConstantType::Integer));
                writeU64(stream, constant.GetIntegerConstant());
            }

            void OnString(const bytecode::Constant& constant) override {
                writeU8(stream, static_cast<std::uint8_t>(bytecode::ConstantType::String));
                writeString(stream, constant.GetStringConstant());
            }

            void OnInvocation(const bytecode::Constant& constant) override {
                writeU8(stream, static_cast<std::uint8_t>(bytecode::ConstantType::Invocation));
                writeU64(stream, constant.GetInvocationConstant().GetModuleNameIndex());
                writeU64(stream, constant.GetInvocationConstant().GetFunctionNameIndex());
                writeU64(stream, constant.GetInvocationConstant().GetArgumentCount());
            }

        } visitor(stream);

        constant.Visit(visitor);
    }

    static void writeString(std::ofstream& output_file, const std::string& string) {
        DEBUGLN("Writing string '" << string << "' (" << string.size() << ")");
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

}

#endif // BYTECODE_WRITER_HH__
