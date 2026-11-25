package ir.salamlang.jetbrains;

import com.intellij.execution.ProgramRunnerUtil;
import com.intellij.execution.RunManager;
import com.intellij.execution.RunnerAndConfigurationSettings;
import com.intellij.execution.executors.DefaultRunExecutor;
import com.intellij.openapi.actionSystem.ActionUpdateThread;
import com.intellij.openapi.actionSystem.AnAction;
import com.intellij.openapi.actionSystem.AnActionEvent;
import com.intellij.openapi.actionSystem.CommonDataKeys;
import com.intellij.openapi.fileEditor.FileDocumentManager;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.ui.popup.JBPopupFactory;
import com.intellij.openapi.vfs.VirtualFile;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Run, build or check the Salam project around the current file, on the platform {@code salam.toml}
 * lists first, or on one picked from a list. Each action reuses a run configuration, so the
 * result opens in the Run window like any other run.
 */
public abstract class SalamProjectAction extends AnAction {
  private final String command;
  private final boolean choose;

  SalamProjectAction(String command, boolean choose) {
    this.command = command;
    this.choose = choose;
  }

  /** Runs the project on its first platform. */
  public static final class Run extends SalamProjectAction {
    /** Registers the action. */
    public Run() {
      super("run", false);
    }
  }

  /** Runs the project on a platform picked from the list. */
  public static final class RunFor extends SalamProjectAction {
    /** Registers the action. */
    public RunFor() {
      super("run", true);
    }
  }

  /** Builds the project for its first platform. */
  public static final class Build extends SalamProjectAction {
    /** Registers the action. */
    public Build() {
      super("build", false);
    }
  }

  /** Builds the project for a platform picked from the list. */
  public static final class BuildFor extends SalamProjectAction {
    /** Registers the action. */
    public BuildFor() {
      super("build", true);
    }
  }

  /** Checks the project for errors without building it. */
  public static final class Check extends SalamProjectAction {
    /** Registers the action. */
    public Check() {
      super("check", false);
    }
  }

  @Override
  public ActionUpdateThread getActionUpdateThread() {
    return ActionUpdateThread.BGT;
  }

  @Override
  public void update(AnActionEvent e) {
    e.getPresentation().setEnabledAndVisible(e.getProject() != null && startFile(e) != null);
  }

  private static File startFile(AnActionEvent e) {
    VirtualFile file = e.getData(CommonDataKeys.VIRTUAL_FILE);
    if (file != null && file.isInLocalFileSystem()) {
      return new File(file.getPath());
    }
    Project project = e.getProject();
    String base = project == null ? null : project.getBasePath();
    return base == null ? null : new File(base);
  }

  @Override
  public void actionPerformed(AnActionEvent e) {
    Project project = e.getProject();
    File start = startFile(e);
    if (project == null || start == null) {
      return;
    }
    SalamProject found = SalamProject.find(start);
    File root = found == null ? (start.isDirectory() ? start : start.getParentFile()) : found.root;
    List<String> platforms = found == null ? List.of() : found.platforms;
    if (!choose || "check".equals(command)) {
      launch(project, root, found == null ? "" : found.firstPlatform());
      return;
    }
    List<String> options = new ArrayList<>(platforms);
    for (String p : SalamProject.PLATFORMS) {
      if (!options.contains(p)) {
        options.add(p);
      }
    }
    JBPopupFactory.getInstance()
        .createPopupChooserBuilder(options)
        .setTitle("Salam platform")
        .setItemChosenCallback(platform -> launch(project, root, platform))
        .createPopup()
        .showInBestPositionFor(e.getDataContext());
  }

  private void launch(Project project, File root, String platform) {
    FileDocumentManager.getInstance().saveAllDocuments();
    RunManager runs = RunManager.getInstance(project);
    String path = root.getPath();
    RunnerAndConfigurationSettings settings = null;
    for (RunnerAndConfigurationSettings s :
        runs.getConfigurationSettingsList(SalamRunConfigurationType.getInstance())) {
      if (s.getConfiguration() instanceof SalamRunConfiguration c
          && path.equals(c.getFile())
          && command.equals(c.getCommand())
          && platform.equals(c.getPlatform() == null ? "" : c.getPlatform())) {
        settings = s;
        break;
      }
    }
    if (settings == null) {
      String name =
          command + " " + root.getName() + (platform.isEmpty() ? "" : " (" + platform + ")");
      settings =
          runs.createConfiguration(
              name, SalamRunConfigurationType.getInstance().getConfigurationFactories()[0]);
      SalamRunConfiguration c = (SalamRunConfiguration) settings.getConfiguration();
      c.setFile(path);
      c.setCommand(command);
      c.setPlatform(platform);
      settings.setTemporary(true);
      runs.addConfiguration(settings);
    }
    runs.setSelectedConfiguration(settings);
    ProgramRunnerUtil.executeConfiguration(settings, DefaultRunExecutor.getRunExecutorInstance());
  }
}
