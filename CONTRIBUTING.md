# 🤝 Contributing Guide for SalamLang/Salam

Welcome to Salam! We're glad you're interested in contributing to our open-source project. This guide will help you understand the contribution process and ensure your contributions are valuable and well-received.

## 🌟 Ways to Contribute

1. **Code Contributions**: Fix bugs, add new features, or improve existing code. Please follow our coding conventions and guidelines when submitting code contributions.

2. **Documentation**: Help write or improve documentation, including readme files, code comments, and user guides, to ensure others can understand and use Salam effectively.

3. **Testing**: Contribute to the quality and stability of Salam by writing and running tests, identifying bugs, and preventing regressions.

4. **Bug Reports**: Report bugs or unexpected behavior you encounter, providing steps to reproduce the issue and details about your environment.

5. **Feature Requests**: Share your ideas for new features or improvements and discuss them with the community and maintainers.

6. **Community Support**: Assist others by answering questions, providing support, and sharing your expertise on mailing lists, forums, or chat channels.

## 🚀 Getting Started

1. **Fork the Repository**: Visit [SalamLang/Salam](https://github.com/SalamLang/Salam) on GitHub and click **Fork** to create your own copy under your account.

2. **Clone Your Fork**: Clone your fork to your local machine, replacing `your-user-name` with your GitHub username:

   ```bash
   git clone https://github.com/your-user-name/Salam.git
   cd Salam
   ```

3. **Add the Upstream Remote**: Connect your local clone to the original repository so you can keep your fork in sync:

   ```bash
   git remote add upstream https://github.com/SalamLang/Salam.git
   ```

4. **Sync Your Fork**: Before starting any new work, always pull the latest changes from upstream into your local `main` branch and push them to your fork:

   ```bash
   git checkout main
   git fetch upstream
   git merge upstream/main
   git push origin main
   ```

5. **Create a Clean Feature Branch**: Create a new branch from a freshly synced `main` for **every pull request**. Use a short, descriptive name:

   ```bash
   git checkout -b feature/my-new-feature
   ```

   > **Important**: Always use a fresh branch per PR. Never reuse a branch that was already merged or submitted as a PR — start from a clean, up-to-date `main` each time.

6. **Make Changes**: Implement your changes to the code, documentation, or tests as necessary.

7. **Test Your Changes**: Ensure your changes work as expected by testing them thoroughly.

8. **Commit Your Changes**: Commit your changes with a clear and descriptive message.

   ```bash
   git commit -am 'Add new feature'
   ```

9. **Push Your Branch**: Push your feature branch to your fork on GitHub.

   ```bash
   git push origin feature/my-new-feature
   ```

10. **Submit a Pull Request**: On GitHub, open a pull request from your feature branch to `SalamLang/Salam:main`, providing a clear description of your changes and their purpose.

## 🏁 Basic Usage

After building the compiler (see the [Build section in the README](README.md#build)), you can run Salam programs right away.

**Build the compiler** from the `compiler/` directory:

```bash
cd compiler
sh tools/bash/build-compiler.sh   # quick build -> ./salam
```

Or with CMake:

```bash
cmake -B build && cmake --build build
```

**Hello, World** — save the following as `hello.salam`:

```salam
func main {
    println("Hello, World!")
}
```

Then compile and run it:

```bash
salam build hello.salam --output=hello
./hello
```

**Layout DSL** — compile a `.salam` layout page to HTML/CSS/JS:

```bash
salam layout build page.salam --inline
```

For a full list of commands and flags, see the [Usage section in the README](README.md#usage) or run:

```bash
salam --help
```

## 🔄 Git Daily Workflow

This is the typical day-to-day cycle for working on a contribution from start to finish.

### 1. Sync your fork

Always start from an up-to-date `main`:

```bash
git checkout main
git fetch upstream
git merge upstream/main
git push origin main
```

### 2. Create a fresh feature branch

```bash
git checkout -b fix/describe-the-fix
```

Use a prefix that matches your change: `feature/`, `fix/`, `docs/`, `test/`, etc.

### 3. Make your changes

Edit files, add tests, update documentation as needed.

### 4. Stage and commit

```bash
git add .
git commit -m "fix: describe what and why in plain terms"
```

Write commit messages in the imperative mood: _"Add feature"_, _"Fix crash"_, _"Update docs"_.

### 5. Keep your branch up to date (optional but recommended)

If `main` has moved on while you were working, merge the latest upstream changes into your branch:

```bash
git fetch upstream
git merge upstream/main
```

Resolve any conflicts, commit the merge, then push normally. Merging keeps a clear history and makes it easy to see exactly what changed on GitHub.

### 6. Push your branch

```bash
git push origin fix/describe-the-fix
```

### 7. Open a Pull Request

On GitHub, navigate to your fork and click **Compare & pull request**. Fill in the title and description, then submit. A maintainer will review your PR and may request changes.

### 8. Address review feedback

Make the requested changes locally, commit them, and push to the same branch — the PR updates automatically:

```bash
git add .
git commit -m "address review: clarify error message"
git push origin fix/describe-the-fix
```

### 9. After the PR is merged

Delete your feature branch locally and remotely to keep things tidy:

```bash
git checkout main
git branch -d fix/describe-the-fix
git push origin --delete fix/describe-the-fix
```

Then sync your fork so it is ready for your next contribution:

```bash
git fetch upstream
git merge upstream/main
git push origin main
```

## ↩️ Keeping Your Branch Up to Date with Rebase (Alternative)

Some contributors prefer to rebase instead of merge when bringing in upstream changes. Rebasing replays your commits on top of the latest `main`, resulting in a linear history with no merge commits.

> **Note**: Rebasing rewrites commit history. Avoid it on branches that other people are also working on, and never rebase commits that have already been pushed to a shared branch without coordinating with your team.

### Rebase your branch onto upstream

```bash
git fetch upstream
git rebase upstream/main
```

If there are conflicts, Git will pause and let you resolve them file by file:

```bash
# After resolving each conflict:
git add <resolved-file>
git rebase --continue
```

To abort and return to the state before the rebase started:

```bash
git rebase --abort
```

### Force-push after rebasing

Because rebase rewrites history, you need to force-push after rebasing a branch that you have already pushed:

```bash
git push --force-with-lease origin fix/describe-the-fix
```

Use `--force-with-lease` rather than `--force` — it refuses to overwrite if someone else has pushed to the branch since your last fetch, protecting against accidental data loss.

## 📝 Code Style and Guidelines

To maintain consistency and readability, please follow these coding conventions when contributing code:

- Use clear and descriptive variable names, function names, and comments.
- Write meaningful commit messages that explain the purpose of each commit.
- Keep your changes focused and atomic. Avoid mixing unrelated changes in the same pull request.
- Test your code thoroughly to ensure its functionality and prevent regressions.

### 🛠️ Additional Code Quality Tools

To help improve efficiency and correctness, you may use the following linting and static analysis tools:

- **Tools**: You may use linting or static analysis tools, such as `cppcheck` or `valgrind`, to improve the efficiency and correctness of the code.

### 🪝 Commit Hooks

We use [`prek`](https://prek.j178.dev) for repository hooks. Salam currently has three hook modes:

- **Standard hooks**: the default checks in `prek.toml` that run for the installed Git hook stages (currently `pre-commit` and `pre-push`).
- **Manual hooks**: hooks in `prek.toml` that are marked for the `manual` stage and are run on demand.
- **Audit hooks**: security-focused hooks in `prek-audit.toml` that are run separately from the standard set.

#### Install `prek`

Choose any of these supported installation methods:

- **Homebrew**

  ```bash
  brew install prek
  ```

- **uv**

  ```bash
  uv tool install prek
  ```

- **pipx**

  ```bash
  pipx install prek
  ```

- **pip**

  ```bash
  python -m pip install prek
  ```

#### Set up and run hooks

1. Install the standard Git hooks and prepare hook environments:

   ```bash
   prek install --prepare-hooks
   ```

2. Run the standard hooks across the repository:

   ```bash
   prek run --all-files
   ```

3. Run the manual hooks:

   ```bash
   prek run --all-files --stage manual
   ```

4. Run the audit hooks:

   ```bash
   prek run --all-files --config prek-audit.toml
   ```

#### Useful hook commands

Run just one hook:

```bash
prek run markdownlint --files CONTRIBUTING.md
```

Skip one or more hooks for a single `prek` run:

```bash
SKIP=markdownlint,codespell prek run --files CONTRIBUTING.md
```

Skip installed Git hooks for one commit:

```bash
git commit --no-verify -m "docs: update contributing guide"
```

Use `git commit --no-verify` only when you intentionally need to bypass the installed Git hooks. If you skip hooks locally, make sure you run the relevant `prek` command yourself before opening or updating a pull request.

Check `prek run` help and output controls:

```bash
prek run -h
```

A few useful output flags from `prek run -h` are:

- `--no-progress`: hides the live progress display, such as spinners and in-place progress updates, while still showing the normal hook results.
- `-q`, `--quiet`: suppresses normal hook output so you can focus on failures or just rely on the exit status. Environment variable: `PREK_QUIET`.
- `-v`, `--verbose`: increases output verbosity so you can inspect extra hook execution detail when troubleshooting.

Clean cached hook environments:

```bash
prek clean
```

`prek clean` removes cached hook data so the next run recreates hook environments. This is useful after upgrading `prek`, changing hook definitions, or recovering from a broken local hook cache.

#### Hook priority in this repository

Hooks run from the lowest `priority` value to the highest value.

In this repository, that means:

- `10`: repository meta checks such as `identity` and `check-hooks-apply`
- `20`: early file cleanup such as `fix-byte-order-marker`
- `30`: whitespace cleanup such as `trailing-whitespace`
- `40`: formatters such as `prettier`, `biome-check`, and `shfmt`
- `50`: validation and audit hooks such as `codespell`, `markdownlint`, `yamllint`, `detect-private-key`, and `gitleaks`

This ordering lets fast repository checks and automatic cleanup happen before the heavier validation and audit steps.

## 💬 Feedback and Support

If you have any questions or need assistance, feel free to reach out to the project maintainers or the community. We're here to support each other in making Salam the best it can be.

Thank you for your interest in contributing to Salam! We appreciate your help in making this project a success. Happy coding!
