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

%description
Backend part of ReCodEx programmer testing solution.

%prep
# Create 'recodex' user if not exist
id -u recodex > /dev/null 2>&1
if [ $? -eq 1 ]
then
	useradd --system --shell /sbin/nologin recodex
fi

%build
%cmake .
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
mkdir -p %{buildroot}/var/log/recodex

%clean

%post


%postun


%pre


%preun


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

