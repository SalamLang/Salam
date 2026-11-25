package ir.salamlang.jetbrains;

import java.util.ArrayList;
import java.util.List;

/**
 * Works out which blocks enclose a line of Salam source, such as {@code page > box > button}.
 *
 * <p>A line that ends in a colon opens a block, a line that starts with {@code end} (or the
 * Persian {@code پایان}) closes one, and a line that holds a whole block between its colon and its
 * {@code end} changes nothing. {@code else} lines swap the label of the open block. The scan is
 * lexical on purpose: it needs no parser and works while the file has errors.
 */
final class SalamBlockChain {
  private static final String END = "end";
  private static final String END_FA = "پایان";

  private SalamBlockChain() {}

  /** The labels of the blocks that enclose {@code line} (0 based) of {@code text}, outermost first. */
  static List<String> at(String text, int line) {
    String[] lines = text.split("\r?\n", -1);
    List<String> stack = new ArrayList<>();
    boolean[] inBlockComment = {false};
    for (int i = 0; i <= line && i < lines.length; i++) {
      String trimmed = code(lines[i], inBlockComment).trim();
      if (trimmed.isEmpty()) {
        continue;
      }
      String first = firstWord(trimmed);
      boolean opens = trimmed.endsWith(":");
      if (i == line) {
        if (opens && !isElse(first)) {
          stack.add(label(trimmed, first));
        }
        break;
      }
      int closers = countEnds(trimmed);
      if (isEnd(first)) {
        pop(stack, closers);
      } else if (opens) {
        if (isElse(first)) {
          if (!stack.isEmpty()) {
            stack.set(stack.size() - 1, first);
          }
        } else {
          stack.add(label(trimmed, first));
        }
        pop(stack, closers);
      } else if (isElse(first)) {
        pop(stack, closers);
      }
    }
    return stack;
  }

  /** The chain as shown in the status bar, with " > " between blocks. */
  static String describe(List<String> chain) {
    return String.join(" > ", chain);
  }

  private static void pop(List<String> stack, int count) {
    for (int i = 0; i < count && !stack.isEmpty(); i++) {
      stack.remove(stack.size() - 1);
    }
  }

  private static boolean isEnd(String word) {
    return word.equals(END) || word.equals(END_FA);
  }

  private static boolean isElse(String word) {
    return word.equals("else") || word.equals("وگرنه");
  }

  private static boolean isWordChar(char c) {
    return Character.isLetterOrDigit(c) || c == '_' || c == '‌';
  }

  private static String firstWord(String s) {
    int i = 0;
    while (i < s.length() && isWordChar(s.charAt(i))) {
      i++;
    }
    return s.substring(0, i);
  }

  private static String label(String line, String first) {
    if (first.isEmpty() || Character.isDigit(first.charAt(0))) {
      return "case";
    }
    switch (first) {
      case "pub":
      case "همگانی":
      case "inline":
      case "pure":
      case "noinline":
      case "deprecated":
        String after = line.substring(first.length()).trim();
        return label(after, firstWord(after));
      case "func":
      case "تابع":
      case "struct":
      case "ساختار":
      case "enum":
      case "جداشمار":
      case "interface":
      case "میانجی":
      case "impl":
      case "کاربست":
      case "component":
        String name = firstWord(line.substring(first.length()).trim());
        return name.isEmpty() ? first : first + " " + name;
      default:
        return first;
    }
  }

  private static int countEnds(String code) {
    int n = 0;
    int i = 0;
    while (i < code.length()) {
      if (isWordChar(code.charAt(i))) {
        int j = i;
        while (j < code.length() && isWordChar(code.charAt(j))) {
          j++;
        }
        boolean member = i > 0 && code.charAt(i - 1) == '.';
        if (!member && isEnd(code.substring(i, j))) {
          n++;
        }
        i = j;
      } else {
        i++;
      }
    }
    return n;
  }

  /**
   * The line with comments dropped and the inside of strings blanked, so words in them are not read
   * as code. {@code inBlockComment[0]} carries an open block comment to the next line.
   */
  private static String code(String line, boolean[] inBlockComment) {
    StringBuilder out = new StringBuilder();
    char quote = 0;
    for (int i = 0; i < line.length(); i++) {
      char c = line.charAt(i);
      char next = i + 1 < line.length() ? line.charAt(i + 1) : 0;
      if (inBlockComment[0]) {
        if (c == '*' && next == '/') {
          inBlockComment[0] = false;
          i++;
        }
      } else if (quote != 0) {
        if (c == '\\') {
          i++;
        } else if (c == quote) {
          quote = 0;
          out.append(c);
        }
      } else if (c == '/' && next == '/') {
        break;
      } else if (c == '/' && next == '*') {
        inBlockComment[0] = true;
        i++;
      } else if (c == '"' || c == '`' || c == '\'' || c == '«') {
        quote = c == '«' ? '»' : c;
        out.append(c);
      } else {
        out.append(c);
      }
    }
    return out.toString();
  }
}
