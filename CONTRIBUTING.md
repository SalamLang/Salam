# Contributing Guide for SalamLang/Salam

Welcome to Salam! We're excited to have you on board and contributing to our open-source project. This guide will help you navigate the contribution process and ensure that your contributions are valuable and well-received.

## Ways to Contribute

1. **Code Contributions**: Fix bugs, add new features, or improve existing code. Follow our coding conventions and guidelines (see below) when submitting code contributions.

2. **Documentation**: Write or improve documentation, including README files, code comments, and user guides, to help others understand and use Salam.

3. **Testing**: Ensure the quality and stability of Salam by writing and running tests, identifying bugs, and preventing regressions.

4. **Bug Reports**: Report any bugs or unexpected behavior you encounter, including steps to reproduce the issue and your environment details.

5. **Feature Requests**: Share your ideas for new features or improvements and discuss them with the community and project maintainers.

6. **Community Support**: Help others by answering questions, providing support, and sharing your expertise on our mailing list, forums, or chat channels.

## Getting Started

1. **Fork the Repository**: Create your own copy of the project by forking the SalamLang/Salam repository on GitHub.

2. **Clone Your Fork**: Clone the repository to your local machine using

   ```
   git clone https://github.com/SalamLang/Salam.git
   ```

3. **Create a Branch**: Create a new branch for your contribution using a descriptive name, such as feature/my-new-feature.

   ```
   git checkout -b feature/my-new-feature
   ```

4. **Make Changes**: Make your changes to the code, documentation, or tests as needed.

5. **Test Your Changes**: Thoroughly test your changes to ensure they work as expected.

6. **Commit Your Changes**: Commit your changes to your branch with a clear and descriptive commit message.

   ```
   git commit -am 'Add my new feature'
   ```

7. **Push Your Changes**: Push your changes to your fork on GitHub.

   ```
   git push origin feature/my-new-feature
   ```

8. **Submit a Pull Request**: Submit a pull request from your branch to the main repository, providing a clear description of your changes and why they're valuable.

## Code Style and Guidelines

To maintain consistency and readability, please follow these coding conventions and guidelines when contributing code:

- Write clear and descriptive variable names, function names, and comments.
- Use meaningful commit messages that describe the purpose of each commit.
- Keep code changes focused and atomic. Avoid making unrelated changes in the same pull request.
- Test your code thoroughly to ensure it functions correctly and doesn't introduce regressions.

### Formatting Tools

We encourage the use of formatting and linting tools to maintain code quality:

- clang-format: We use `clang-format` to maintain consistent code formatting for C/C++ files. Ensure that you run `clang-format` on your code before submitting a pull request. You can configure your editor to apply this automatically.

  To format your code using `clang-format`:

  ```bash
  clang-format -i <your-file>
  ```

- clang-tidy: Use `clang-tidy` to catch common errors, style issues, and potential bugs early in the development process. Running clang-tidy as part of your development workflow is highly recommended.

  To run clang-tidy:

  ```bash
  clang-tidy <your-file>
  ```

- Other Tools: Feel free to use other linting or static analysis tools, such as `cppcheck` or `valgrind`, to ensure the code is efficient and bug-free.

### Commit Hooks

We recommend using Git hooks or pre-commit hooks to automatically run formatters or linters on your code before each commit. You can set this up with the following steps:

1. Install `pre-commit`:

   ```bash
   pip install pre-commit
   ```

2. Set up the hooks:

   ```bash
   pre-commit install
   ```

## Feedback and Support

If you have any questions, feedback, or need support, don't hesitate to reach out to the project maintainers or the community. We're here to help and support each other in making Salam the best it can be.

Thank you for your interest in contributing to Salam! We appreciate your help in making this project a success. Happy coding!
