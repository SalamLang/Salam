package ir.salamlang.jetbrains;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

final class SalamExecutable {
  private SalamExecutable() {}

  static String find() {
    String exe = isWindows() ? "salam.exe" : "salam";
    String fromEnv = System.getenv("SALAM_PATH");
    if (fromEnv != null && !fromEnv.isBlank() && Files.isExecutable(Paths.get(fromEnv))) {
      return fromEnv;
    }
    String home = System.getProperty("user.home");
    if (home != null) {
      Path installed = Paths.get(home, ".salam", "bin", exe);
      if (Files.isExecutable(installed)) {
        return installed.toString();
      }
    }
    String path = System.getenv("PATH");
    if (path != null) {
      for (String dir : path.split(File.pathSeparator)) {
        if (dir.isEmpty()) {
          continue;
        }
        Path candidate = Paths.get(dir, exe);
        if (Files.isExecutable(candidate)) {
          return candidate.toString();
        }
      }
    }
    for (String dir : new String[] {"/usr/local/bin", "/opt/homebrew/bin", "/usr/bin"}) {
      Path candidate = Paths.get(dir, exe);
      if (Files.isExecutable(candidate)) {
        return candidate.toString();
      }
    }
    return exe;
  }

  private static boolean isWindows() {
    return System.getProperty("os.name", "").toLowerCase().startsWith("windows");
  }
}
