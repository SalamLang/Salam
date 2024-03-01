package ir.salamlang.jetbrains;

import com.intellij.execution.DefaultExecutionResult;
import com.intellij.execution.ExecutionException;
import com.intellij.execution.ExecutionResult;
import com.intellij.execution.Executor;
import com.intellij.execution.configurations.CommandLineState;
import com.intellij.execution.configurations.ConfigurationFactory;
import com.intellij.execution.configurations.GeneralCommandLine;
import com.intellij.execution.configurations.ParametersList;
import com.intellij.execution.configurations.RunConfiguration;
import com.intellij.execution.configurations.RunConfigurationBase;
import com.intellij.execution.configurations.RunProfileState;
import com.intellij.execution.configurations.RuntimeConfigurationError;
import com.intellij.execution.configurations.RuntimeConfigurationException;
import com.intellij.execution.process.KillableColoredProcessHandler;
import com.intellij.execution.process.ProcessHandler;
import com.intellij.execution.process.ProcessTerminatedListener;
import com.intellij.execution.runners.ExecutionEnvironment;
import com.intellij.execution.runners.ProgramRunner;
import com.intellij.execution.testframework.sm.SMTestRunnerConnectionUtil;
import com.intellij.execution.testframework.sm.runner.SMTRunnerConsoleProperties;
import com.intellij.execution.testframework.sm.runner.SMTestLocator;
import com.intellij.execution.ui.ConsoleView;
import com.intellij.openapi.options.SettingsEditor;
import com.intellij.openapi.project.Project;
import java.io.File;
import java.nio.charset.StandardCharsets;

/**
 * Runs a Salam file with {@code salam run} or {@code salam exec}, or its tests with {@code salam
 * test}, whose results fill the IDE's test tree.
 */
public final class SalamRunConfiguration
    extends RunConfigurationBase<SalamRunConfigurationOptions> {
  private static final long serialVersionUID = 1L;

  SalamRunConfiguration(Project project, ConfigurationFactory factory, String name) {
    super(project, factory, name);
  }

  @Override
  protected SalamRunConfigurationOptions getOptions() {
    return (SalamRunConfigurationOptions) super.getOptions();
  }

  String getFile() {
    return getOptions().getFile();
  }

  void setFile(String value) {
    getOptions().setFile(value);
  }

  String getCommand() {
    return getOptions().getCommand();
  }

  void setCommand(String value) {
    getOptions().setCommand(value);
  }

  String getArguments() {
    return getOptions().getArguments();
  }

  void setArguments(String value) {
    getOptions().setArguments(value);
  }

  String getWorkingDirectory() {
    return getOptions().getWorkingDirectory();
  }

  void setWorkingDirectory(String value) {
    getOptions().setWorkingDirectory(value);
  }

  String getTestName() {
    return getOptions().getTestName();
  }

  void setTestName(String value) {
    getOptions().setTestName(value);
  }

  static String normalCommand(String command) {
    return "exec".equals(command) || "test".equals(command) ? command : "run";
  }

  boolean isTest() {
    return "test".equals(getCommand());
  }

  @Override
  public SettingsEditor<? extends RunConfiguration> getConfigurationEditor() {
    return new SalamSettingsEditor();
  }

  @Override
  public void checkConfiguration() throws RuntimeConfigurationException {
    String file = getFile();
    if (file == null || file.isBlank()) {
      throw new RuntimeConfigurationError("Choose the Salam file or directory to run.");
    }
    if (!new File(file).exists()) {
      throw new RuntimeConfigurationError("No such file: " + file);
    }
  }

  GeneralCommandLine commandLine() {
    String command = normalCommand(getCommand());
    GeneralCommandLine line = new GeneralCommandLine(SalamExecutable.find(), command, getFile());
    if ("test".equals(command)) {
      line.addParameter("--format=teamcity");
      String test = getTestName();
      if (test != null && !test.isBlank()) {
        line.addParameter("--run=" + test.trim());
      }
    }
    String args = getArguments();
    if (args != null && !args.isBlank() && !"test".equals(command)) {
      if ("run".equals(command)) {
        line.addParameter("--");
      }
      line.addParameters(ParametersList.parse(args));
    }
    String dir = getWorkingDirectory();
    if (dir == null || dir.isBlank()) {
      File target = new File(getFile());
      File parent = target.isDirectory() ? target : target.getParentFile();
      dir = parent == null ? getProject().getBasePath() : parent.getPath();
    }
    if (dir != null) {
      line.setWorkDirectory(dir);
    }
    line.setCharset(StandardCharsets.UTF_8);
    return line;
  }

  @Override
  public RunProfileState getState(Executor executor, ExecutionEnvironment environment) {
    return new CommandLineState(environment) {
      @Override
      protected ProcessHandler startProcess() throws ExecutionException {
        KillableColoredProcessHandler handler = new KillableColoredProcessHandler(commandLine());
        ProcessTerminatedListener.attach(handler);
        return handler;
      }

      @Override
      public ExecutionResult execute(Executor exec, ProgramRunner<?> runner)
          throws ExecutionException {
        if (!isTest()) {
          return super.execute(exec, runner);
        }
        ProcessHandler handler = startProcess();
        SMTRunnerConsoleProperties properties =
            new SMTRunnerConsoleProperties(SalamRunConfiguration.this, "Salam", exec) {
              @Override
              public SMTestLocator getTestLocator() {
                return SalamTestLocator.INSTANCE;
              }
            };
        ConsoleView console =
            SMTestRunnerConnectionUtil.createAndAttachConsole("Salam", handler, properties);
        return new DefaultExecutionResult(console, handler);
      }
    };
  }
}
