%{expand:%%global gd_head %(git describe --tags HEAD)}
%{expand:%%global gd_rel_delta %(git describe --abbrev=4 --always --tags --long --match 'v[[:digit:]][[:alnum:].]*[[:alnum:]]' --dirty=.1 | cut -d- -f 2- | tr '-' '.')}
%global rel_pre_post %(echo "%{gd_head}" | grep -Eq '^v%{version}' >&/dev/null && echo 1. || echo 0.)
%{!?rel:%global rel %{rel_pre_post}%{gd_rel_delta}%{?dist}}

%global name    libast
%global version 0.8.1
%global release %{rel}


Summary: Library of Assorted Spiffy Things
Name: %{name}
Version: %{version}
Release: %{release}
Group: System Environment/Libraries
License: BSD
URL: http://www.eterm.org/
Source: %{name}-%{version}.tar.gz
#BuildSuggests: pcre-devel xorg-x11-devel imlib2-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root


%description
LibAST is the Library of Assorted Spiffy Things.  It contains various
handy routines and drop-in substitutes for some good-but-non-portable
functions.  It currently has a built-in memory tracking subsystem as
well as some debugging aids and other similar tools.

It's not documented yet, mostly because it's not finished.  Hence the
version number that begins with 0.


%prep
%setup -q


%build
%{configure} --prefix=%{_prefix} --bindir=%{_bindir} --libdir=%{_libdir} \
             --includedir=%{_includedir} --datadir=%{_datadir} %{?acflags}
%{__make} %{?_smp_mflags} %{?mflags}


%install
%{__make} install DESTDIR=%{buildroot} %{?mflags_install}


%post
test -x /sbin/ldconfig && /sbin/ldconfig || :


%postun
test -x /sbin/ldconfig && /sbin/ldconfig || :


%clean
test "%{buildroot}" != "/" && rm -rf %{buildroot}


%files
%defattr(-, root, root, 0755)
%doc ChangeLog DESIGN LICENSE README
%{_bindir}/*
%{_libdir}/*
%{_includedir}/*
%{_datadir}/*


%changelog
