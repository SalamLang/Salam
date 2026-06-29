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

1. **Fork the Repository**: Create a copy of the project by forking the SalamLang/Salam repository on GitHub.

2. **Clone Your Fork**: Clone the repository to your local machine by replacing `your-user-name` with your own:

   ```
   git clone https://github.com/your-user-name/Salam.git
   ```

3. **Create a Branch**: Create a new branch for your contribution with a descriptive name, such as `feature/my-new-feature`.

   ```
   git checkout -b feature/my-new-feature
   ```

4. **Make Changes**: Implement your changes to the code, documentation, or tests as necessary.

5. **Test Your Changes**: Ensure your changes work as expected by testing them thoroughly.

6. **Commit Your Changes**: Commit your changes with a clear and descriptive message.

   ```
   git commit -am 'Add new feature'
   ```

7. **Push Your Changes**: Push your changes to your fork on GitHub.

   ```
   git push origin feature/my-new-feature
   ```

8. **Submit a Pull Request**: Submit a pull request from your branch to the main repository, providing a description of your changes and their value.

## 📝 Code Style and Guidelines

To maintain consistency and readability, please follow these coding conventions when contributing code:

- Use clear and descriptive variable names, function names, and comments.
- Write meaningful commit messages that explain the purpose of each commit.
- Keep your changes focused and atomic. Avoid mixing unrelated changes in the same pull request.
- Test your code thoroughly to ensure its functionality and prevent regressions.

### 🛠️ Formatting Tools

To help maintain consistent code quality, we use the following formatting and linting tools:

- **clang-format**: We use `clang-format` to ensure consistent formatting for C/C++ files. Please run `clang-format` before submitting a pull request. You can configure your editor to do this automatically.

  To format your code using `clang-format`:

  ```bash
  clang-format -i <your-file>
  ```

- **clang-tidy**: Use `clang-tidy` to catch common errors, style issues, and potential bugs. It's recommended to run `clang-tidy` as part of your workflow.

  To run `clang-tidy`:

  ```bash
  clang-tidy <your-file>
  ```

- **Other Tools**: You may also use additional linting or static analysis tools, such as `cppcheck` or `valgrind`, to improve the efficiency and correctness of the code.

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
