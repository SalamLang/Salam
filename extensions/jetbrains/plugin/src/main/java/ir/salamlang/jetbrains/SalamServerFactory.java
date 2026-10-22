package ir.salamlang.jetbrains;

import com.intellij.openapi.project.Project;
import com.redhat.devtools.lsp4ij.LanguageServerFactory;
import com.redhat.devtools.lsp4ij.server.ProcessStreamConnectionProvider;
import com.redhat.devtools.lsp4ij.server.StreamConnectionProvider;
import java.util.List;

/** Starts {@code salam lsp} for LSP4IJ. */
public final class SalamServerFactory implements LanguageServerFactory {
  @Override
  public StreamConnectionProvider createConnectionProvider(Project project) {
    List<String> command = List.of(SalamExecutable.find(), "lsp");
    String dir = project == null ? null : project.getBasePath();
    return new ProcessStreamConnectionProvider(command, dir) {};
  }
}
