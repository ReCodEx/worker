%define name recodex-worker
%define version 1.0.0
%define unmangled_version 1.0.0
%define release 1

Summary: ReCodEx worker component
Name: %{name}
Version: %{version}
Release: %{release}
License: MIT
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}
Vendor: Petr Stefan <UNKNOWN>
Url: https://github.com/ReCodEx/worker
BuildRequires: systemd cmake
Requires(post): systemd
Requires(preun): systemd
Requires(postun): systemd

%description
Backend part of ReCodEx programmer testing solution.

%prep


%build
%cmake .
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
mkdir -p %{buildroot}/var/log/recodex

%clean


%post
%systemd_post 'recodex-worker@*.service'

%postun
%systemd_postun_with_restart 'recodex-worker@*.service'

%pre
getent group recodex >/dev/null || groupadd -r recodex
getent passwd recodex >/dev/null || useradd -r -g recodex -d %{_sysconfdir}/recodex -s /sbin/nologin -c "ReCodEx Code Examiner" recodex
exit 0

%preun
%systemd_preun 'recodex-worker@*.service'

%files
%defattr(-,root,root)
%dir %attr(-,recodex,recodex) %{_sysconfdir}/recodex/worker
%dir %attr(-,recodex,recodex) /var/log/recodex

%{_bindir}/recodex-worker
%{_bindir}/recodex-judge-normal
%{_bindir}/recodex-judge-filter
%{_bindir}/recodex-judge-shuffle
%config(noreplace) %attr(-,recodex,recodex) %{_sysconfdir}/recodex/worker/config-1.yml

#%{_unitdir}/recodex-worker@.service
/lib/systemd/system/recodex-worker@.service

%changelog

