# 🤝 Contributing Guide for SalamLang/Salam

Welcome to Salam! We're glad you're interested in contributing to our open-source project. This guide will help you understand the contribution process and ensure your contributions are valuable and well-received.

Please read our [Code of Conduct](./CODE_OF_CONDUCT.md) before contributing.

---

## 🌟 Ways to Contribute

1. **Code Contributions**: Fix bugs, add new features, or improve existing code. Please follow our coding conventions and guidelines when submitting code contributions.

2. **Documentation**: Help write or improve documentation, including readme files, code comments, and user guides, to ensure others can understand and use Salam effectively.

3. **Testing**: Contribute to the quality and stability of Salam by writing and running tests, identifying bugs, and preventing regressions.

4. **Bug Reports**: Report bugs or unexpected behavior you encounter, providing steps to reproduce the issue and details about your environment.

5. **Feature Requests**: Share your ideas for new features or improvements and discuss them with the community and maintainers.

6. **Community Support**: Assist others by answering questions, providing support, and sharing your expertise on mailing lists, forums, or chat channels.

7. **Localization**: Help expand Arabic language support or improve Persian/English keyword coverage.

---

## 🚀 Getting Started

1. **Fork the Repository**: Create a copy of the project by forking the SalamLang/Salam repository on GitHub.

2. **Clone Your Fork**: Clone the repository to your local machine (replace `your-user-name` with your GitHub username):

   ```bash
   git clone https://github.com/your-user-name/Salam.git
   cd Salam
   ```

3. **Create a Branch**: Create a new branch for your contribution with a descriptive name:

   ```bash
   git checkout -b feature/my-new-feature
   ```

4. **Make Changes**: Implement your changes to the code, documentation, or tests as necessary.

5. **Test Your Changes**: Ensure your changes work as expected (see [Building & Testing](#-building--testing) below).

6. **Commit Your Changes**: Commit your changes with a clear and descriptive message:

   ```bash
   git commit -am 'Add new feature'
   ```

7. **Push Your Changes**: Push your changes to your fork on GitHub:

   ```bash
   git push origin feature/my-new-feature
   ```

8. **Submit a Pull Request**: Open a pull request from your branch to the main repository, providing a description of your changes and their value.

---

## 🔨 Building & Testing

All build commands should be run from the root directory.

### Quick Build (recommended)

Requires a C compiler. `tcc` is the default backend (bundled, fast); `gcc` or `clang` also work.

```bash
sh tools/build-compiler.sh   # builds ./salam
```

### Build with CMake

```bash
cd compiler
cmake -B build
cmake --build build
ctest --test-dir build   # run the test suite
```

### Build with Make

```bash
cd compiler
make   # release build
```

### Run the Compiler

```bash
# General language → native executable
./salam build app.salam --output=app.exe

# Layout DSL → website
./salam layout build page.salam

# Format source files
./salam fmt app.salam           # reformat one file
./salam fmt --check             # check formatting without modifying
```

### Docker (optional)

For a consistent build environment using Docker:

```bash
# Development mode (live reload on src/ changes)
docker compose -f compiler/docker/docker-compose.yml up dev

# Production build
docker compose -f compiler/docker/docker-compose.yml build prod
docker compose -f compiler/docker/docker-compose.yml run --rm prod build examples/en/basics/hello.salam --output=hello
```

---

## 📝 Code Style and Guidelines

To maintain consistency and readability, please follow these coding conventions when contributing code:

- Use clear and descriptive variable names, function names, and comments.
- Write meaningful commit messages that explain the purpose of each commit.
- Keep your changes focused and atomic. Avoid mixing unrelated changes in the same pull request.
- Test your code thoroughly to ensure its functionality and prevent regressions.

### 🛠️ Formatting Tools

We use the following formatting and linting tools to maintain consistent code quality:

- **clang-format**: Ensures consistent formatting for C/C++ files. Run before submitting a pull request:

  ```bash
  clang-format -i <your-file>
  ```

- **salam fmt**: For `.salam` source files, use the built-in formatter:

  ```bash
  ./salam fmt <your-file.salam>
  ./salam fmt --check   # verify without modifying
  ```

- **clang-tidy**: Catches common errors, style issues, and potential bugs:

  ```bash
  clang-tidy <your-file>
  ```

- **Other Tools**: You may also use `cppcheck` or `valgrind` for additional static analysis and memory checking.

### 🪝 Commit Hooks

We use [prek](https://prek.j178.dev) — a fast, Rust-based drop-in alternative to `pre-commit` — to automatically run formatters and linters before each commit. It is fully compatible with `.pre-commit-config.yaml` and requires no Python runtime.

See our config: [`.pre-commit-config.yaml`](./.pre-commit-config.yaml)

**Install prek:**

```bash
# Using pip
pip install prek

# Using uv (recommended)
uv tool install prek

# Using pipx
pipx install prek
```

**Set up hooks:**

```bash
prek install
```

**Run all hooks against all files:**

```bash
prek run -a
```

That's it — every `git commit` will now trigger prek automatically.

---

## 💬 Feedback and Support

If you have any questions or need assistance, feel free to reach out to the project maintainers or the community. We're here to support each other in making Salam the best it can be.

For general inquiries or feedback, visit: [salamlang.ir](https://salamlang.ir)

Thank you for your interest in contributing to Salam! We appreciate your help in making this project a success. Happy coding!

---

© 2024-2026 Salam Language Team
