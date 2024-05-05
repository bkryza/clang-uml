# Building releases

* Update CHANGELOG.md
* Tag the release commit, e.g. ```git tag 0.1.0```

## Ubuntu

```bash
cd packaging
make DIST=focal deb
make DIST=jammy deb
make DIST=mantic deb
make DIST=noble deb

cd _BUILD/ubuntu/focal
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/jammy
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/mantic
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/noble
dput ppa:bkryza/clang-uml *.changes
```

## Fedora

```bash
cd clang-uml
make fedora/38
make fedora/39
make fedora/40
find packaging/_BUILD/fedora
```

## Anaconda

```bash
docker run --rm -v $PWD:$PWD -it continuumio/miniconda3 bash
conda install conda-build make anaconda-client
cd packaging
git config --global --add safe.directory /home/bartek/devel/clang-uml
make CONDA_TOKEN=<TOKEN> conda
```

## Windows

First build release configuration using `cmake` and `msbuild` according
to the [documentation](../docs/installation.md#visual-studio-native-build).

```bash
cd packaging
.\make_installer.ps1
ls .\_BUILD\windows\clang-uml-0.5.2-win64.exe
```