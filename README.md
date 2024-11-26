<div align="center">

<p>
    <a href="https://salamlang.ir/">
        <img width="150" src="https://raw.githubusercontent.com/SalamLang/Salam/main/assets/logo-box.svg" alt="The Salam Programming Language - زبان برنامه نویسی سلام">
    </a>
</p>

# Salam Language

Salam is a new programming language for local developers. Everyone can learn programming and write code.

</div>

---

# Salam Language: The Salam Programming Language

Salam Language, inspired by the Persian and Arabic roots of the word `salam`, which means `peace`, is a [programming language](https://en.wikipedia.org/wiki/Programming_language) crafted for simplicity and accessibility within the Iranian, Persian-speaking, and Arabic-speaking communities. Serving as the first Persian and Arabic programming language, Salam Language is designed to provide an intuitive and user-friendly experience, specifically tailored for users, students, and developers familiar with Persian or Arabic. Embracing the cultural and linguistic richness of the region, Salam Language aims to empower a diverse range of individuals, making programming more approachable and fostering a sense of ease and collaboration in the local coding community.

```
$ ./salam
Welcome to Salam Programming Language!
Salam is the first Persian/Iranian computer scripting language.

Usage:
./salam <filename>                      # Execute a Salam script
./salam code <content> <output_dir>     # Compile and run Salam code

./salam lint <filename> <output_dir>    # Lint a Salam script
./salam lint code <content>             # Lint Salam code

./salam version                         # Print the version of Salam

./salam update                          # Update Salam to the latest version

Feel free to explore and create using Salam!
For more information, visit: https://salamlang.ir
```

### Docker

```
git clone https://github.com/SalamLang/Salam
cd Salam

docker build -f docker/Dockerfile -t salam .
docker images salam
docker run --rm -it salam ls -l /app
docker run --rm -it salam
```

## Contributing

Want to contribute to Salam? Check out our [Contributing Guide](CONTRIBUTING.md) for more information.

All contributors are expected to adhere to our [Code of Conduct](CODE_OF_CONDUCT.md). Please make sure you are familiar with it before participating in this project.

Copyright 2024, Salam Language Team
