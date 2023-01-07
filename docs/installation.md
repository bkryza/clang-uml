# Installation

### Distribution packages

#### Ubuntu

```bash
# Currently supported Ubuntu versions are Focal, Jammy and Kinetic
sudo add-apt-repository ppa:bkryza/clang-uml
sudo apt update
sudo apt install clang-uml
```

#### Fedora

```bash
# Fedora 36
wget https://github.com/bkryza/clang-uml/releases/download/0.3.0/clang-uml-0.3.0-1.fc36.x86_64.rpm
sudo dnf install ./clang-uml-0.3.0-1.fc36.x86_64.rpm

# Fedora 37
wget https://github.com/bkryza/clang-uml/releases/download/0.3.0/clang-uml-0.3.0-1.fc37.x86_64.rpm
sudo dnf install ./clang-uml-0.3.0-1.fc37.x86_64.rpm
```

#### Conda

```bash
conda config --add channels conda-forge
conda config --set channel_priority strict
conda install -c bkryza/label/clang-uml clang-uml
```

### Building from source

#### Linux
First make sure that you have the following dependencies installed:

```bash
# Ubuntu (clang version will vary depending on Ubuntu version)
apt install ccache cmake libyaml-cpp-dev clang-12 libclang-12-dev libclang-cpp12-dev
```

Then proceed with building the sources:

```bash
git clone https://github.com/bkryza/clang-uml
cd clang-uml
# Please note that top level Makefile is just a convenience wrapper for CMake
make release
release/clang-uml --help

# To build using a specific installed version of LLVM use:
LLVM_VERSION=14 make release

# Optionally
make install
# or
export PATH=$PATH:$PWD/release
```

#### macos

```bash
brew install ccache cmake llvm yaml-cpp

export CC=/usr/local/opt/llvm/bin/clang
export CCX=/usr/local/opt/llvm/bin/clang++
LLVM_VERSION=14 make release
```

#### Windows

##### Visual Studio native build

These steps present how to build and use `clang-uml` natively using Visual Studio only.

First, install the following dependencies manually:

* [Python 3](https://www.python.org/downloads/windows/)
* [Git](https://git-scm.com/download/win)
* [CMake](https://cmake.org/download/)
* [Visual Studio](https://visualstudio.microsoft.com/vs/community/)
* [Clang Power Tools](https://clangpowertools.com/) - this can be installed from VS Extension manager

Create installation directory for `clang-uml` and its dependencies:
```bash
# This is where our working clang-uml binary will be located
# If you change this path, adapt all consecutive steps
mkdir C:\clang-uml
# This directory will be removed after build
mkdir C:\clang-uml-tmp
cd C:\clang-uml-tmp
```

Build and install yaml-cpp:

```bash
git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
git checkout yaml-cpp-0.7.0
cd ..
cmake -S .\yaml-cpp\ -B .\yaml-cpp-build\ -DCMAKE_INSTALL_PREFIX="C:\clang-uml" -Thost=x64
cd yaml-cpp-build
msbuild .\INSTALL.sln -maxcpucount /p:Configuration=Release
```

Build and install LLVM:

```bash 
pip install psutil
git clone --branch llvmorg-15.0.6 --depth 1 https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-15.0.6.zip
cmake -S .\llvm\llvm -B llvm-build -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX="C:\clang-uml" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -Thost=x64
msbuild .\INSTALL.sln  -maxcpucount /p:Configuration=Release
```

Build and install clang-uml:

```bash
git clone https://github.com/bkryza/clang-uml
cmake -S .\clang-uml\ -B .\clang-uml-build\ -DCMAKE_PREFIX_PATH="C:\clang-uml" -Thost=x64
```
