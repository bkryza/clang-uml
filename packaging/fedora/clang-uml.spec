%define _unpackaged_files_terminate_build 0

Name:     clang-uml
Version:  %{?git_version}
Release:  1%{?dist}
Summary:  C++ UML diagram generator based on Clang
License:  ASL 2.0
URL:      https://github.com/bkryza/clang-uml
Source0:  clang-uml-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: git
BuildRequires: clang-devel
BuildRequires: clang-tools-extra
BuildRequires: ccache
BuildRequires: yaml-cpp-devel
BuildRequires: llvm-devel
BuildRequires: gdb

Requires: clang
Requires: yaml-cpp

Requires(post): info
Requires(preun): info

%description
clang-uml is an automatic C++ to UML class, sequence, package and include
diagram generator, driven by YAML configuration files. The main idea behind the
project is to easily maintain up-to-date diagrams within a code-base or
document legacy code. The configuration file or files for clang-uml define the
type and contents of each generated diagram. Currently the diagrams are
generated in PlantUML format.


%prep
%setup -q -n clang-uml-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
       -DCMAKE_CXX_FLAGS="-Wno-nonnull -Wno-stringop-overflow -Wno-dangling-reference -Wno-array-bounds" \
       -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON \
       -DCMAKE_INSTALL_PREFIX=%{_exec_prefix} \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DGIT_VERSION=%{version} \
       -DBUILD_TESTS=OFF

%cmake_build

%install
%cmake_install
mkdir -p %{buildroot}/%{_datadir}/bash-completion/completions
mkdir -p %{buildroot}/%{_datadir}/zsh/site-functions
cp -p packaging/autocomplete/clang-uml %{buildroot}/%{_datadir}/bash-completion/completions/clang-uml
cp -p packaging/autocomplete/_clang-uml %{buildroot}/%{_datadir}/zsh/site-functions/_clang-uml
rm -f %{buildroot}/%{_infodir}/dir

%post
/sbin/install-info %{_infodir}/%{name}.info %{_infodir}/dir || :

%preun
if [ $1 = 0 ] ; then
/sbin/install-info --delete %{_infodir}/%{name}.info %{_infodir}/dir || :
fi

%files
%{_bindir}/clang-uml
%{_datadir}/bash-completion/completions/clang-uml
%{_datadir}/zsh/site-functions/_clang-uml
%doc CHANGELOG.md README.md AUTHORS.md LICENSE.md
%license LICENSE.md

%changelog
* Sun Jan 01 2023 Bartek Kryza <bkryza@gmail.com>
- Initial version of the package for Fedora
