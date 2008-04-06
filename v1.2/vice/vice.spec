%define version 1.2
%define rel     1
%define prefix  /usr/local

Summary: VICE, the Versatile Commodore Emulator
Name: vice
Version: %version
Release: %rel
Copyright: GPL
Group: X11/Applications/Emulators
Source: ftp://ftp.funet.fi/pub/cbm/crossplatform/emulators/VICE/vice-%{version}.tar.gz
URL: http://www.cs.cmu.edu/%7Edsladic/vice/vice.html
Packager: Andreas Boose <boose@linux.rz.fh-hannover.de>
Prefix: %{prefix}
BuildRoot: /opt/rpm-build-root

%description
VICE is a set of accurate emulators for the Commodore 64, 128, VIC20,
PET and CBM-II 8-bit computers, all of which run under the X Window
System.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS -DNO_REGPARM" ./configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{prefix}
make prefix=$RPM_BUILD_ROOT%{prefix} install
strip $RPM_BUILD_ROOT%{prefix}/bin/*
gzip -9 $RPM_BUILD_ROOT%{prefix}/info/*

%clean
rm -rf $RPM_BUILD_ROOT

%files

%doc AUTHORS BUGS COPYING FEEDBACK INSTALL NEWS README TODO

%{prefix}/bin/x64
%{prefix}/bin/x128
%{prefix}/bin/xvic
%{prefix}/bin/xpet
%{prefix}/bin/xcbm2
%{prefix}/bin/c1541
%{prefix}/bin/petcat
%{prefix}/lib/vice
%{prefix}/man/man1/c1541.1
%{prefix}/man/man1/petcat.1
%{prefix}/man/man1/vice.1
%{prefix}/info/vice.info
%{prefix}/info/vice.info-1
%{prefix}/info/vice.info-2
%{prefix}/info/vice.info-3
%{prefix}/info/vice.info-4
%{prefix}/info/vice.info-5
