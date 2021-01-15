#
# spec file for package libyui-ncurses
#
# Copyright (c) 2015-2019 SUSE LINUX GmbH, Nuernberg, Germany.
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


Name:           libyui-ncurses
# DO NOT manually bump the version here; instead, use   rake version:bump
Version:        2.57.2
Release:        0

%define         so_version 14
%define         bin_name %{name}%{so_version}

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++
BuildRequires:  boost-devel
BuildRequires:  ncurses-devel

# YLabel::setAutoWrap()
%define         libyui_devel_version libyui-devel >= 3.10.0
BuildRequires:  %{libyui_devel_version}

Url:            http://github.com/libyui/
Summary:        Libyui - Character Based User Interface
License:        LGPL-2.1 or LGPL-3.0
Source:         %{name}-%{version}.tar.bz2

%description
This package contains the character based (ncurses) user interface
component for libYUI.


%package -n %{bin_name}

Requires:       glibc-locale
Requires:       libyui%{so_version}
Provides:       %{name} = %{version}
Provides:       yast2-ncurses = 2.42.0
Obsoletes:      yast2-ncurses < 2.42.0
Provides:       yui_backend = %{so_version}

Url:            http://github.com/libyui/
Summary:        Libyui - Character Based User Interface

%description -n %{bin_name}
This package contains the character based (ncurses) user interface
component for libYUI.



%package devel

Requires:       glibc-devel
Requires:       libstdc++-devel
Requires:       boost-devel
Requires:       ncurses-devel
Requires:       %{libyui_devel_version}
Requires:       %{bin_name} = %{version}

Url:            http://github.com/libyui/
Summary:        Libyui-ncurses header files
Group:          Development/Languages/C and C++

%description devel
This package contains the character based (ncurses) user interface
component for libYUI.


This can be used independently of YaST for generic (C++) applications.
This package has very few dependencies.

%package tools

Url:            http://github.com/libyui/
Summary:        Libyui-ncurses tools
Group:          System/Libraries
# conflict with libyui-ncurses8, /usr/bin/libyui-terminal was originally there
Conflicts:      %{name}8

Requires:       screen

%description tools
Character based (ncurses) user interface component for libYUI.

libyui-terminal - useful for testing on headless machines

%prep
%setup -q -n %{name}-%{version}

%build

export CFLAGS="$RPM_OPT_FLAGS -DNDEBUG"
export CXXFLAGS="$RPM_OPT_FLAGS -DNDEBUG"

mkdir build
cd build

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


%install
cd build
make install DESTDIR="$RPM_BUILD_ROOT"
install -m0755 -d $RPM_BUILD_ROOT/%{_docdir}/%{bin_name}/
install -m0755 -d $RPM_BUILD_ROOT/%{_libdir}/yui
install -m0644 ../COPYING* $RPM_BUILD_ROOT/%{_docdir}/%{bin_name}/


%post -n %{bin_name} -p /sbin/ldconfig
%postun -n %{bin_name} -p /sbin/ldconfig

%files -n %{bin_name}
%defattr(-,root,root)
%dir %{_libdir}/yui
%{_libdir}/yui/lib*.so.*
%doc %dir %{_docdir}/%{bin_name}
%license %{_docdir}/%{bin_name}/COPYING*

%files devel
%defattr(-,root,root)
%dir %{_docdir}/%{bin_name}
%{_libdir}/yui/lib*.so
%{_prefix}/include/yui


%files tools
%defattr(-,root,root)
%{_bindir}/libyui-terminal

%changelog
