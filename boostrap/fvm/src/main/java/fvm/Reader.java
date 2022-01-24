package fvm;

import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.ParseTreeWalker;

import fvm.antlr.LanguageBaseListener;
import fvm.antlr.LanguageLexer;
import fvm.antlr.LanguageParser;
import fvm.antlr.LanguageParser.IntegerContext;
import fvm.antlr.LanguageParser.ListContext;
import fvm.antlr.LanguageParser.StringContext;
import fvm.antlr.LanguageParser.SymbolContext;

public class Reader {
    public Value read(java.lang.String input) {
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
        return listener.context.result.head();
    }

    private static final class ThrowingErrorListener extends BaseErrorListener {
        @Override
        public void syntaxError(Recognizer<?, ?> recognizer, Object offendingSymbol, int line, int charPositionInLine,
                java.lang.String msg, RecognitionException e) {
                    throw new RuntimeException(msg, e);
        }
    }

    private static final class Listener extends LanguageBaseListener {

        private static final class Context {
            private Context outer;
            private ListBuilder result = new ListBuilder();
        }

        private Context context = new Context();

        @Override
        public void enterInteger(IntegerContext ctx) {
            java.lang.String text = ctx.getText();
            long parsed = Long.parseLong(text);
            context.result.append(new Integer(parsed));
        }

        @Override
        public void enterSymbol(SymbolContext ctx) {
            java.lang.String text = ctx.getText();
            context.result.append(new Symbol(text));
        }

        @Override
        public void enterString(StringContext ctx) {
            java.lang.String text = ctx.getText();
            context.result.append(new String(text));
        }

        @Override
        public void enterList(ListContext ctx) {
            Context inner = new Context();
            inner.outer = context;
            context = inner;
        }

        @Override
        public void exitList(ListContext ctx) {
            Context inner = context;
            context = inner.outer;
            context.result.append(inner.result.head());
        }
    }

}
