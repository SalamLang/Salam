package ir.salamlang.jetbrains;

import java.io.File;
import java.util.List;

/** Checks the block chain and salam.toml readers, which need no IDE. */
public final class ChainSmoke {
  private ChainSmoke() {}

  private static int failures;

  private static void eq(String what, Object want, Object got) {
    if (!want.equals(got)) {
      failures++;
      System.out.println("FAIL " + what + ": want " + want + ", got " + got);
    }
  }

  /**
   * Runs the checks.
   *
   * @param args unused
   */
  public static void main(String[] args) {
    String src =
        String.join(
            "\n",
            "page:",
            "    box:",
            "        // end: a comment",
            "        button: id = \"go\" click = (): n++ end end",
            "        paragraph:",
            "            content = \"x:\"",
            "        end",
            "    end",
            "end",
            "",
            "pub func main(): int:",
            "    if a:",
            "        x := 1",
            "    else b:",
            "        y := 2",
            "    end",
            "    ret 0",
            "end");
    eq("page", "page", SalamBlockChain.describe(SalamBlockChain.at(src, 0)));
    eq("on box", "page > box", SalamBlockChain.describe(SalamBlockChain.at(src, 1)));
    eq("inside box", "page > box", SalamBlockChain.describe(SalamBlockChain.at(src, 2)));
    eq("one line button", "page > box", SalamBlockChain.describe(SalamBlockChain.at(src, 3)));
    eq("on paragraph", "page > box > paragraph", SalamBlockChain.describe(SalamBlockChain.at(src, 4)));
    eq("content", "page > box > paragraph", SalamBlockChain.describe(SalamBlockChain.at(src, 5)));
    eq("after paragraph end", "page > box", SalamBlockChain.describe(SalamBlockChain.at(src, 7)));
    eq("top level", "", SalamBlockChain.describe(SalamBlockChain.at(src, 9)));
    eq("on main", "func main", SalamBlockChain.describe(SalamBlockChain.at(src, 10)));
    eq("in if", "func main > if", SalamBlockChain.describe(SalamBlockChain.at(src, 12)));
    eq("on if", "func main > if", SalamBlockChain.describe(SalamBlockChain.at(src, 11)));
    eq("in else", "func main > else", SalamBlockChain.describe(SalamBlockChain.at(src, 14)));
    eq("after if", "func main", SalamBlockChain.describe(SalamBlockChain.at(src, 16)));
    eq("persian", "تابع ریشه", SalamBlockChain.describe(SalamBlockChain.at("تابع ریشه:\n    ret 0\nend", 1)));

    String toml =
        "[package]\nname = \"demo\"\nplatforms = [\"nope\"]\n\n[project]\ntype = \"dsl\"\n"
            + "platforms = [\"web\", \"android\"] # kept\nweb = \"page\"\n\n[design]\ndesign = \"shadcn\"\n";
    SalamProject p = SalamProject.parse(new File("."), toml);
    eq("type", "dsl", p.type);
    eq("platforms", List.of("web", "android"), p.platforms);
    eq("first", "web", p.firstPlatform());
    SalamProject multi = SalamProject.parse(new File("."), "[project]\nplatforms = [\n  \"linux\",\n  \"macos\",\n]\n");
    eq("multi line", List.of("linux", "macos"), multi.platforms);
    eq("none", "", SalamProject.parse(new File("."), "[package]\nname = \"x\"\n").firstPlatform());

    if (failures > 0) {
      System.exit(1);
    }
    System.out.println("chain ok");
  }
}
