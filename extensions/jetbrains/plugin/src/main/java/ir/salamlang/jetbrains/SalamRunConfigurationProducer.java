package ir.salamlang.jetbrains;

import com.intellij.execution.actions.ConfigurationContext;
import com.intellij.execution.actions.LazyRunConfigurationProducer;
import com.intellij.execution.configurations.ConfigurationFactory;
import com.intellij.openapi.util.Ref;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.psi.PsiElement;
import com.intellij.psi.PsiFile;

/** Offers "Run" for a Salam file from the editor, the project view and the gutter. */
public final class SalamRunConfigurationProducer
    extends LazyRunConfigurationProducer<SalamRunConfiguration> {
  @Override
  public ConfigurationFactory getConfigurationFactory() {
    return SalamRunConfigurationType.getInstance().getConfigurationFactories()[0];
  }

  private static VirtualFile salamFile(ConfigurationContext context) {
    PsiElement element = context.getPsiLocation();
    if (element == null) {
      return null;
    }
    PsiFile psi = element.getContainingFile();
    VirtualFile file = psi == null ? null : psi.getVirtualFile();
    if (file == null || !"salam".equalsIgnoreCase(file.getExtension())) {
      return null;
    }
    return file;
  }

  @Override
  protected boolean setupConfigurationFromContext(
      SalamRunConfiguration configuration,
      ConfigurationContext context,
      Ref<PsiElement> sourceElement) {
    VirtualFile file = salamFile(context);
    if (file == null) {
      return false;
    }
    configuration.setFile(file.getPath());
    configuration.setName(file.getNameWithoutExtension());
    return true;
  }

  @Override
  public boolean isConfigurationFromContext(
      SalamRunConfiguration configuration, ConfigurationContext context) {
    VirtualFile file = salamFile(context);
    return file != null && file.getPath().equals(configuration.getFile());
  }
}
