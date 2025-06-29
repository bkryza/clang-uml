# Building releases

* Update CHANGELOG.md
* Tag the release commit, e.g. ```git tag 0.1.0```

## Ubuntu

```bash
cd packaging
make DIST=focal deb
make DIST=jammy deb
make DIST=noble deb
make DIST=oracular deb
make DIST=plucky deb

cd _BUILD/ubuntu/focal
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/jammy
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/noble
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/oracular
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/plucky
dput ppa:bkryza/clang-uml *.changes
```

## Debian

```bash
docker run --rm -v $PWD:$PWD -it debian:bookworm bash
apt update
apt install debhelper python3 python3-pip git make ccache pkg-config gcc g++ gdb cmake libyaml-cpp-dev llvm-19 llvm-19-dev clang-19 clang-tools-19 libclang-19-dev libclang-cpp19-dev libmlir-19-dev mlir-19-tools bash-completion dh-sequence-bash-completion libdw-dev libunwind-dev
pip3 install --break-system-packages git-archive-all
git config --global --add safe.directory /home/bartek/devel/clang-uml
cd packaging
make OS=debian DIST=bookworm debian
```

## Fedora

```bash
cd clang-uml
make fedora/41
make fedora/42
find packaging/_BUILD/fedora
```

## Anaconda

```bash
docker run --rm -v $PWD:$PWD -it continuumio/miniconda3 bash
conda install conda-build make anaconda-client
cd /home/bartek/devel/clang-uml/packaging
git config --global --add safe.directory /home/bartek/devel/clang-uml
make CONDA_TOKEN=<TOKEN> conda
```

## Windows

First build release configuration using `cmake` and `msbuild` according
to the [documentation](../docs/installation.md#visual-studio-native-build).

```bash
cd packaging
.\make_installer.ps1
ls .\_BUILD\windows\clang-uml-0.6.2-win64.exe
```