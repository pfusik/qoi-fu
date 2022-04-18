Name: qoi-ci
Version: 1.1.1
Release: 1
Summary: Quite OK Image (QOI) encoder/decoder
License: MIT
Group: Applications/Multimedia
Source: https://github.com/pfusik/qoi-ci/archive/qoi-ci-%{version}/qoi-ci-%{version}.tar.gz
URL: https://github.com/pfusik/qoi-ci
BuildRequires: gcc
BuildRoot: %{_tmppath}/%{name}-root

%description
Quite OK Image (QOI) encoder/decoder.

%package gimp
Summary: RECOIL plugin for GIMP
Requires: gimp
BuildRequires: gimp-devel

%description gimp
GIMP plugin for loading and exporting the Quite OK Image (QOI) format.

%global debug_package %{nil}

%prep
%setup -q

%build
make file-qoi

%install
rm -rf $RPM_BUILD_ROOT
make PREFIX=$RPM_BUILD_ROOT/%{_prefix} libdir=$RPM_BUILD_ROOT%{_libdir} BUILDING_PACKAGE=1 install-gimp

%clean
rm -rf $RPM_BUILD_ROOT

%files gimp
%defattr(-,root,root)
%{_libdir}/gimp/2.0/plug-ins/file-qoi/file-qoi

%changelog
* Mon Apr 18 2022 Piotr Fusik <fox@scene.pl>
- 1.1.1-1
- Initial packaging
