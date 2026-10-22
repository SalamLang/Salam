package ir.salamlang.jetbrains;

import com.intellij.execution.configurations.ConfigurationFactory;
import com.intellij.execution.configurations.ConfigurationTypeBase;
import com.intellij.execution.configurations.ConfigurationTypeUtil;
import com.intellij.execution.configurations.RunConfiguration;
import com.intellij.openapi.components.BaseState;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.IconLoader;
import com.intellij.openapi.util.NotNullLazyValue;

/** The "Salam" run configuration type. */
public final class SalamRunConfigurationType extends ConfigurationTypeBase {
  static final String ID = "SalamRunConfiguration";

  /** Registers the type and its one factory. */
  public SalamRunConfigurationType() {
    super(
        ID,
        "Salam",
        "Run a Salam program with salam run or salam exec",
        NotNullLazyValue.lazy(
            () -> IconLoader.getIcon("/icons/salam.svg", SalamRunConfigurationType.class)));
    addFactory(new Factory(this));
  }

  static SalamRunConfigurationType getInstance() {
    return ConfigurationTypeUtil.findConfigurationType(SalamRunConfigurationType.class);
  }

  /** Creates Salam run configurations. */
  static final class Factory extends ConfigurationFactory {
    Factory(SalamRunConfigurationType type) {
      super(type);
    }

    @Override
    public String getId() {
      return ID;
    }

    @Override
    public RunConfiguration createTemplateConfiguration(Project project) {
      return new SalamRunConfiguration(project, this, "Salam");
    }

    @Override
    public Class<? extends BaseState> getOptionsClass() {
      return SalamRunConfigurationOptions.class;
    }
  }
}
