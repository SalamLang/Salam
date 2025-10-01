# Contributing Guide for SalamLang/Salam

Welcome to Salam! We're glad you're interested in contributing to our open-source project. This guide will help you understand the contribution process and ensure your contributions are valuable and well-received.

## Ways to Contribute

1. **Code Contributions**: Fix bugs, add new features, or improve existing code. Please follow our coding conventions and guidelines when submitting code contributions.

2. **Documentation**: Help write or improve documentation, including readme files, code comments, and user guides, to ensure others can understand and use Salam effectively.

3. **Testing**: Contribute to the quality and stability of Salam by writing and running tests, identifying bugs, and preventing regressions.

4. **Bug Reports**: Report bugs or unexpected behavior you encounter, providing steps to reproduce the issue and details about your environment.

5. **Feature Requests**: Share your ideas for new features or improvements and discuss them with the community and maintainers.

6. **Community Support**: Assist others by answering questions, providing support, and sharing your expertise on mailing lists, forums, or chat channels.

## Getting Started

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

## Code Style and Guidelines

To maintain consistency and readability, please follow these coding conventions when contributing code:

- Use clear and descriptive variable names, function names, and comments.
- Write meaningful commit messages that explain the purpose of each commit.
- Keep your changes focused and atomic. Avoid mixing unrelated changes in the same pull request.
- Test your code thoroughly to ensure its functionality and prevent regressions.

### Formatting Tools

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

### Commit Hooks

We recommend using Git hooks or pre-commit hooks to automatically run formatters or linters before each commit. You can set this up with the following steps:

1. Install `pre-commit`:

   ```bash
   pip install pre-commit
   ```

2. Set up the hooks:

   ```bash
   pre-commit install
   ```

## Feedback and Support

If you have any questions or need assistance, feel free to reach out to the project maintainers or the community. We're here to support each other in making Salam the best it can be.

Thank you for your interest in contributing to Salam! We appreciate your help in making this project a success. Happy coding!
