import com.intellij.openapi.project.Project;
import com.redhat.devtools.lsp4ij.server.StreamConnectionProvider;
import ir.salamlang.jetbrains.SalamServerFactory;
import ir.salamlang.jetbrains.SalamTextMateBundles;
import java.io.*;
import java.nio.charset.StandardCharsets;

public class PluginSmoke {
    public static void main(String[] args) throws Exception {
        Project project = null;
        StreamConnectionProvider conn = new SalamServerFactory().createConnectionProvider(project);
        conn.start();
        String body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{\"processId\":null,\"rootUri\":null,\"capabilities\":{}}}";
        OutputStream out = conn.getOutputStream();
        out.write(("Content-Length: " + body.getBytes(StandardCharsets.UTF_8).length + "\r\n\r\n" + body).getBytes(StandardCharsets.UTF_8));
        out.flush();
        InputStream in = conn.getInputStream();
        StringBuilder head = new StringBuilder();
        while (!head.toString().endsWith("\r\n\r\n")) head.append((char) in.read());
        int n = Integer.parseInt(head.toString().replaceAll("(?s).*Content-Length: *(\\d+).*", "$1"));
        byte[] buf = in.readNBytes(n);
        String reply = new String(buf, StandardCharsets.UTF_8);
        System.out.println("server: " + (reply.contains("\"salam-lsp\"") ? "initialize ok" : reply));
        conn.stop();
        System.out.println("bundles: " + new SalamTextMateBundles().getBundles());
    }
}
