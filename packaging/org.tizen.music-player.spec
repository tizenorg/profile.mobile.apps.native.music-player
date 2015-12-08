%define PKG_PREFIX org.tizen

Name:       org.tizen.music-player
Summary:    music player application
Version:    0.2.180
Release:    1
Group:      Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-media-player)
BuildRequires:  pkgconfig(capi-media-metadata-extractor)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(capi-telephony)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(libxml-2.0)
#BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(minicontrol-provider)
BuildRequires:  pkgconfig(capi-system-media-key)
BuildRequires:  pkgconfig(capi-appfw-preference)
BuildRequires:  pkgconfig(capi-content-media-content)
BuildRequires:  pkgconfig(capi-content-mime-type)
BuildRequires:  pkgconfig(capi-network-bluetooth)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(storage)
BuildRequires:  pkgconfig(capi-message-port)
#for service
BuildRequires:  pkgconfig(capi-appfw-service-application)
BuildRequires:  cmake
BuildRequires:  prelink
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
#BuildRequires:  hash-signer
#START_PUBLIC_REMOVED_STRING
BuildRequires:  pkgconfig(capi-web-url-download)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(libcore-context-manager)
BuildRequires:  pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-network-wifi)
BuildRequires:  pkgconfig(capi-network-wifi-direct)
BuildRequires:  pkgconfig(capi-system-device)
#widget relevant
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(widget)
BuildRequires:  pkgconfig(widget_service)
BuildRequires:  pkgconfig(widget_provider_app)
BuildRequires:  pkgconfig(widget_provider)
BuildRequires:  pkgconfig(capi-appfw-widget-application)

%if "%{?sec_build_project_type}" != "lite"
#Build requires only for highend.
%endif

%if "%{?sec_build_project_type}" == "lite"
#Build requires for lite version.
%endif

#END_START_PUBLIC_REMOVED_STRING
Requires:  media-server
Requires(post): coreutils
#Requires(post): signing-client

%description
music player application.

%package -n %{PKG_PREFIX}.sound-player
Summary:    Sound player
Group:      Applications
Requires:   %{name} = %{version}-%{release}

%description -n %{PKG_PREFIX}.sound-player
Description: sound player application

#START_PUBLIC_REMOVED_STRING
%package -n %{PKG_PREFIX}.music-chooser
Summary:    music-chooser chooser
Group:      Applications
#Requires:   %{name} = %{version}-%{release}

%description -n %{PKG_PREFIX}.music-chooser
Description: music-chooser chooser
#END_START_PUBLIC_REMOVED_STRING

%prep
%setup -q

%define DESKTOP_DIR %{TZ_SYS_SHARE}
%define INSTALL_DIR	 %{TZ_SYS_RO_APP}

%define PKG_NAME %{name}
%define PREFIX %{INSTALL_DIR}/%{PKG_NAME}

%define MC_PKG_NAME %{PKG_PREFIX}.music-chooser
%define MC_PREFIX %{INSTALL_DIR}/%{MC_PKG_NAME}
%define DATA_PREFIX %{INSTALL_DIR}/%{PKG_NAME}

%define SP_PKG_NAME %{PKG_PREFIX}.sound-player
%define SP_PREFIX %{INSTALL_DIR}/%{SP_PKG_NAME}
%define SP_DATA_PREFIX %{INSTALL_DIR}/%{SP_PKG_NAME}

%define _app_icon_dir %{TZ_SYS_RO_ICONS}/default/small/
%define _app_share_packages_dir %{TZ_SYS_RO_PACKAGES}
%define _app_license_dir %{TZ_SYS_SHARE}/license

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
cmake . -DMC_PREFIX="%{MC_PREFIX}" \
	-DINSTALL_DIR="%{INSTALL_DIR}" \
	-DCMAKE_INSTALL_PREFIX="%{PREFIX}" \
	-DCMAKE_DESKTOP_ICON_DIR="%{_app_icon_dir}" \
	-DDESKTOP_DIR="%{DESKTOP_DIR}" \
	-DPKG_NAME="%{PKG_NAME}" \
	-DSP_PKG_NAME="%{SP_PKG_NAME}" \
	-DDATA_PREFIX="%{DATA_PREFIX}" \
	-DSP_DATA_PREFIX="%{SP_DATA_PREFIX}" \
%if 0%{?sec_product_feature_msg_disable_mms}
	-DCMAKE_DISABLE_FEATURE_MMS=YES \
%endif
%if 0%{?sec_product_feature_cloud_enable_content_sync_dropbox}
	-DCMAKE_ENABLE_FEATURE_DROPBOX=YES \
%endif
%if 0%{?feature_debug_mode}
	-DCMAKE_DEBUG_MODE=YES \
%endif
%if 0%{?sec_product_feature_multiwindow}
	-DCMAKE_ENABLE_MULTIWINDOW=YES \
%endif


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{_app_license_dir}
cp LICENSE %{buildroot}/%{_app_license_dir}/%{name}

%make_install
%define tizen_sign 1
%define tizen_sign_base %{PREFIX}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

#execstack -c %{buildroot}%{PREFIX}/bin/music-player

%pre
if [ -n "`env|grep SBOX`" ]; then
        echo "postinst: sbox installation"
else
        RESULT=` /usr/bin/killall music-player`
        if [ -n "$RESULT" ]; then
                echo "preinst: kill current music-player app"
        fi
        RESULT=`/usr/bin/killall sound-player`
        if [ -n "$RESULT" ]; then
                echo "preinst: kill current sound-player app"
        fi
fi

%post
%files
%manifest %{name}.manifest
%{_app_share_packages_dir}/%{name}.xml
%{_app_icon_dir}/%{name}.png
%{_app_icon_dir}/preview_music_4x2.png
%{PREFIX}/bin/*
%{PREFIX}/res/locale/*/LC_MESSAGES/*.mo
%{PREFIX}/res/images/*
%{PREFIX}/res/edje/*.edj
%{PREFIX}/shared/res/*
%attr(-,app,app) %dir %{DATA_PREFIX}/data
%{PREFIX}/lib/*.so*
%{_app_license_dir}/%{name}
%attr(-,app,app) %dir %{DATA_PREFIX}/shared/data

%{_app_icon_dir}%{SP_PKG_NAME}.png
%{SP_PREFIX}/bin/sound-player
%attr(-,app,app) %dir %{SP_DATA_PREFIX}/data
%attr(-,app,app) %dir %{SP_DATA_PREFIX}/shared/data

#%{PREFIX}/author-signature.xml
#%{PREFIX}/signature1.xml

#START_PUBLIC_REMOVED_STRING

%files -n org.tizen.music-chooser
%manifest org.tizen.music-chooser.manifest
%{DESKTOP_DIR}/packages/org.tizen.music-chooser.xml
%{MC_PREFIX}/res/locale/*/LC_MESSAGES/*.mo
%defattr(-,root,root,-)
#%attr(-,app,app) %dir %{MC_PREFIX}/res/etc/music-chooser
%{MC_PREFIX}/bin/*
%{MC_PREFIX}/shared/trusted/music-chooser.edj
%{TZ_SYS_RO_ICONS}/default/small/music-chooser.png

#END_START_PUBLIC_REMOVED_STRING

%{TZ_SYS_RW_APP}/org.tizen.music-player/shared/res/*
