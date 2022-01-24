package fvm.nativefns;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.List;

import fvm.NativeFunction;
import fvm.String;
import fvm.Symbol;
import fvm.Value;
import fvm.VirtualMachine;

public class ReadFile implements NativeFunction {

    private static final Symbol file = new Symbol("file");

    @Override
    public List<Symbol> parameters() {
        return Collections.singletonList(file);
    }

    @Override
    public void apply(VirtualMachine vm) {
        Value value = vm.lookup(file).orElseThrow(IllegalStateException::new);
        if (!(value instanceof String)) {
            throw new IllegalStateException("Expected file name parameter to be string");
        }
        String casted = (String) value;
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        byte[] buffer = new byte[4096];
        File file = new File(casted.value());
        try (FileInputStream fis = new FileInputStream(file)) {
            int bytesRead = -1;
            while ((bytesRead = fis.read(buffer)) != -1) {
                byteArrayOutputStream.write(buffer, 0, bytesRead);
            }
        } catch (IOException e) {
            throw new RuntimeException("Could not read file", e);
        }
        byte[] bytes = byteArrayOutputStream.toByteArray();
        String result = new String(new java.lang.String(bytes, StandardCharsets.UTF_8));
        vm.pushStack(result);
    }
}
