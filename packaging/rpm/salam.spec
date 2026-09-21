%global debug_package %{nil}
%global __strip /bin/true
%global __os_install_post %{nil}

Name:           salam
Version:        @VERSION@
Release:        1%{?dist}
Summary:        General-purpose systems programming language with a built-in DSL

License:        GPL-3.0-or-later
URL:            https://github.com/SalamLang/Salam
Source0:        %{url}/releases/download/v%{version}/salam-%{version}-linux-x86_64.tar.gz
Source1:        %{url}/releases/download/v%{version}/salam-%{version}-linux-aarch64.tar.gz

ExclusiveArch:  x86_64 aarch64

Requires:       glibc >= 2.38
Requires:       libgcc
Requires:       libxml2
Requires:       ncurses-libs

# The payload is a prebuilt release binary: Salam's compiler is written in
# Salam, so a from-source build needs a seed compiler that COPR cannot fetch
# during a network-isolated build.
AutoReqProv:    no

%description
Salam is a general-purpose and systems programming language with a built-in
domain-specific language for describing user interfaces. It ships an
interpreter, a C backend and an embedded LLVM backend that can cross-compile
and JIT with no other toolchain installed.

%prep
%ifarch x86_64
%setup -q -n salam-linux-x86_64 -T -b 0
%endif
%ifarch aarch64
%setup -q -n salam-linux-aarch64 -T -b 1
%endif

%install
install -Dm0755 salam %{buildroot}%{_libdir}/salam/salam

install -dm0755 %{buildroot}%{_libdir}/salam/std
cp -a std/. %{buildroot}%{_libdir}/salam/std/
find %{buildroot}%{_libdir}/salam/std -type d -exec chmod 0755 {} +
find %{buildroot}%{_libdir}/salam/std -type f -exec chmod 0644 {} +

if [ -d sysroots ]; then
    cp -a sysroots %{buildroot}%{_libdir}/salam/sysroots
fi

install -dm0755 %{buildroot}%{_bindir}
cat > %{buildroot}%{_bindir}/salam <<'EOF'
#!/bin/sh
SALAM_HOME=__LIBDIR__/salam
export SALAM_STD="${SALAM_STD:-$SALAM_HOME/std}"
exec "$SALAM_HOME/salam" "$@"
EOF
sed -i "s|__LIBDIR__|%{_libdir}|" %{buildroot}%{_bindir}/salam
chmod 0755 %{buildroot}%{_bindir}/salam

install -Dm0644 LICENSE %{buildroot}%{_datadir}/licenses/salam/LICENSE

%check
printf 'func main:\n    println "salam"\nend\n' > hello.salam
SALAM_STD=%{buildroot}%{_libdir}/salam/std %{buildroot}%{_libdir}/salam/salam exec hello.salam | grep -qx salam

%files
%license LICENSE
%{_bindir}/salam
%{_libdir}/salam/

%changelog
* @RPM_DATE@ Seyyed Ali Mohammadiyeh (Max Base) <MaxBaseCode@gmail.com> - @VERSION@-1
- Release @VERSION@
