package ir.salamlang.jetbrains;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * What the plugin needs from a project's {@code salam.toml}: where the project starts and the
 * {@code platforms} listed under {@code [project]}. It reads only that table, with no TOML library.
 */
final class SalamProject {
  static final List<String> PLATFORMS =
      List.of("web", "js", "android", "ios", "linux", "macos", "windows", "all");

  private static final Pattern STRING = Pattern.compile("\"([^\"]*)\"|'([^']*)'");

  final File root;
  final String type;
  final List<String> platforms;

  SalamProject(File root, String type, List<String> platforms) {
    this.root = root;
    this.type = type;
    this.platforms = platforms;
  }

  /** The platform a plain {@code salam run} or {@code salam build} would use, or "". */
  String firstPlatform() {
    return platforms.isEmpty() ? "" : platforms.get(0);
  }

  /** The nearest directory at or above {@code start} that holds a {@code salam.toml}, or null. */
  static File findRoot(File start) {
    File dir = start;
    if (dir != null && !dir.isDirectory()) {
      dir = dir.getParentFile();
    }
    while (dir != null) {
      if (new File(dir, "salam.toml").isFile()) {
        return dir;
      }
      dir = dir.getParentFile();
    }
    return null;
  }

  /** Reads the project that contains {@code start}, or returns null when there is none. */
  static SalamProject find(File start) {
    File root = findRoot(start);
    if (root == null) {
      return null;
    }
    try {
      byte[] bytes = Files.readAllBytes(new File(root, "salam.toml").toPath());
      return parse(root, new String(bytes, StandardCharsets.UTF_8));
    } catch (IOException e) {
      return new SalamProject(root, "", List.of());
    }
  }

  static SalamProject parse(File root, String text) {
    String type = "";
    List<String> platforms = new ArrayList<>();
    boolean inProject = false;
    String[] lines = text.split("\r?\n");
    for (int i = 0; i < lines.length; i++) {
      String line = stripComment(lines[i]).trim();
      if (line.startsWith("[")) {
        inProject = line.equals("[project]");
        continue;
      }
      if (!inProject) {
        continue;
      }
      int eq = line.indexOf('=');
      if (eq < 0) {
        continue;
      }
      String key = line.substring(0, eq).trim();
      String value = line.substring(eq + 1).trim();
      if (key.equals("type")) {
        type = firstString(value);
      } else if (key.equals("platforms")) {
        StringBuilder all = new StringBuilder(value);
        while (all.indexOf("]") < 0 && i + 1 < lines.length) {
          i++;
          all.append(' ').append(stripComment(lines[i]).trim());
        }
        Matcher m = STRING.matcher(all);
        while (m.find()) {
          platforms.add(m.group(1) != null ? m.group(1) : m.group(2));
        }
      }
    }
    return new SalamProject(root, type, platforms);
  }

  private static String firstString(String value) {
    Matcher m = STRING.matcher(value);
    if (!m.find()) {
      return "";
    }
    return m.group(1) != null ? m.group(1) : m.group(2);
  }

  private static String stripComment(String line) {
    char quote = 0;
    for (int i = 0; i < line.length(); i++) {
      char c = line.charAt(i);
      if (quote != 0) {
        if (c == quote) {
          quote = 0;
        }
      } else if (c == '"' || c == '\'') {
        quote = c;
      } else if (c == '#') {
        return line.substring(0, i);
      }
    }
    return line;
  }
}
