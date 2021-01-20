#
# spec file for package libyui-qt
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


Name:           libyui-qt

# DO NOT manually bump the version here; instead, use   rake version:bump
Version:        2.56.4
Release:        0

%define         so_version 14
%define         libyui_devel_version libyui-devel >= 3.10.0
%define         qt_name libyui-qt
%define         qt_bin_name %{qt_name}%{so_version}

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++
BuildRequires:  pkg-config
BuildRequires:  fontconfig-devel

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(Qt5Svg)

BuildRequires:  %{libyui_devel_version}
Provides:       yui_backend = %{so_version}

Summary:        Libyui - Qt User Interface
License:        LGPL-2.1 or LGPL-3.0
Url:            http://github.com/libyui/
Source:         ${qt_name}-%{version}.tar.bz2

%description
This package contains the Qt user interface component for libYUI.


%package -n %{qt_bin_name}

Requires:       libyui%{so_version}
Provides:       ${qt_name} = %{version}
Provides:       yast2-qt = %{version}
Obsoletes:      yast2-qt < 2.51.0

Url:            http://github.com/libyui/
Summary:        Libyui - Qt User Interface
Group:          System/Libraries

%description -n %{qt_bin_name}
This package contains the Qt user interface component for libYUI.


%package -n %{qt_name}-devel

Requires:       %{libyui_devel_version}
Requires:       %{qt_bin_name} = %{version}
Requires:       fontconfig-devel

Url:            http://github.com/libyui/
Summary:        Libyui-qt header files
Group:          Development/Languages/C and C++

%description -n %{qt_name}-devel
This package contains the Qt user interface component for libYUI.

This can be used independently of YaST for generic (C++) applications.
This package has very few dependencies.


%prep
%setup -q -n ${qt_name}-%{version}

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
install -m0755 -d $RPM_BUILD_ROOT/%{_libdir}/yui
install -m0755 -d $RPM_BUILD_ROOT/%{_docdir}/%{qt_bin_name}/
install -m0644 ../COPYING* $RPM_BUILD_ROOT/%{_docdir}/%{qt_bin_name}/


%post -n %{qt_bin_name} -p /sbin/ldconfig
%postun -n %{qt_bin_name} -p /sbin/ldconfig


%files -n %{qt_bin_name}
%defattr(-,root,root)
%dir %{_libdir}/yui
%{_libdir}/yui/lib*.so.*
%doc %dir %{_docdir}/%{qt_bin_name}
%license %{_docdir}/%{qt_bin_name}/COPYING*


%files -n %{qt_name}-devel
%defattr(-,root,root)
%{_libdir}/yui/lib*.so
%{_includedir}/yui/qt

%changelog
