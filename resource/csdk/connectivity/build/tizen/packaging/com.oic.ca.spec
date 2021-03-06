%define PREFIX /usr/apps/com.oic.ca
%define ROOTDIR  %{_builddir}/%{name}-%{version}
%define DEST_INC_DIR  %{buildroot}/%{_includedir}/OICHeaders
%define DEST_LIB_DIR  %{buildroot}/%{_libdir}

Name: com-oic-ca
Version:    0.1
Release:    1
Summary: Tizen oicca application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
License: Apache-2.0
Group: Applications/OIC
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: boost-devel
BuildRequires: boost-thread
BuildRequires: boost-system
BuildRequires: boost-filesystem
BuildRequires: scons


%description
SLP oicca application

%prep

%setup -q

%build

echo %{ROOTDIR}

scons TARGET_OS=tizen -c
scons TARGET_OS=tizen TARGET_TRANSPORT=%{TARGET_TRANSPORT} RELEASE=%{RELEASE}

%install
mkdir -p %{DEST_INC_DIR}
mkdir -p %{DEST_LIB_DIR}/pkgconfig

cp -f %{ROOTDIR}/con/src/libconnectivity_abstraction.a %{buildroot}/%{_libdir}
cp -f %{ROOTDIR}/con/lib/libcoap-4.1.1/libcoap.a %{buildroot}/%{_libdir}
cp -rf %{ROOTDIR}/con/api/cacommon.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/inc/caadapterinterface.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/common/inc/uthreadpool.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/inc/cawifiadapter.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/inc/caethernetadapter.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/inc/caedradapter.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/inc/caleadapter.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/con/api/cainterface.h* %{DEST_INC_DIR}/
cp -rf %{ROOTDIR}/com.oic.ca.pc %{DEST_LIB_DIR}/pkgconfig/


%files
%manifest com.oic.ca.manifest
%defattr(-,root,root,-)
%{_libdir}/lib*.a*
%{_includedir}/OICHeaders/*
%{_libdir}/pkgconfig/*.pc

