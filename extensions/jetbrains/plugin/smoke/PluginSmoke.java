import com.intellij.openapi.project.Project;
import com.redhat.devtools.lsp4ij.server.StreamConnectionProvider;
import ir.salamlang.jetbrains.SalamServerFactory;
import ir.salamlang.jetbrains.SalamTextMateBundles;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

/** Starts salam lsp through the plugin's classes and checks the reply, without an IDE. */
public final class PluginSmoke {
  private PluginSmoke() {}

  /**
   * Runs the smoke test.
   *
   * @param args unused
   * @throws Exception when the server cannot be started or read
   */
  public static void main(String[] args) throws Exception {
    Project project = null;
    StreamConnectionProvider conn = new SalamServerFactory().createConnectionProvider(project);
    conn.start();
    String body =
        "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\","
            + "\"params\":{\"processId\":null,\"rootUri\":null,\"capabilities\":{}}}";
    OutputStream out = conn.getOutputStream();
    out.write(
        ("Content-Length: " + body.getBytes(StandardCharsets.UTF_8).length + "\r\n\r\n" + body)
            .getBytes(StandardCharsets.UTF_8));
    out.flush();
    InputStream in = conn.getInputStream();
    StringBuilder head = new StringBuilder();
    while (!head.toString().endsWith("\r\n\r\n")) {
      head.append((char) in.read());
    }
    int n = Integer.parseInt(head.toString().replaceAll("(?s).*Content-Length: *(\\d+).*", "$1"));
    byte[] buf = in.readNBytes(n);
    String reply = new String(buf, StandardCharsets.UTF_8);
    System.out.println("server: " + (reply.contains("\"salam-lsp\"") ? "initialize ok" : reply));
    conn.stop();
    System.out.println("bundles: " + new SalamTextMateBundles().getBundles());
  }
}
