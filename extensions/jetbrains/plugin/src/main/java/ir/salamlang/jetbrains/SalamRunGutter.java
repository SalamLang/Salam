package ir.salamlang.jetbrains;

import com.intellij.execution.ProgramRunnerUtil;
import com.intellij.execution.RunManager;
import com.intellij.execution.RunnerAndConfigurationSettings;
import com.intellij.execution.executors.DefaultRunExecutor;
import com.intellij.icons.AllIcons;
import com.intellij.openapi.Disposable;
import com.intellij.openapi.actionSystem.AnAction;
import com.intellij.openapi.actionSystem.AnActionEvent;
import com.intellij.openapi.editor.Document;
import com.intellij.openapi.editor.Editor;
import com.intellij.openapi.editor.event.DocumentEvent;
import com.intellij.openapi.editor.event.DocumentListener;
import com.intellij.openapi.editor.event.EditorFactoryEvent;
import com.intellij.openapi.editor.event.EditorFactoryListener;
import com.intellij.openapi.editor.markup.GutterIconRenderer;
import com.intellij.openapi.editor.markup.HighlighterLayer;
import com.intellij.openapi.editor.markup.MarkupModel;
import com.intellij.openapi.editor.markup.RangeHighlighter;
import com.intellij.openapi.fileEditor.FileDocumentManager;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.Disposer;
import com.intellij.openapi.util.Key;
import com.intellij.openapi.util.TextRange;
import com.intellij.openapi.vfs.VirtualFile;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.Icon;

/**
 * Puts a Run button in the gutter beside {@code func main} and {@code تابع ریشه}, and in a {@code
 * _test.salam} file beside every test function.
 *
 * <p>TextMate files have no PSI below the file, so a run line marker contributor never sees the
 * line; the button is an editor highlighter instead, refreshed as the document changes.
 */
public final class SalamRunGutter implements EditorFactoryListener {
  private static final Pattern ENTRY =
      Pattern.compile(
          "^\\s*(pub\\s+)?(func|تابع)\\s+(main|ریشه)(?![\\w\\u200c])",
          Pattern.UNICODE_CHARACTER_CLASS);
  private static final Pattern TEST =
      Pattern.compile(
          "^\\s*(?:pub\\s+)?(?:func|تابع)\\s+(test_\\w*|آزمون[^(:]*?)"
              + "\\s*(?:\\(\\s*\\))?\\s*:\\s*(?://.*)?$",
          Pattern.UNICODE_CHARACTER_CLASS);
  private static final Key<Disposable> LIFETIME = Key.create("salam.run.gutter");
  private static final Key<List<RangeHighlighter>> MARKS = Key.create("salam.run.gutter.marks");

  @Override
  public void editorCreated(EditorFactoryEvent event) {
    Editor editor = event.getEditor();
    Project project = editor.getProject();
    VirtualFile file = FileDocumentManager.getInstance().getFile(editor.getDocument());
    if (project == null || file == null || !"salam".equalsIgnoreCase(file.getExtension())) {
      return;
    }
    Disposable lifetime = Disposer.newDisposable("Salam run gutter");
    editor.putUserData(LIFETIME, lifetime);
    editor
        .getDocument()
        .addDocumentListener(
            new DocumentListener() {
              @Override
              public void documentChanged(DocumentEvent e) {
                refresh(editor, project, file);
              }
            },
            lifetime);
    refresh(editor, project, file);
  }

  @Override
  public void editorReleased(EditorFactoryEvent event) {
    Disposable lifetime = event.getEditor().getUserData(LIFETIME);
    if (lifetime != null) {
      Disposer.dispose(lifetime);
      event.getEditor().putUserData(LIFETIME, null);
    }
  }

  private static void refresh(Editor editor, Project project, VirtualFile file) {
    if (editor.isDisposed()) {
      return;
    }
    MarkupModel markup = editor.getMarkupModel();
    List<RangeHighlighter> old = editor.getUserData(MARKS);
    if (old != null) {
      for (RangeHighlighter h : old) {
        markup.removeHighlighter(h);
      }
    }
    List<RangeHighlighter> marks = new ArrayList<>();
    Document document = editor.getDocument();
    boolean tests = SalamRunConfigurationProducer.isTestFile(file);
    for (int line = 0; line < document.getLineCount(); line++) {
      int start = document.getLineStartOffset(line);
      int end = document.getLineEndOffset(line);
      String text = document.getText(new TextRange(start, end));
      String test = null;
      if (tests) {
        Matcher m = TEST.matcher(text);
        if (!m.lookingAt()) {
          continue;
        }
        test = m.group(1).trim();
      } else if (!ENTRY.matcher(text).lookingAt()) {
        continue;
      }
      RangeHighlighter h =
          markup.addLineHighlighter(line, HighlighterLayer.ADDITIONAL_SYNTAX, null);
      h.setGutterIconRenderer(new RunIcon(project, file, test));
      marks.add(h);
    }
    editor.putUserData(MARKS, marks);
  }

  static void run(Project project, VirtualFile file, String test) {
    FileDocumentManager.getInstance().saveAllDocuments();
    String testName = test == null ? "" : "^" + test + "$";
    RunManager runs = RunManager.getInstance(project);
    RunnerAndConfigurationSettings settings = null;
    for (RunnerAndConfigurationSettings s :
        runs.getConfigurationSettingsList(SalamRunConfigurationType.getInstance())) {
      if (s.getConfiguration() instanceof SalamRunConfiguration c
          && file.getPath().equals(c.getFile())
          && testName.equals(c.getTestName() == null ? "" : c.getTestName())) {
        settings = s;
        break;
      }
    }
    if (settings == null) {
      String name = file.getNameWithoutExtension() + (test == null ? "" : ": " + test);
      settings =
          runs.createConfiguration(
              name, SalamRunConfigurationType.getInstance().getConfigurationFactories()[0]);
      SalamRunConfiguration c = (SalamRunConfiguration) settings.getConfiguration();
      c.setFile(file.getPath());
      if (SalamRunConfigurationProducer.isTestFile(file)) {
        c.setCommand("test");
        c.setTestName(testName);
      }
      settings.setTemporary(true);
      runs.addConfiguration(settings);
    }
    runs.setSelectedConfiguration(settings);
    ProgramRunnerUtil.executeConfiguration(settings, DefaultRunExecutor.getRunExecutorInstance());
  }

  /** The gutter button; equal for the same target so the IDE does not repaint it needlessly. */
  private static final class RunIcon extends GutterIconRenderer {
    private final Project project;
    private final VirtualFile file;
    private final String test;

    RunIcon(Project project, VirtualFile file, String test) {
      this.project = project;
      this.file = file;
      this.test = test;
    }

    private String label() {
      return "Run " + (test == null ? file.getName() : test);
    }

    @Override
    public Icon getIcon() {
      return AllIcons.RunConfigurations.TestState.Run;
    }

    @Override
    public String getTooltipText() {
      return label();
    }

    @Override
    public AnAction getClickAction() {
      return new AnAction(label()) {
        @Override
        public void actionPerformed(AnActionEvent e) {
          run(project, file, test);
        }
      };
    }

    @Override
    public boolean isNavigateAction() {
      return true;
    }

    @Override
    public boolean equals(Object other) {
      return other instanceof RunIcon r
          && r.file.equals(file)
          && r.project.equals(project)
          && Objects.equals(r.test, test);
    }

    @Override
    public int hashCode() {
      return file.hashCode();
    }
  }
}
