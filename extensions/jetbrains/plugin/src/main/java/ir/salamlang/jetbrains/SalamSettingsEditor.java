package ir.salamlang.jetbrains;

import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory;
import com.intellij.openapi.options.SettingsEditor;
import com.intellij.openapi.ui.ComboBox;
import com.intellij.openapi.ui.TextFieldWithBrowseButton;
import com.intellij.ui.components.JBTextField;
import com.intellij.util.ui.FormBuilder;
import javax.swing.JComponent;
import javax.swing.JPanel;

/** The form for a Salam run configuration. */
final class SalamSettingsEditor extends SettingsEditor<SalamRunConfiguration> {
  private final TextFieldWithBrowseButton file = new TextFieldWithBrowseButton();
  private final ComboBox<String> command = new ComboBox<>(SalamRunConfiguration.COMMANDS);
  private final ComboBox<String> platform = new ComboBox<>(platformChoices());
  private final JBTextField arguments = new JBTextField();
  private final JBTextField testName = new JBTextField();
  private final TextFieldWithBrowseButton workingDirectory = new TextFieldWithBrowseButton();
  private final JPanel panel;

  SalamSettingsEditor() {
    file.addBrowseFolderListener(
        "Salam File or Test Directory",
        null,
        null,
        FileChooserDescriptorFactory.createSingleFileOrFolderDescriptor());
    workingDirectory.addBrowseFolderListener(
        "Working Directory",
        null,
        null,
        FileChooserDescriptorFactory.createSingleFolderDescriptor());
    panel =
        FormBuilder.createFormBuilder()
            .addLabeledComponent("Salam file or directory:", file)
            .addLabeledComponent("Command:", command)
            .addLabeledComponent("Platform (run and build):", platform)
            .addLabeledComponent("Test name contains:", testName)
            .addLabeledComponent("Program arguments:", arguments)
            .addLabeledComponent("Working directory:", workingDirectory)
            .getPanel();
  }

  private static String[] platformChoices() {
    String[] all = new String[SalamProject.PLATFORMS.size() + 1];
    all[0] = "";
    for (int i = 0; i < SalamProject.PLATFORMS.size(); i++) {
      all[i + 1] = SalamProject.PLATFORMS.get(i);
    }
    return all;
  }

  @Override
  protected void resetEditorFrom(SalamRunConfiguration configuration) {
    file.setText(configuration.getFile());
    command.setSelectedItem(SalamRunConfiguration.normalCommand(configuration.getCommand()));
    platform.setSelectedItem(configuration.getPlatform() == null ? "" : configuration.getPlatform());
    testName.setText(configuration.getTestName());
    arguments.setText(configuration.getArguments());
    workingDirectory.setText(configuration.getWorkingDirectory());
  }

  @Override
  protected void applyEditorTo(SalamRunConfiguration configuration) {
    configuration.setFile(file.getText());
    Object selected = command.getSelectedItem();
    configuration.setCommand(selected == null ? "run" : selected.toString());
    Object chosen = platform.getSelectedItem();
    configuration.setPlatform(chosen == null ? "" : chosen.toString());
    configuration.setArguments(arguments.getText());
    configuration.setTestName(testName.getText());
    configuration.setWorkingDirectory(workingDirectory.getText());
  }

  @Override
  protected JComponent createEditor() {
    return panel;
  }
}
