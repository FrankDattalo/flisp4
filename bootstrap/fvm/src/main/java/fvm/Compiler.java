package fvm;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;

import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.ParseTreeWalker;
import org.apache.commons.io.IOUtils;

import fvm.antlr.LanguageBaseListener;
import fvm.antlr.LanguageLexer;
import fvm.antlr.LanguageParser;
import fvm.antlr.LanguageParser.FunctionDefinitionContext;
import fvm.antlr.LanguageParser.IntegerContext;
import fvm.antlr.LanguageParser.StringContext;
import fvm.antlr.LanguageParser.SymbolContext;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.NonNull;
import lombok.SneakyThrows;
import lombok.Value;

public class Compiler {

    public void compile(String inputFilePath, String outputFilePath) {
        System.out.println("Reading " + inputFilePath);
        String input = readToString(inputFilePath);
        System.out.println("Contents:");
        System.out.println(input);
        System.out.println("Compiling file...");
        CompiledFile compiledFile = compile(input);
        System.out.println("Compiled!");
        System.out.println("Writing to output " + outputFilePath);
        writeToFile(compiledFile, outputFilePath);
        System.out.println("Done!");
    }

    private CompiledFile compile(String input) {
        CharStream stream = CharStreams.fromString(input);
        LanguageLexer lexer = new LanguageLexer(stream);
        lexer.removeErrorListeners();
        lexer.addErrorListener(new ThrowingErrorListener());
        CommonTokenStream cts = new CommonTokenStream(lexer);
        LanguageParser parser = new LanguageParser(cts);
        parser.removeErrorListeners();
        parser.addErrorListener(new ThrowingErrorListener());
        ParseTree tree = parser.program();
        ParseTreeWalker walker = new ParseTreeWalker();
        Listener listener = new Listener();
        walker.walk(listener, tree);
        return listener.getFile();
    }

    private void writeToFile(CompiledFile compiledCode, String outputFilePath) {
        try (FileOutputStream out = new FileOutputStream(new File(outputFilePath));
             ) {
            // write out each function
            for (Function fn : compiledCode.getFunctions()) {
            }
            // write out integer constant
            for (Long integer : compiledCode.getIntegerConstants()) {

            }
            // write out each string constant
            for (String str : compiledCode.getStringConstants()) {
                
            }
        }
    }

    @SneakyThrows
    private String readToString(String inputFilePath) {
        try (FileInputStream is = new FileInputStream(new File(inputFilePath))) {
            return IOUtils.toString(is, StandardCharsets.UTF_8);
        } 
    }

    private static final class ThrowingErrorListener extends BaseErrorListener {
        @Override
        public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, int charPositionInLine,
                java.lang.String msg, RecognitionException e) {
                    throw new RuntimeException(String.format("Error on line %d column %d: %s", line, charPositionInLine, msg), e);
        }
    }

    private static enum BytecodeType {

        Halt("Halt", false),
        LoadInteger("LoadInteger", true),
        LoadString("LoadString", true),
        LoadTrue("LoadTrue", false),
        LoadFalse("LoadFalse", false);

        BytecodeType(String asString, boolean hasArg) {
            this.asString = asString;
            this.hasArg = hasArg;
        }

        private String asString;

        @Getter private boolean hasArg;

        @Override
        public String toString() {
            return asString;
        }
    }

    @Value
    private static class Bytecode {
        @NonNull @Getter private BytecodeType type;
        @Getter private long arg;

        public Bytecode(BytecodeType type) {
            this.type = type;
            this.arg = 0;
            check(false, type.isHasArg());
        }

        public Bytecode(BytecodeType type, long arg) {
            this.type = type;
            this.arg = arg;
            check(true, type.isHasArg());
        }

        private void check(boolean expected, boolean actual) {
            if (expected == actual) {
                return;
            }
            throw new IllegalAccessError(String.format("Bytecode arg mismatch, want %s got %s", expected, actual));
        }
    }

    @Data
    @Builder
    private static class Function {
        private long arity;
        private long locals;
        @NonNull private List<Bytecode> bytecode;
    }

    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    private static class CompiledFile {
        @NonNull @Builder.Default private List<Function> functions = new ArrayList();
        @NonNull @Builder.Default private List<Long> integerConstants = new ArrayList();
        @NonNull @Builder.Default private List<String> stringConstants = new ArrayList();
    }

    private static final class ConstantPool<T> {
        private Map<T, Long> pool = new HashMap<>();
        public Long value(T t) {
            if (pool.containsKey(t)) {
                return pool.get(t);
            } else {
                Long result = pool.size();
                pool.put(t, result);
                return result;
            }
        }
    }

    private static final class Listener extends LanguageBaseListener {

        @Getter
        private final CompiledFile file = new CompiledFile();

        @NonNull private final ConstantPool<Long> integers = new ConstantPool<>();
        @NonNull private final ConstantPool<Character> characters = new ConstantPool<>();
        @NonNull private final ConstantPool<String> symbols = new ConstantPool<>();
        @NonNull private final ConstantPool<String> strings = new ConstantPool<>();

        private Function fn;

        @Override
        public void enterFunctionDefinition(FunctionDefinitionContext ctx) {
            
        }

        @Override
        public void enterInteger(IntegerContext ctx) {
            String text = ctx.INTEGER().getSymbol().getText();
            emit(BytecodeType.LoadInteger, integers.pool(Long.valueOf(text)));
        }

        @Override
        public void enterString(StringContext ctx) {
            String text = ctx.STRING().getSymbol().getText();
            emit(BytecodeType.LoadString, strings.pool(text));
        }

        private void emit(BytecodeType bc, long arg) {

        }

        private void emit(BytecodeType bc) {

        }
    }

}
