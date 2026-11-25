package ir.salamlang.jetbrains;

import com.intellij.notification.NotificationGroupManager;
import com.intellij.notification.NotificationType;
import com.intellij.openapi.actionSystem.AnAction;
import com.intellij.openapi.actionSystem.AnActionEvent;
import com.intellij.openapi.fileChooser.FileChooser;
import com.intellij.openapi.fileChooser.FileChooserDescriptorFactory;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.terminal.ui.TerminalWidget;
import org.jetbrains.plugins.terminal.TerminalToolWindowManager;

/**
 * Asks for a folder and runs {@code salam init} there in a terminal tab, so the wizard's questions
 * (name, type, platforms, design, web mode) are answered in the terminal.
 */
public final class SalamNewProjectAction extends AnAction {
  @Override
  public void actionPerformed(AnActionEvent e) {
    Project project = e.getProject();
    if (project == null) {
      return;
    }
    VirtualFile folder =
        FileChooser.chooseFile(
            FileChooserDescriptorFactory.createSingleFolderDescriptor()
                .withTitle("Folder for the New Salam Project")
                .withDescription("Make a folder first if needed; salam init asks the rest in the terminal."),
            project,
            null);
    if (folder == null) {
      return;
    }
    String command = quote(SalamExecutable.find()) + " init";
    try {
      TerminalWidget widget =
          TerminalToolWindowManager.getInstance(project)
              .createShellWidget(folder.getPath(), "salam init", true, true);
      widget.sendCommandToExecute(command);
    } catch (Exception ex) {
      NotificationGroupManager.getInstance()
          .getNotificationGroup("Salam")
          .createNotification(
              "Could not open a terminal. Run this in " + folder.getPath() + ": " + command,
              NotificationType.WARNING)
          .notify(project);
    }
  }

  private static String quote(String path) {
    if (path.matches("[A-Za-z0-9_./:\\\\-]+")) {
      return path;
    }
    return "\"" + path.replace("\"", "\\\"") + "\"";
  }
}
