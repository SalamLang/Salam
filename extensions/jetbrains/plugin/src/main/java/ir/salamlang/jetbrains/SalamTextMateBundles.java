package ir.salamlang.jetbrains;

import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;
import org.jetbrains.plugins.textmate.api.TextMateBundleProvider;

/** Offers the Salam TextMate grammar that ships inside the plugin. */
public final class SalamTextMateBundles implements TextMateBundleProvider {
  @Override
  public List<PluginBundle> getBundles() {
    Path dir = bundleDir();
    if (dir == null) {
      return List.of();
    }
    return List.of(new PluginBundle("Salam", dir));
  }

  private static Path bundleDir() {
    try {
      Path jar =
          Paths.get(
              SalamTextMateBundles.class
                  .getProtectionDomain()
                  .getCodeSource()
                  .getLocation()
                  .toURI());
      Path root = jar.getParent() == null ? null : jar.getParent().getParent();
      if (root == null) {
        return null;
      }
      Path dir = root.resolve("textmate").resolve("salam");
      return Files.isDirectory(dir) ? dir : null;
    } catch (URISyntaxException | SecurityException | IllegalArgumentException e) {
      return null;
    }
  }
}
