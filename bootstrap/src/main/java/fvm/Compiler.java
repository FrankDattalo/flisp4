package fvm;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.lang.annotation.Native;
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
import fvm.antlr.LanguageParser.AssignStatementContext;
import fvm.antlr.LanguageParser.BooleanLiteralContext;
import fvm.antlr.LanguageParser.DefineStatementContext;
import fvm.antlr.LanguageParser.EmptyReturnStatementContext;
import fvm.antlr.LanguageParser.ExportContext;
import fvm.antlr.LanguageParser.ExpressionReturnStatementContext;
import fvm.antlr.LanguageParser.ExpressionStatementContext;
import fvm.antlr.LanguageParser.ExternalInvokeExpressionContext;
import fvm.antlr.LanguageParser.FunctionDefinitionContext;
import fvm.antlr.LanguageParser.Import_Context;
import fvm.antlr.LanguageParser.IntegerLiteralContext;
import fvm.antlr.LanguageParser.InternalInvokeExpressionContext;
import fvm.antlr.LanguageParser.ModuleContext;
import fvm.antlr.LanguageParser.NilLiteralContext;
import fvm.antlr.LanguageParser.OneArmedIfStatementContext;
import fvm.antlr.LanguageParser.ProgramContext;
import fvm.antlr.LanguageParser.StringLiteralContext;
import fvm.antlr.LanguageParser.ThrowStatementContext;
import fvm.antlr.LanguageParser.TryCatchStatementContext;
import fvm.antlr.LanguageParser.TwoArmedIfStatementContext;
import fvm.antlr.LanguageParser.VariableExpressionContext;
import fvm.antlr.LanguageParser.WhileStatementContext;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.Getter;
import lombok.NonNull;
import lombok.RequiredArgsConstructor;
import lombok.Setter;
import lombok.SneakyThrows;
import lombok.Value;

public class Compiler {

    public void compile(String inputFilePath, String outputFilePath) {
        System.out.println("Reading " + inputFilePath);
        String input = readToString(inputFilePath);
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
        Listener listener = new Listener();
        ParseTreeWalker.DEFAULT.walk(listener, tree);
        return listener.getFile();
    }

    @SneakyThrows
    private void write(OutputStream out, String fmt, Object... args) {
        IOUtils.write(String.format(fmt, args), out, StandardCharsets.UTF_8);
    }

    private void writeLn(OutputStream out, String fmt, Object... args) {
        write(out, fmt + "\n", args);
    }

    @SneakyThrows
    private void writeToFile(CompiledFile compiledCode, String outputFilePath) {
        try (FileOutputStream out = new FileOutputStream(new File(outputFilePath))) {
            // version
            writeLn(out, "@version 1");
            writeLn(out, "");

            writeLn(out, "@module %s", compiledCode.getModuleName());
            writeLn(out, "");

            for (String imprt : compiledCode.getImports()) {
                writeLn(out, "@import %s", imprt);
            }
            writeLn(out, "");

            for (String export : compiledCode.getExports()) {
                writeLn(out, "@export %s", export);
            }
            writeLn(out, "");

            // write out each function
            for (Function fn : compiledCode.getFunctions()) {
                writeLn(out, "; begin - %s", fn.getName());
                writeLn(out, "@function %s", fn.getName());
                writeLn(out, "@arity %s", fn.getArity());
                writeLn(out, "@locals %s", fn.getLocals());
                for (Bytecode bc : fn.getBytecode()) {
                    if (bc.getType().isHasArg()) {
                        writeLn(out, "    %s %s", bc.getType().toString(), bc.getArg());
                    } else {
                        writeLn(out, "    " + bc.getType().toString());
                    }
                }
                // writeLn(out, "@exceptiontable");
                // writeLn(out, "@endexceptiontable");
                writeLn(out, "@endfunction");
                writeLn(out, "; end - %s\n", fn.getName());
            }
            writeLn(out, "");
            writeLn(out, "; constants");
            // write out each string constant
            for (Constant<?> constant : compiledCode.getConstants()) {
                write(out, constant.asAssemblyString());
                writeLn(out, "");
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
        JumpIfFalse("JumpIfFalse", true),
        Jump("Jump", true),
        LoadNil("LoadNil", false),
        Return("Return", false),
        LoadLocal("LoadLocal", true),
        StoreLocal("StoreLocal", true),
        LoadInteger("LoadInteger", true),
        LoadString("LoadString", true),
        LoadTrue("LoadTrue", false),
        LoadFalse("LoadFalse", false),
        Invoke("Invoke", true),
        Throw("Throw", false),
        Pop("Pop", false);

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

    @Data
    private static class Bytecode {
        @NonNull @Getter private BytecodeType type;
        @Getter @Setter private long arg;

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
    @AllArgsConstructor
    private static class Function {
        @NonNull private Long arity;
        @NonNull private Long locals;
        @NonNull private String name;
        @NonNull private List<Bytecode> bytecode;
        @Getter @Setter private long index;
    }

    @Value
    @Builder
    @AllArgsConstructor
    private static class CompiledFile {
        @NonNull private List<Function> functions;
        @NonNull private List<Constant<?>> constants;
        @NonNull private List<String> imports;
        @NonNull private List<String> exports;
        @NonNull private String moduleName;
    }

    private static final class ConstantPool {
        private Map<Constant<?>, Long> pool = new HashMap<>();
        @Getter private List<Constant<?>> list = new ArrayList<>();

        public Long value(Constant<?> t) {
            if (pool.containsKey(t)) {
                return pool.get(t);
            } else {
                Long result = Long.valueOf(pool.size());
                pool.put(t, result);
                list.add(t);
                return result;
            }
        }
    }

    @RequiredArgsConstructor
    private static abstract class Constant<T> {
        @NonNull protected final T data;

        abstract public String asAssemblyString();

        @Override
        public int hashCode() {
            return data.hashCode();
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof Constant)) {
                return false;
            }
            Constant<?> other = (Constant<?>) obj;
            return data.equals(other.data);
        }
    }

    @Data
    @Builder
    private static final class Invocation {
        @NonNull private final Long moduleNameIndex;
        @NonNull private final Long methodNameIndex;
        @NonNull private final Long parameterCount;
    }

    private static final class InvocationConstant extends Constant<Invocation> {

        public InvocationConstant(@NonNull Invocation data) {
            super(data);
        }

        @Override
        public String asAssemblyString() {
            return String.format(
                "@invocation %s %s %s", 
                data.getModuleNameIndex(), 
                data.getMethodNameIndex(), 
                data.getParameterCount());
        }

    }

    private static final class StringConstant extends Constant<String> {
        public StringConstant(@NonNull String data) {
            super(data);
        }

        @Override
        public String asAssemblyString() {
            return String.format("@string %s %s", data.length(), data);
        }
    }

    private static final class IntegerConstant extends Constant<Long> {
        public IntegerConstant(@NonNull Long data) {
            super(data);
        }

        @Override
        public String asAssemblyString() {
            return String.format("@integer %s", data);
        }
    }

    @Value
    @Builder
    private static final class FunctionCall {
        @NonNull private final String calleeName;
        @NonNull private final Long callerArgs;
        @NonNull private final String callerName;
        @NonNull private final Long callerBytecodeNumber;
    }

    private static final class Listener extends LanguageBaseListener {

        private static final Map<String, Long> NATIVES = new HashMap<>();
        static {
            NATIVES.put("print", 1L);
            NATIVES.put("println", 1L);
        }

        @Getter
        private CompiledFile file;

        private String moduleName;
        private final List<String> imports = new ArrayList<>();
        private final List<String> exports = new ArrayList<>();

        private final ConstantPool constants = new ConstantPool();

        private long arity;
        private String functionName;
        private Map<String, Long> locals;
        private List<Bytecode> bytecode;

        private final Map<String, Function> functions = new HashMap<>();

        private final List<FunctionCall> functionsCallsNeedingResolution = new ArrayList<>();

        @Override
        public void enterModule(ModuleContext ctx) {
            moduleName = ctx.identifier().getText();
            if ("self".equals(moduleName)) {
                throw new IllegalStateException("self is a reserved module name");
            }
        }

        @Override
        public void enterExport(ExportContext ctx) {
            String exportName = ctx.identifier().getText();
            if (this.exports.contains(exportName)) {
                throw new IllegalStateException(String.format("Duplicate export: %s", exportName));
            }
            this.exports.add(exportName);
        }

        @Override
        public void enterImport_(Import_Context ctx) {
            String importName = ctx.identifier().getText();
            if (this.imports.contains(importName)) {
                throw new IllegalStateException(String.format("Duplicate import: %s", importName));
            }
            this.imports.add(importName);
        }

        @Override
        public void exitProgram(ProgramContext ctx) {
            // validate main fn exists
            if (!this.functions.containsKey("main")) {
                throw new IllegalStateException("No main function defined");
            }
            Function main = this.functions.get("main");
            if (main.getArity() != 0) {
                throw new IllegalStateException("Arity of main function should be 0");
            }
            
            // validate exports
            for (String export : exports) {
                if (!this.functions.containsKey(export)) {
                    throw new IllegalStateException("Undefined export: " + export);
                }
            }

            // serialize functions
            List<Function> serializedFunctions = new ArrayList<>();
            serializedFunctions.add(main);
            for (Function fn : this.functions.values()) {
                if (fn.getName().equals("main")) {
                    fn.setIndex(0);
                    continue;
                }
                serializedFunctions.add(fn);
            }
            for (int i = 0; i < serializedFunctions.size(); i++) {
                serializedFunctions.get(i).setIndex(i);
            }
            // validate all functions refer to known things
            for (FunctionCall call : functionsCallsNeedingResolution) {
                System.out.println(String.format(
                    "Compiling function call in %s at %s to %s", 
                    call.getCallerName(), call.getCallerBytecodeNumber(), call.getCalleeName()));
                String toResolve = call.getCalleeName();
                if (!this.functions.containsKey(toResolve)) {
                    throw new IllegalStateException(
                        String.format("Call to unbound function %s in %s", toResolve, call.getCallerName())
                    );
                }
                Function callee = this.functions.get(toResolve);
                if (call.getCallerArgs() != callee.getArity()) {
                    throw new IllegalStateException(
                        String.format(
                            "Arity mismatch in call to function %s in %s expected %s argument(s) but got %s argument(s)", 
                            toResolve, call.getCallerName(), callee.getArity(), call.getCallerArgs()
                        )
                    );
                }
                long fnIndex = callee.getIndex();
                this.functions.get(call.getCallerName()).getBytecode().get(call.getCallerBytecodeNumber().intValue()).setArg(fnIndex);
            }
            // copy over integer and strings
            this.file = CompiledFile.builder()
                .functions(serializedFunctions)
                .constants(this.constants.getList())
                .imports(imports)
                .exports(exports)
                .moduleName(moduleName)
                .build();
        }

        @Override
        public void enterFunctionDefinition(FunctionDefinitionContext ctx) {
            functionName = ctx.identifier().getText();
            arity = ctx.functionParameters().identifier().size();
            locals = new HashMap<>();
            bytecode = new ArrayList<>();
            ctx.functionParameters().identifier().forEach(ident -> {
                defineLocal(ident.getText());
            });
            if (functions.containsKey(functionName)) {
                throw new IllegalStateException(
                    String.format("Redefinition of function %s", functionName)
                );
            }
            System.out.println("Compiling " + functionName);
        }

        @Override
        public void exitFunctionDefinition(FunctionDefinitionContext ctx) {
            if (functionName.equals("main")) {
                emit(BytecodeType.Halt);
            } else {
                emit(BytecodeType.LoadNil);
                emit(BytecodeType.Return);
            }
            functions.put(
                functionName,
                Function.builder()
                    .name(functionName)
                    .arity(arity)
                    .locals(Long.valueOf(locals.size()))
                    .bytecode(bytecode)
                    .build()
            );
        }

        @Override
        public void enterDefineStatement(DefineStatementContext ctx) {
            String local = ctx.identifier().getText();
            if (isDefined(local)) {
                throw new IllegalStateException(
                    String.format("Redefinition of local %s in function %s", local, functionName)
                );
            }
        }

        @Override
        public void exitDefineStatement(DefineStatementContext ctx) {
            String local = ctx.identifier().getText();
            long localId = defineLocal(local);
            emit(BytecodeType.StoreLocal, localId);
        }

        @Override
        public void enterAssignStatement(AssignStatementContext ctx) {
            String local = ctx.identifier().getText();
            if (!isDefined(local)) {
                throw new IllegalStateException(
                    String.format("Undefined variable %s in function %s", local, functionName)
                );
            }
        }

        @Override
        public void enterWhileStatement(WhileStatementContext ctx) {
            long loopStart = nextBytecodePosition();
            ParseTreeWalker.DEFAULT.walk(this, ctx.expression());
            long jumpIfFalsePosition = emit(BytecodeType.JumpIfFalse, 0);
            ParseTreeWalker.DEFAULT.walk(this, ctx.statements());
            emit(BytecodeType.Jump, loopStart);
            updateBytecodeArg(jumpIfFalsePosition, nextBytecodePosition());
            // clear the children so the listener doesn't walk them after we have manually done so
            ctx.children.clear();
        }

        @Override
        public void exitThrowStatement(ThrowStatementContext ctx) {
            emit(BytecodeType.Throw);
        }

        @Override
        public void enterOneArmedIfStatement(OneArmedIfStatementContext ctx) {
            ParseTreeWalker.DEFAULT.walk(this, ctx.expression());
            long jumpIfFalsePosition = emit(BytecodeType.JumpIfFalse, 0);
            ParseTreeWalker.DEFAULT.walk(this, ctx.statements());
            updateBytecodeArg(jumpIfFalsePosition, nextBytecodePosition());
            // clear the children so the listener doesn't walk them after we have manually done so
            ctx.children.clear();
        }

        @Override
        public void enterTwoArmedIfStatement(TwoArmedIfStatementContext ctx) {
            ParseTreeWalker.DEFAULT.walk(this, ctx.expression());
            long jumpIfFalsePosition = emit(BytecodeType.JumpIfFalse, 0);
            ParseTreeWalker.DEFAULT.walk(this, ctx.statements().get(0)); // true branch
            long jumpPosition = emit(BytecodeType.Jump, 0); // jump over else
            updateBytecodeArg(jumpIfFalsePosition, nextBytecodePosition()); // jump to false
            ParseTreeWalker.DEFAULT.walk(this, ctx.statements().get(1)); // false branch
            updateBytecodeArg(jumpPosition, nextBytecodePosition());
            // clear the children so the listener doesn't walk them after we have manually done so
            ctx.children.clear();
        }

        @Override
        public void exitInternalInvokeExpression(InternalInvokeExpressionContext ctx) {
            long args = ctx.functionArguments().expression().size();
            System.out.println(String.format("Compiling internal call in %s", functionName));
            // internal call
            String fnName = ctx.identifier().getText();

            long index = constants.value(new InvocationConstant(Invocation.builder()
                .moduleNameIndex(constants.value(new StringConstant("self")))
                .methodNameIndex(constants.value(new StringConstant(fnName)))
                .parameterCount(args)
                .build()));

            emit(BytecodeType.Invoke, index);
        }

        @Override
        public void exitExternalInvokeExpression(ExternalInvokeExpressionContext ctx) {
            long args = ctx.functionArguments().expression().size();
            System.out.println(String.format("Compiling external call in %s", functionName));
            String module = ctx.identifier(0).getText();

            if (!imports.contains(module)) {
                throw new IllegalStateException("No import for module: " + module);
            }

            String fn = ctx.identifier(1).getText();

            long index = constants.value(new InvocationConstant(Invocation.builder()
                .moduleNameIndex(constants.value(new StringConstant(module)))
                .methodNameIndex(constants.value(new StringConstant(fn)))
                .parameterCount(args)
                .build()));

            emit(BytecodeType.Invoke, index);
        }

        @Override
        public void exitExpressionStatement(ExpressionStatementContext ctx) {
            emit(BytecodeType.Pop);
        }

        @Override
        public void exitAssignStatement(AssignStatementContext ctx) {
            String local = ctx.identifier().getText();
            long localId = lookupLocal(local);
            emit(BytecodeType.StoreLocal, localId);
        }

        @Override
        public void exitEmptyReturnStatement(EmptyReturnStatementContext ctx) {
            emit(BytecodeType.LoadNil);
            emit(BytecodeType.Return);
        }

        @Override
        public void exitExpressionReturnStatement(ExpressionReturnStatementContext ctx) {
            emit(BytecodeType.Return);
        }

        @Override
        public void enterVariableExpression(VariableExpressionContext ctx) {
            String local = ctx.identifier().getText();
            long localId = lookupLocal(local);
            emit(BytecodeType.LoadLocal, localId);
        }

        @Override
        public void enterNilLiteral(NilLiteralContext ctx) {
            emit(BytecodeType.LoadNil);
        }

        @Override
        public void enterIntegerLiteral(IntegerLiteralContext ctx) {
            String text = ctx.INTEGER().getText();
            long integer = Long.valueOf(text);
            IntegerConstant constant = new IntegerConstant(integer);
            Long index = constants.value(constant);
            emit(BytecodeType.LoadInteger, index);
        }

        @Override
        public void enterStringLiteral(StringLiteralContext ctx) {
            String text = ctx.STRING().getText();
            text = text.substring(1, text.length() - 1);
            StringConstant constant = new StringConstant(text);
            Long argument = constants.value(constant);
            emit(BytecodeType.LoadString, argument);
        }

        @Override
        public void enterBooleanLiteral(BooleanLiteralContext ctx) {
            boolean result = Boolean.valueOf(ctx.BOOLEAN().getText());
            BytecodeType bc = result ? BytecodeType.LoadTrue : BytecodeType.LoadFalse;
            emit(bc);
        }

        private long emit(BytecodeType bc, long arg) {
            return emit(new Bytecode(bc, arg));
        }

        private long emit(BytecodeType bc) {
            return emit(new Bytecode(bc));
        }

        private void updateBytecodeArg(long position, long value) {
            this.bytecode.get((int) position).setArg(value);
        }

        private long nextBytecodePosition() {
            return this.bytecode.size();
        }

        private long emit(Bytecode bc) {
            long position = this.bytecode.size();
            this.bytecode.add(bc);
            return position;
        }

        private long defineLocal(String identifier) {
            if (this.locals.containsKey(identifier)) {
                throw new IllegalStateException(
                    String.format("Duplicate identifier %s in function %s", identifier, functionName)
                );
            }
            long ret = this.locals.size();
            this.locals.put(identifier, ret);
            return ret;
        }

        private boolean isDefined(String local) {
            return this.locals.containsKey(local);
        }

        private Long lookupLocal(String identifier) {
            if (!this.locals.containsKey(identifier)) {
                throw new IllegalStateException(
                    String.format("Reference to undefined variable %s in function %s", identifier, functionName)
                );
            }
            return this.locals.get(identifier);
        }
    }

}
