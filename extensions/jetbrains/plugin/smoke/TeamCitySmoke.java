import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import jetbrains.buildServer.messages.serviceMessages.ServiceMessage;

/** Reads salam test --format=teamcity output with the parser IntelliJ uses for the test tree. */
public final class TeamCitySmoke {
  private TeamCitySmoke() {}

  /**
   * Counts the tests in the messages on standard input.
   *
   * @param args unused
   * @throws Exception when a message does not parse
   */
  public static void main(String[] args) throws Exception {
    int started = 0;
    int failed = 0;
    int finished = 0;
    String failure = "";
    BufferedReader in =
        new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
    for (String line = in.readLine(); line != null; line = in.readLine()) {
      if (!line.startsWith("##teamcity[")) {
        continue;
      }
      ServiceMessage m = ServiceMessage.parse(line);
      if (m == null) {
        throw new IllegalStateException("not a service message: " + line);
      }
      switch (m.getMessageName()) {
        case "testStarted" -> started++;
        case "testFailed" -> {
          failed++;
          failure = m.getAttributes().get("message");
        }
        case "testFinished" -> finished++;
        default -> {}
      }
    }
    System.out.println(
        "teamcity: "
            + started
            + " started, "
            + finished
            + " finished, "
            + failed
            + " failed: "
            + failure);
  }
}
