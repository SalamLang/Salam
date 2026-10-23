package ir.salamlang.jetbrains;

import com.intellij.execution.Location;
import com.intellij.execution.PsiLocation;
import com.intellij.execution.testframework.sm.runner.SMTestLocator;
import com.intellij.openapi.fileEditor.OpenFileDescriptor;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.LocalFileSystem;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.psi.PsiFile;
import com.intellij.psi.PsiManager;
import com.intellij.psi.search.GlobalSearchScope;
import java.io.File;
import java.util.Collections;
import java.util.List;

/** Turns the {@code salam://file:line} hints of {@code salam test} into editor locations. */
final class SalamTestLocator implements SMTestLocator {
  static final SalamTestLocator INSTANCE = new SalamTestLocator();

  private SalamTestLocator() {}

  @Override
  @SuppressWarnings("rawtypes")
  public List<Location> getLocation(
      String protocol, String path, Project project, GlobalSearchScope scope) {
    if (!"salam".equals(protocol)) {
      return Collections.emptyList();
    }
    String file = path;
    int line = 0;
    int colon = path.lastIndexOf(':');
    if (colon > 0 && colon < path.length() - 1) {
      try {
        line = Integer.parseInt(path.substring(colon + 1)) - 1;
        file = path.substring(0, colon);
      } catch (NumberFormatException e) {
        line = 0;
      }
    }
    File f = new File(file);
    if (!f.isAbsolute() && project.getBasePath() != null) {
      f = new File(project.getBasePath(), file);
    }
    VirtualFile vf = LocalFileSystem.getInstance().findFileByIoFile(f);
    if (vf == null) {
      return Collections.emptyList();
    }
    PsiFile psi = PsiManager.getInstance(project).findFile(vf);
    if (psi == null) {
      return Collections.emptyList();
    }
    int target = Math.max(line, 0);
    return Collections.singletonList(
        new PsiLocation<PsiFile>(project, psi) {
          @Override
          public OpenFileDescriptor getOpenFileDescriptor() {
            return new OpenFileDescriptor(project, vf, target, 0);
          }
        });
  }
}
