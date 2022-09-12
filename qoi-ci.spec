Name: qoi-ci
Version: 1.1.2
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

%package 2png
Summary: Command-line converter between QOI and PNG formats
BuildRequires: libpng-devel

%description 2png
Provides "png2qoi" command-line converter between the Quite OK Image (QOI)
and PNG formats.

%package gdk-pixbuf
Summary: GdkPixbuf loader for QOI image files
BuildRequires: gdk-pixbuf2-devel

%description gdk-pixbuf
GdkPixbuf loader for the Quite OK Image (QOI) format.

%package gimp
Summary: QOI plugin for GIMP
Requires: gimp
BuildRequires: gimp-devel

%description gimp
GIMP plugin for loading and exporting the Quite OK Image (QOI) format.

%global debug_package %{nil}

%prep
%setup -q

%build
make png2qoi libpixbufloader-qoi.so file-qoi

%install
rm -rf $RPM_BUILD_ROOT
make PREFIX=$RPM_BUILD_ROOT/%{_prefix} GDK_PIXBUF_LOADERS_DIR=$RPM_BUILD_ROOT%{_libdir}/gdk-pixbuf-2.0/2.10.0/loaders libdir=$RPM_BUILD_ROOT%{_libdir} BUILDING_PACKAGE=1 install-png2qoi install-gdk-pixbuf install-gimp

%clean
rm -rf $RPM_BUILD_ROOT

%files 2png
%defattr(-,root,root)
%{_bindir}/png2qoi

%files gdk-pixbuf
%defattr(-,root,root)
%{_libdir}/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-qoi.so
%{_datadir}/mime/packages/qoi-mime.xml
%{_datadir}/thumbnailers/qoi.thumbnailer

%files gimp
%defattr(-,root,root)
%{_libdir}/gimp/2.0/plug-ins/file-qoi/file-qoi

%changelog
* Wed Aug 3 2022 Piotr Fusik <fox@scene.pl>
- 1.1.2-1

* Mon Apr 18 2022 Piotr Fusik <fox@scene.pl>
- 1.1.1-1
- Initial packaging
