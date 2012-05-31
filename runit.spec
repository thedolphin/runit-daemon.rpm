Summary: Unix services supervisor
Name: runit
Version: 2.1.1
Release: 1
License: BSD-alike
Group: Utilities/System
Source0: http://smarden.org/runit/runit-%{version}.tar.gz
Source1: runsvdir
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
URL: http://smarden.org/runit/
Packager: Alexander Rumyantsev <alexander.rumyantsev@wikimart.ru>
Requires(post): /sbin/chkconfig
Patch0: runit_runit-start.patch

%description
The successor of daemontools, runit intended to manage services

%prep
%setup -n admin/runit-%{version}
%patch0 -p2

%build
package/compile

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_localstatedir}/runit
mkdir -p %{buildroot}%{_localstatedir}/log/runit
mkdir -p %{buildroot}%{_initrddir}
while read command; do
    %{__install} command/$command %{buildroot}%{_bindir}/$command
done < package/commands
%{__install} %{SOURCE1} %{buildroot}%{_initrddir}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_initrddir}/runsvdir
%{_bindir}/*
%dir %{_localstatedir}/runit
%dir %{_localstatedir}/log/runit

%post
/sbin/chkconfig runsvdir on

%changelog

* Thu May 31 2012 Alexander Rumyantsev <alexander.rumyantsev@wikimart.ru>
- Initial release
