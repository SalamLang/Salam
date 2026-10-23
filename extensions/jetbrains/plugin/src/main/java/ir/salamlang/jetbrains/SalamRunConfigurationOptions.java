package ir.salamlang.jetbrains;

import com.intellij.execution.configurations.RunConfigurationOptions;
import com.intellij.openapi.components.StoredProperty;

/** The settings a Salam run configuration stores. */
public final class SalamRunConfigurationOptions extends RunConfigurationOptions {
  private final StoredProperty<String> file = string("").provideDelegate(this, "file");
  private final StoredProperty<String> command = string("run").provideDelegate(this, "command");
  private final StoredProperty<String> arguments = string("").provideDelegate(this, "arguments");
  private final StoredProperty<String> workingDirectory =
      string("").provideDelegate(this, "workingDirectory");
  private final StoredProperty<String> testName = string("").provideDelegate(this, "testName");

  String getFile() {
    return file.getValue(this);
  }

  void setFile(String value) {
    file.setValue(this, value);
  }

  String getCommand() {
    return command.getValue(this);
  }

  void setCommand(String value) {
    command.setValue(this, value);
  }

  String getArguments() {
    return arguments.getValue(this);
  }

  void setArguments(String value) {
    arguments.setValue(this, value);
  }

  String getWorkingDirectory() {
    return workingDirectory.getValue(this);
  }

  void setWorkingDirectory(String value) {
    workingDirectory.setValue(this, value);
  }

  String getTestName() {
    return testName.getValue(this);
  }

  void setTestName(String value) {
    testName.setValue(this, value);
  }
}
