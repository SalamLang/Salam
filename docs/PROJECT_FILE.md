# The project file, salam.toml

`salam.toml` sits in the project folder. `salam build`, `salam run`, `salam bundle`,
`salam sign` and the phone app scripts read it. Paths in it are relative to the folder
that holds it. Key names are English. A command line option wins over the file.

The Windows and signing settings of `salam build` are read when it builds a project folder
(`salam build` or `salam build DIR`). A build of a single file takes them from options,
except `--sign`, which also reads the `salam.toml` next to that file.

## Package and project

| table                        | keys                                                                                                        | read by                                                               |
| ---------------------------- | ----------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- |
| `[package]`                  | `name`, `version`, `salam` (oldest compiler, such as `">=0.7.0"`), `kind` (`"lib"` builds a shared library) | the package manager and `salam build`; see [PACKAGES.md](PACKAGES.md) |
| `[lib]`                      | `export_pub`                                                                                                | library builds                                                        |
| `[project]`                  | `type` (`cli`, `http`, `dsl`), `platforms`, `web`                                                           | `salam build`, `run` and `check`                                      |
| `[design]`                   | `design`                                                                                                    | page builds                                                           |
| `[dependencies]`, `[native]` | see [PACKAGES.md](PACKAGES.md)                                                                              | the package manager                                                   |

## [app]: the identity every platform shares

| key                                                                                                                  | meaning                                                                     |
| -------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| `id`                                                                                                                 | reverse domain identifier, `com.example.notes`; `salam bundle` needs it     |
| `name`                                                                                                               | name people see; the Windows product name                                   |
| `version`                                                                                                            | `1.2.0`                                                                     |
| `build`                                                                                                              | build number of a phone app                                                 |
| `description`                                                                                                        | Windows file description, package description                               |
| `author`, `publisher`                                                                                                | Windows company, maintainer of the `.deb`                                   |
| `copyright`                                                                                                          | copyright line                                                              |
| `icon`                                                                                                               | a `.png` or `.ico` file; Windows builds embed either, bundles convert a PNG |
| `main`                                                                                                               | entry file for `salam bundle`, `main.salam` by default                      |
| `assets`                                                                                                             | folder packed into a desktop bundle                                         |
| `category`                                                                                                           | desktop menu category, or a phone store category                            |
| `engine`, `runtime`                                                                                                  | web view engine of a desktop bundle, see [DESKTOP.md](DESKTOP.md)           |
| `icon_bg`, `splash_color`, `splash_icon`, `orientation`, `permissions`, `permission`, `fonts`, `scheme`, `link_host` | phone apps, see [MOBILE.md](MOBILE.md)                                      |

## [windows]

Takes every `[app]` key, which then applies to Windows only, and:

| key                                                         | meaning                                                                   |
| ----------------------------------------------------------- | ------------------------------------------------------------------------- |
| `subsystem`                                                 | `console` or `windows` (no console window)                                |
| `icon`                                                      | `.ico` or `.png` file for the program                                     |
| `manifest`                                                  | application manifest file                                                 |
| `version`, `company`, `description`, `product`, `copyright` | version resource                                                          |
| `sign_tool`                                                 | `auto` (default), `signtool` or `osslsigncode`                            |
| `sign_cert`                                                 | certificate file: `.pfx` or `.p12`, or a certificate used with `sign_key` |
| `sign_key`                                                  | private key file, osslsigncode only                                       |
| `sign_thumbprint`                                           | SHA-1 thumbprint of a certificate in the Windows store, signtool only     |
| `sign_command`                                              | your own signer: a list, the program and then one argument per item       |
| `timestamp_url`                                             | RFC 3161 timestamp server, `none` to leave the timestamp out              |

When the file has no `[windows]` value, the version, product name, description, company,
copyright and icon come from `[app]`.

## [macos]

Takes every `[app]` key, and:

| key                                            | meaning                                                                                 |
| ---------------------------------------------- | --------------------------------------------------------------------------------------- |
| `sign_identity`                                | codesign identity, such as `Developer ID Application: NAME (TEAMID)`, or `-` for ad-hoc |
| `entitlements`                                 | entitlements property list                                                              |
| `hardened_runtime`                             | `true` by default                                                                       |
| `timestamp_url`                                | timestamp server; Apple's is used when empty                                            |
| `notarize`                                     | `true` sends the signed file to Apple and staples the ticket                            |
| `notary_profile`                               | keychain profile made with `xcrun notarytool store-credentials`                         |
| `notary_key`, `notary_key_id`, `notary_issuer` | App Store Connect API key file, its id and issuer, used without a profile               |

## [linux]

Takes every `[app]` key, and `gpg_key`: the GPG key that writes a detached `.asc`
signature next to each package.

## [android] and [ios]

Take every `[app]` key. `[android]` adds `min_sdk`, `target_sdk`, `abi`, `keystore` and
`key_alias`; `[ios]` adds `min_ios`, `team`, `profile`, `sign_identity` and `ipa`.
`[app.mobile]`, `[app.android]` and `[app.ios]` still work. See [MOBILE.md](MOBILE.md).

## Passwords and tokens

Passwords and tokens are never read from `salam.toml`. A key such as `sign_password`,
`pfx_password`, `notary_password` or `keystore_pass` is ignored with a warning. Use:

| secret                                              | where it comes from                                                              |
| --------------------------------------------------- | -------------------------------------------------------------------------------- |
| Windows certificate or key password, GPG passphrase | `SALAM_SIGN_PASSWORD`, `SALAM_SIGN_PASSWORD_FILE` or `--sign-password-file=FILE` |
| Apple notary credentials                            | the macOS keychain (`notary_profile`) or the API key file                        |
| Android keystore                                    | `SALAM_KEYSTORE_PASS`, `SALAM_KEY_PASS`                                          |

## Signing happens only when asked

The signing keys do nothing until a command has `--sign` (`salam build --sign`,
`salam bundle --sign`) or is `salam sign`. A `sign_command` in a project you did not
write runs that program on your computer, so read the file before you pass `--sign`.
[DESKTOP.md](DESKTOP.md#signing) has the full guide.
