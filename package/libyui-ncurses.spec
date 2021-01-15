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


Url:            http://github.com/libyui/
Summary:        Libyui - Character Based User Interface
License:        LGPL-2.1 or LGPL-3.0
Source:         %{ncurses_name}-%{version}.tar.bz2

%description
This package contains the character based (ncurses) user interface
component for libYUI.


%prep
%setup -q -n %{ncurses_name}-%{version}

%build



%install




%changelog
