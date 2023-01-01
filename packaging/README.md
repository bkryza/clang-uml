# Building releases

* Update CHANGELOG.md
* Tag the release commit, e.g. ```git tag 0.1.0```

## Ubuntu

```bash
cd packaging
make DIST=focal deb
make DIST=jammy deb
make DIST=kinetic deb

cd _BUILD/ubuntu/focal
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/jammy
dput ppa:bkryza/clang-uml *.changes

cd _BUILD/ubuntu/kinetic
dput ppa:bkryza/clang-uml *.changes

```

## Fedora

```bash
docker run -v $PWD:$PWD fedora:37 bash
dnf install fedora-packager rpmdevtools gcc cmake git clang-devel clang-tools-extra ccache yaml-cpp llvm-devel wget
rpmdev-setuptree
cd /root/rpmbuild/SOURCES
wget https://github.com/bkryza/clang-uml/archive/refs/heads/v0.3.0.zip
cd /root/rpmbuild/SPECS/
wget https://raw.githubusercontent.com/bkryza/clang-uml/v0.3.0/packaging/fedora/clang-uml.spec
rpmbuild -ba clang-uml.spec
```

## Anaconda

```bash
docker run --rm -v $PWD:$PWD continuum/miniconda3 bash
conda install conda-build
cd packaging
make conda
```