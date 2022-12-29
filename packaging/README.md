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

## Anaconda

```bash
docker run --rm -v $PWD:$PWD continuum/miniconda3 bash
conda install conda-build
cd packaging
make conda
```