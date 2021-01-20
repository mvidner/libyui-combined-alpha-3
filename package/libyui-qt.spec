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

BuildRequires:  %{libyui_devel_version}
Provides:       yui_backend = %{so_version}

Summary:        Libyui - Qt User Interface
License:        LGPL-2.1 or LGPL-3.0
Url:            http://github.com/libyui/
Source:         ${qt_name}-%{version}.tar.bz2

%description
This package contains the Qt user interface component for libYUI.


%prep
%setup -q -n ${qt_name}-%{version}

%build


%install
cd build


%changelog
