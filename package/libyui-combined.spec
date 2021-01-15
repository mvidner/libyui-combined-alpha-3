#
# spec file for package libyui
#
# Copyright (c) 2014-2019 SUSE LINUX Products GmbH, Nuernberg, Germany.
# Copyright (c) 2020-2021 SUSE LLC, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Name:           libyui-combined

# DO NOT manually bump the version here; instead, use   rake version:bump
Version:        0.0.1
Release:        0

%define         so_version 14
%define         bin_name libyui%{so_version}
%define         ncurses_name libyui-ncurses
%define         ncurses_bin_name %{ncurses_name}%{so_version}

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++
BuildRequires:  pkg-config
BuildRequires:  boost-devel
BuildRequires:  libboost_test-devel
# for %{ncurses_bin_name}
BuildRequires:  ncurses-devel

Url:            http://github.com/libyui/
Summary:        GUI-abstraction library
License:        LGPL-2.1 or LGPL-3.0
Source:         %{name}-%{version}.tar.gz

%description
This is the user interface engine that provides the abstraction from
graphical user interfaces (Qt, Gtk) and text based user interfaces
(ncurses).

Originally developed for YaST, it can now be used independently of
YaST for generic (C++) applications. This package has very few
dependencies.

%package -n %{bin_name}

Provides:       yast2-libyui = 2.42.0
Obsoletes:      yast2-libyui < 2.42.0
Requires:       yui_backend = %{so_version}

Url:            http://github.com/libyui/
Summary:        Libyui - GUI-abstraction library
Group:          System/Libraries


%description -n %{bin_name}
This is the user interface engine that provides the abstraction from
graphical user interfaces (Qt, Gtk) and text based user interfaces
(ncurses).

Originally developed for YaST, it can now be used independently of
YaST for generic (C++) applications. This package has very few
dependencies.


%package -n libyui-devel

Requires:       glibc-devel
Requires:       libstdc++-devel
Requires:       boost-devel
Requires:       %{bin_name} = %{version}

Url:            http://github.com/libyui/
Summary:        Libyui header files and examples
Group:          Development/Languages/C and C++

%description -n libyui-devel
This is the user interface engine that provides the abstraction from
graphical user interfaces (Qt, Gtk) and text based user interfaces
(ncurses).

Originally developed for YaST, it can now be used independently of
YaST for generic (C++) applications. This package has very few
dependencies.

This package provides the C++ header files and some C++ examples.


%package -n %{ncurses_bin_name}

Requires:       glibc-locale
Requires:       libyui%{so_version}
Provides:       %{ncurses_name} = %{version}
Provides:       yast2-ncurses = 2.42.0
Obsoletes:      yast2-ncurses < 2.42.0
Provides:       yui_backend = %{so_version}

Url:            http://github.com/libyui/
Summary:        Libyui - Character Based User Interface

%description -n %{ncurses_bin_name}
This package contains the character based (ncurses) user interface
component for libYUI.



%package -n %{ncurses_name}-devel

Requires:       glibc-devel
Requires:       libstdc++-devel
Requires:       boost-devel
Requires:       ncurses-devel
Requires:       libyui-devel = %{version}
Requires:       %{ncurses_bin_name} = %{version}

Url:            http://github.com/libyui/
Summary:        Libyui-ncurses header files
Group:          Development/Languages/C and C++

%description -n %{ncurses_name}-devel
This package contains the character based (ncurses) user interface
component for libYUI.


This can be used independently of YaST for generic (C++) applications.
This package has very few dependencies.

%package -n %{ncurses_name}-tools

Url:            http://github.com/libyui/
Summary:        Libyui-ncurses tools
Group:          System/Libraries
# conflict with libyui-ncurses8, /usr/bin/libyui-terminal was originally there
Conflicts:      %{ncurses_name}8

Requires:       screen

%description -n %{ncurses_name}-tools
Character based (ncurses) user interface component for libYUI.

libyui-terminal - useful for testing on headless machines

%prep
%setup -q -n %{name}-%{version}


%build

pushd libyui
mkdir build
cd build

export CFLAGS="$RPM_OPT_FLAGS -DNDEBUG $(getconf LFS_CFLAGS)"
export CXXFLAGS="$RPM_OPT_FLAGS -DNDEBUG $(getconf LFS_CFLAGS)"

%if %{?_with_debug:1}%{!?_with_debug:0}
CMAKE_OPTS="-DCMAKE_BUILD_TYPE=RELWITHDEBINFO"
%else
CMAKE_OPTS="-DCMAKE_BUILD_TYPE=RELEASE"
%endif

cmake .. \
 -DDOC_DIR=%{_docdir} \
 -DLIB_DIR=%{_lib} \
 $CMAKE_OPTS

make %{?jobs:-j%jobs}
popd

pushd libyui-ncurses
mkdir build
cd build

cmake .. \
 -DDOC_DIR=%{_docdir} \
 -DLIB_DIR=%{_lib} \
 $CMAKE_OPTS

make %{?jobs:-j%jobs}
popd

%install

pushd libyui
cd build
make install DESTDIR="$RPM_BUILD_ROOT"
install -m0755 -d $RPM_BUILD_ROOT/%{_docdir}/%{bin_name}/
install -m0755 -d $RPM_BUILD_ROOT/%{_libdir}/yui
install -m0644 ../COPYING* $RPM_BUILD_ROOT/%{_docdir}/%{bin_name}/
popd

pushd libyui-ncurses
cd build
make install DESTDIR="$RPM_BUILD_ROOT"
install -m0755 -d $RPM_BUILD_ROOT/%{_docdir}/%{ncurses_bin_name}/
install -m0755 -d $RPM_BUILD_ROOT/%{_libdir}/yui
install -m0644 ../COPYING* $RPM_BUILD_ROOT/%{_docdir}/%{ncurses_bin_name}/
popd

%post -n %{bin_name} -p /sbin/ldconfig
%postun -n %{bin_name} -p /sbin/ldconfig
%post -n %{ncurses_bin_name} -p /sbin/ldconfig
%postun -n %{ncurses_bin_name} -p /sbin/ldconfig


%files -n %{bin_name}
%defattr(-,root,root)
%dir %{_libdir}/yui
%{_libdir}/libyui.so.*
%doc %dir %{_docdir}/%{bin_name}
%license %{_docdir}/%{bin_name}/COPYING*


%files -n libyui-devel
%defattr(-,root,root)
%dir %{_docdir}/%{bin_name}
%{_libdir}/libyui.so
%{_includedir}/yui/*.*
%dir %{_datadir}/libyui
%{_datadir}/libyui/buildtools
%doc %{_docdir}/%{bin_name}/examples
%{_libdir}/pkgconfig/libyui.pc
# %{_libdir}/cmake/%{name}

%files -n %{ncurses_bin_name}
%defattr(-,root,root)
%dir %{_libdir}/yui
%{_libdir}/yui/%{ncurses_name}.so.*
%doc %dir %{_docdir}/%{ncurses_bin_name}
%license %{_docdir}/%{ncurses_bin_name}/COPYING*

%files -n %{ncurses_name}-devel
%defattr(-,root,root)
%dir %{_docdir}/%{ncurses_bin_name}
%{_libdir}/yui/%{ncurses_name}.so
%{_prefix}/include/yui/ncurses


%files -n %{ncurses_name}-tools
%defattr(-,root,root)
%{_bindir}/libyui-terminal

%changelog
