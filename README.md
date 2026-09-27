# Nougat

Nougat is a lightweight package manager for CLI tools on Windows and Unix-like operating systems.

It installs tools from local archives or URLs and makes their executables available through a shim system. Packages and their configuration remain isolated inside the Nougat installation directory.

## Motivation

Nougat was originally created as a personal project to make it easier to install and manage development tools.

On Windows, setting up tools often involves manually downloading archives or installers, configuring environment variables, and keeping track of where each tool is installed. On Unix-like systems, tools are often easier to centralize through system package managers, but this can also make it difficult to keep project-specific tools isolated from the rest of the system.

Nougat was created to provide a simple way to install and manage CLI tools while keeping each package isolated and self-contained. It was initially developed for personal use, but was later opened as a public project.

## Installation

Nougat can be installed in any directory.

Set `NOUGAT_HOME` to the Nougat installation directory and add both `Binaries` and `System` to `PATH`.

The installation directory can be anywhere. The only requirement is that `NOUGAT_HOME` points to the Nougat directory and that both `Binaries` and `System` are available through `PATH`.

Use `nougat --help` to see the available commands and options.

Shims also provide internal commands through the `--nougat:` prefix. Use `--nougat:help` to see the available internal commands.

## How It Works

Nougat keeps installed packages inside `Packages/<id>/`.

When a package provides executables, Nougat creates shims for them in `Binaries`. A shim locates the real executable inside the package, loads the package-specific environment, and forwards the command-line arguments to it.

Package-specific environment variables are stored in `Environment/<id>.db`.

This keeps each package self-contained while allowing its executables to be used directly through `PATH`.

## Building

Building Nougat requires:

* Clang or GCC with full C++20 support
* Forge Build Tool
* `stdext`, distributed with Forge
* `libcurl`
* OpenSSL
* zlib

The `nougat-miniz` package is included in the Nougat source tree. It contains the source code of [miniz](https://github.com/richgel999/miniz), a small and lightweight ZIP compression and extraction library.

The other dependencies are external and must be available when building the project.

## Credits

Nougat includes source code from [miniz](https://github.com/richgel999/miniz). The source files are located in `packages/nougat-miniz`, along with the corresponding license.

## License

Nougat is licensed under the BSD 3-Clause License. See [license.txt](license.txt) for the full license text.
