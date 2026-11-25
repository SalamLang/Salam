package ir.salamlang.jetbrains;

import com.intellij.openapi.Disposable;
import com.intellij.openapi.editor.Editor;
import com.intellij.openapi.editor.EditorFactory;
import com.intellij.openapi.editor.event.CaretEvent;
import com.intellij.openapi.editor.event.CaretListener;
import com.intellij.openapi.fileEditor.FileDocumentManager;
import com.intellij.openapi.fileEditor.FileEditorManager;
import com.intellij.openapi.fileEditor.FileEditorManagerEvent;
import com.intellij.openapi.fileEditor.FileEditorManagerListener;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.openapi.wm.StatusBar;
import com.intellij.openapi.wm.StatusBarWidget;
import com.intellij.openapi.wm.StatusBarWidgetFactory;
import com.intellij.util.messages.MessageBusConnection;
import java.awt.Component;
import java.awt.event.MouseEvent;
import java.util.List;
import com.intellij.util.Consumer;

/**
 * The status bar shows the blocks around the caret in a Salam file, for example {@code page > box >
 * button}. TextMate files have no syntax tree to build breadcrumbs from, so the chain is read from
 * the text by {@link SalamBlockChain}.
 */
public final class SalamBlockChainWidget implements StatusBarWidget, StatusBarWidget.TextPresentation {
  static final String ID = "salam.blockChain";

  private final Project project;
  private StatusBar statusBar;
  private MessageBusConnection connection;

  SalamBlockChainWidget(Project project) {
    this.project = project;
  }

  @Override
  public String ID() {
    return ID;
  }

  @Override
  public StatusBarWidget.WidgetPresentation getPresentation() {
    return this;
  }

  @Override
  public void install(StatusBar bar) {
    statusBar = bar;
    Disposable lifetime = this;
    EditorFactory.getInstance()
        .getEventMulticaster()
        .addCaretListener(
            new CaretListener() {
              @Override
              public void caretPositionChanged(CaretEvent event) {
                refresh();
              }
            },
            lifetime);
    connection = project.getMessageBus().connect(this);
    connection.subscribe(
        FileEditorManagerListener.FILE_EDITOR_MANAGER,
        new FileEditorManagerListener() {
          @Override
          public void selectionChanged(FileEditorManagerEvent event) {
            refresh();
          }
        });
  }

  private void refresh() {
    if (statusBar != null) {
      statusBar.updateWidget(ID);
    }
  }

  @Override
  public String getText() {
    Editor editor = FileEditorManager.getInstance(project).getSelectedTextEditor();
    if (editor == null) {
      return "";
    }
    VirtualFile file = FileDocumentManager.getInstance().getFile(editor.getDocument());
    if (file == null || !"salam".equalsIgnoreCase(file.getExtension())) {
      return "";
    }
    int line = editor.getCaretModel().getLogicalPosition().line;
    List<String> chain = SalamBlockChain.at(editor.getDocument().getText(), line);
    return SalamBlockChain.describe(chain);
  }

  @Override
  public float getAlignment() {
    return Component.LEFT_ALIGNMENT;
  }

  @Override
  public String getTooltipText() {
    return "The Salam blocks around the caret";
  }

  @Override
  public Consumer<MouseEvent> getClickConsumer() {
    return null;
  }

  @Override
  public void dispose() {
    statusBar = null;
  }

  /** Creates the widget for each project window. */
  public static final class Factory implements StatusBarWidgetFactory {
    @Override
    public String getId() {
      return ID;
    }

    @Override
    public String getDisplayName() {
      return "Salam Block Chain";
    }

    @Override
    public boolean isAvailable(Project project) {
      return true;
    }

    @Override
    public StatusBarWidget createWidget(Project project) {
      return new SalamBlockChainWidget(project);
    }

    @Override
    public boolean canBeEnabledOn(StatusBar bar) {
      return true;
    }
  }
}
