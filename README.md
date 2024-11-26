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

## Using Salam with Docker

The Salam programming language can be run inside a Docker container, providing an isolated and consistent environment for development and execution. Follow the steps below to get started.

https://hub.docker.com/repository/docker/basemax/salam

### Step 1: Pull the Image

To pull the Salam Docker image from Docker Hub, run the following command:

```bash
docker pull basemax/salam:latest
```

This will download the latest version of the Salam image to your system.

### Step 2: Run the Image

After the image has been downloaded, you can run it interactively using this command:

```bash
docker run --rm -it basemax/salam
```

This will start a container and open an interactive session with the Salam programming language. The --rm flag ensures that the container is removed once you exit.

Or, if you'd prefer to build the Docker image locally instead of pulling it from Docker Hub, follow these steps:

### Step 1: Clone the Salam Repository

Start by cloning the Salam language repository:

```bash
git clone https://github.com/SalamLang/Salam
cd Salam
```

### Step 2: Build the Docker Image

Use the provided Dockerfile to build the Docker image. This will create a lightweight image containing the compiled Salam binary:

```bash
docker build -f docker/Dockerfile -t salam .
```

Verify the image is successfully built:

```bash
docker images salam
```

### Step 3: Run the Salam Container

To confirm the container works and view its contents, run the following:

```bash
docker run --rm -it salam ls -l /app
```

This will display the /app directory, which contains the `salam` binary.

### Step 4: Interact with Salam

Run the Salam binary directly in an interactive container session:

```bash
docker run --rm -it salam
```

You should see the following welcome message:

```
Welcome to Salam Programming Language!
Salam is the first Persian/Iranian computer scripting language.
...
```

### Step 5: Execute Salam Scripts

To execute a Salam script, use the following steps:

#### Option 1: Mount a Local Script

If you have a Salam script on your local machine, you can mount it into the container and execute it:

```bash
docker run --rm -it -v $(pwd):/scripts salam ./salam /scripts/your_script.salam
```

Here:
	- `$(pwd)` mounts the current directory to /scripts inside the container.
	- Replace `your_script.salam` with the actual filename of your Salam script.

#### Option 2: Inline Code Execution

You can also use the code command to execute Salam code directly:

```bash
docker run --rm -it salam ./salam code "صفحه: محتوا = «سلام دنیا از سلام» تمام"
```

### Step 6: Explore Salam Commands

You can access additional Salam commands inside the container:

#### Check the Salam Version:

```bash
docker run --rm -it salam ./salam version
```

#### Update Salam to the Latest Version:

```bash
docker run --rm -it salam ./salam update
```

#### Lint Salam Scripts:

Lint a File:

```bash
docker run --rm -it -v $(pwd):/scripts salam ./salam lint /scripts/your_script.salam /output
```

Lint Inline Code:

```bash
docker run --rm -it salam ./salam lint code "some salam code"
```

## Contributing

Want to contribute to Salam? Check out our [Contributing Guide](CONTRIBUTING.md) for more information.

All contributors are expected to adhere to our [Code of Conduct](CODE_OF_CONDUCT.md). Please make sure you are familiar with it before participating in this project.

Copyright 2024, Salam Language Team
