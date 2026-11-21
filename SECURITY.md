# Security policy

## Supported versions

Salam is before 1.0. Security fixes go into the latest release and into `main`. Older releases are not patched, so update to the newest version first.

## Reporting a vulnerability

Please do not open a public issue for a security problem.

Report it privately through GitHub: open the repository's **Security** tab, choose **Report a vulnerability**, and describe the problem there. Include the Salam version (`salam version`), your platform, and the smallest program or request that shows it.

You can expect an answer within a few days. When the report is confirmed, the fix is prepared in private, released, and then described publicly with credit to the reporter if they want it.

## What is in scope

- The compiler and its backends (generated programs that are memory unsafe because of a compiler bug).
- The standard library, especially `std/net`, `std/tls`, `std/ssh`, `std/sshd`, `std/ftpd`, `std/crypto`, `std/jwt`, `std/db` and `std/archive`.
- The installers and the release packages.

Bugs that need a program to already misuse the language (for example freeing a buffer twice by hand) are normal bugs, not vulnerabilities, unless the standard library makes them easy to trigger from untrusted input.
