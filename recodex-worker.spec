%define name recodex-worker
%define short_name worker
%define version 1.9.1
%define unmangled_version 885c6bb4b3fa8e21400636f1bee1aefed19956e9
%define release 1

%define spdlog_name spdlog
%define spdlog_version 0.13.0

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
BuildRequires: systemd gcc-c++ cmake zeromq-devel cppzmq-devel yaml-cpp-devel libcurl-devel libarchive-devel boost-devel
Requires: systemd isolate

#Source0: %{name}-%{unmangled_version}.tar.gz
Source0: https://github.com/ReCodEx/%{short_name}/archive/%{unmangled_version}.tar.gz#/%{short_name}-%{unmangled_version}.tar.gz
Source1: https://github.com/gabime/%{spdlog_name}/archive/v%{spdlog_version}.tar.gz#/%{spdlog_name}-%{spdlog_version}.tar.gz

%global debug_package %{nil}

%description
Worker is a backend component of ReCodEx code examiner, an educational application for evaluating programming assignments. Worker is responsible for the evaluation of code submissions in a sandbox.

%prep
%setup -n %{short_name}-%{unmangled_version}
# Unpack spdlog to the right location
%setup -n %{short_name}-%{unmangled_version} -T -D -a 1
rmdir vendor/spdlog
mv -f %{spdlog_name}-%{spdlog_version} vendor/spdlog


%build
%cmake -DDISABLE_TESTS=true .
%cmake_build

%install
%cmake_install --prefix %{buildroot}
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
%{_bindir}/recodex-judge-passthrough
%{_bindir}/recodex-data-only-wrapper.sh
%{_bindir}/recodex-token-judge
%config(noreplace) %attr(-,recodex,recodex) %{_sysconfdir}/recodex/worker/config-1.yml

/lib/systemd/system/recodex-worker@.service

%changelog

