package ir.salamlang.jetbrains;

import com.intellij.execution.actions.ConfigurationContext;
import com.intellij.execution.actions.LazyRunConfigurationProducer;
import com.intellij.execution.configurations.ConfigurationFactory;
import com.intellij.openapi.util.Ref;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.psi.PsiDirectory;
import com.intellij.psi.PsiElement;
import com.intellij.psi.PsiFile;

/**
 * Offers "Run" for a Salam file from the editor and the project view: a program runs, a {@code
 * _test.salam} file or a directory runs its tests.
 */
public final class SalamRunConfigurationProducer
    extends LazyRunConfigurationProducer<SalamRunConfiguration> {
  @Override
  public ConfigurationFactory getConfigurationFactory() {
    return SalamRunConfigurationType.getInstance().getConfigurationFactories()[0];
  }

  private static VirtualFile target(ConfigurationContext context) {
    PsiElement element = context.getPsiLocation();
    if (element == null) {
      return null;
    }
    if (element instanceof PsiDirectory dir) {
      return dir.getVirtualFile();
    }
    PsiFile psi = element.getContainingFile();
    VirtualFile file = psi == null ? null : psi.getVirtualFile();
    if (file == null || !"salam".equalsIgnoreCase(file.getExtension())) {
      return null;
    }
    return file;
  }

  static boolean isTestFile(VirtualFile file) {
    return !file.isDirectory() && file.getName().endsWith("_test.salam");
  }

  @Override
  protected boolean setupConfigurationFromContext(
      SalamRunConfiguration configuration,
      ConfigurationContext context,
      Ref<PsiElement> sourceElement) {
    VirtualFile file = target(context);
    if (file == null) {
      return false;
    }
    configuration.setFile(file.getPath());
    if (file.isDirectory() || isTestFile(file)) {
      configuration.setCommand("test");
      configuration.setName("Tests in " + file.getName());
    } else {
      configuration.setName(file.getNameWithoutExtension());
    }
    return true;
  }

  @Override
  public boolean isConfigurationFromContext(
      SalamRunConfiguration configuration, ConfigurationContext context) {
    VirtualFile file = target(context);
    return file != null
        && file.getPath().equals(configuration.getFile())
        && (configuration.getTestName() == null || configuration.getTestName().isBlank());
  }
}
