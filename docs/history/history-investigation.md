# Is there a historical link between One and Salam?

Question: does Salam descend from One-Language/One, so that One's history belongs before Salam's first commit?

Answer: no. The two projects are independent. They share an author, a coding style and repository scaffolding, but the evidence shows no transferred code and no common ancestry.

## The two repositories

|                        | One                                        | Salam                                             |
| ---------------------- | ------------------------------------------ | ------------------------------------------------- |
| Repository             | One-Language/One                           | SalamLang/Salam                                   |
| First commit           | 2018-06-30                                 | 2024-02-24                                        |
| Head inspected         | `d0a480e06ae389209122418db24a57f03d6d17be` | `a16421c3b2711d210083d10caef6a6bdab06537a` (main) |
| Commits                | 7,417 (all refs)                           | 3,891 (main)                                      |
| Fork                   | no                                         | no                                                |
| License                | GPL-3.0                                    | GPL-3.0                                           |
| Language it implements | One (`.one` files)                         | Salam (`.salam` files)                            |

Both mirrors are saved in `/home/developer/Backups/One-2026-10-31/` and `/home/developer/Backups/Salam-2026-10-31/`, each with a verified Git bundle.

## Tests and results

1. Shared file content. Every blob in either history was hashed and the two sets intersected. Of 74,848 Salam blobs and 4,321 One blobs, 20 are identical. All 20 are small scaffolding files: issue templates, `CODEOWNERS`, a workflow, `.gitattributes`, `android/.keep`, a 162 byte `Makefile`, a 23 byte `generator.c` stub and two 3 byte test outputs. No lexer, parser, compiler or library file is shared.

2. Source overlap over time. Meaningful C lines (longer than 12 characters, whitespace normalised) of Salam were compared with every `.c`, `.h` and `.one` line in One's whole history.

   | Salam commit | Date       | Salam C lines | Also in One |
   | ------------ | ---------- | ------------- | ----------- |
   | #5           | 2024-02-24 | 178           | 32 (18.0%)  |
   | #40          | 2024-02-25 | 587           | 54 (9.2%)   |
   | #100         | 2024-02-27 | 1,010         | 74 (7.3%)   |
   | #300         | 2024-03-02 | 1,640         | 71 (4.3%)   |
   | #600         | 2024-07-30 | 1,102         | 72 (6.5%)   |
   | #1000        | 2024-08-26 | 6,873         | 229 (3.3%)  |

   The overlap is highest in the first commits and falls as Salam grows.

3. What the early overlap is. The shared lines in Salam's first C file are `#include` lines, `typedef enum {`, `token_type_t`, `TOKEN_TYPE_*` names, `lexer_t`, `array_push(lexer->tokens, t)`, `fseek` and `ftell` calls. That is common C vocabulary plus one author's naming habits.

4. Direct comparison. Salam's first lexer (`sallam.c`, 296 normalised lines) was compared with One's `lexer.c`, `lexer.h` and `array.c` as of 2024-02-21, three days earlier. The sequence similarity is 0.064 and there is no matching block of four or more consecutive lines. Nothing was copied as a block.

5. Cross references. No One commit mentions Salam. Salam commits that match "one" patterns are about "one PDF per language" and similar English phrases. No file in Salam mentions onelang or One-Language, and no Salam pull request or issue discusses One as a source.

6. People. Shared commit emails: BaseMax (the owner), jbampton, dependabot, and a placeholder address. This shows a common maintainer, not shared code.

7. Activity. One is still active. Its last commit under `src/` is from 2023-06-11 and it has its own site, releases and contributors (1,048 commits by BaseMax, 229 by jbampton). Salam's history starts from a single file in a fresh repository ("Initial commit", then "start lexer").

## Conclusion

Salam was started as a new project on 2024-02-24. It reuses its author's conventions and the repository scaffolding from One, which is normal for one maintainer. Nothing shows that One's code, design documents or commits were moved into Salam.

The justified strategy is D: keep both histories independent and document the relationship. Joining them would give Salam a start date of 2018 and credit One's contributors with work they did on a different project.

## Limits

- The comparison is textual. A design idea carried over without any shared text would not be found.
- Only English line matching was used for the overlap test. Persian keyword tables were not compared.
- GitHub pull request refs (`refs/pull/*`) were fetched for both repositories, but only commit content was examined, not review discussions.
- If a specific file or design is later shown to come from One, strategy C applies: record that file's origin and attribution, without merging the whole history.
