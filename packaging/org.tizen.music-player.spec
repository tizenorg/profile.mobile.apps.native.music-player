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

%define DESKTOP_DIR /usr/share
%define INSTALL_DIR	/usr/apps
%define DATA_DIR	/opt/usr/apps

%define PKG_NAME %{name}
%define PREFIX %{INSTALL_DIR}/%{PKG_NAME}

%define MC_PKG_NAME %{PKG_PREFIX}.music-chooser
%define MC_PREFIX %{INSTALL_DIR}/%{MC_PKG_NAME}
%define DATA_PREFIX %{DATA_DIR}/%{PKG_NAME}

%define SP_PKG_NAME %{PKG_PREFIX}.sound-player
%define SP_PREFIX %{INSTALL_DIR}/%{SP_PKG_NAME}
%define SP_DATA_PREFIX %{DATA_DIR}/%{SP_PKG_NAME}

%define _storage_phone /opt/usr/media
%define _storage_sdcard /opt/storage/sdcard

%define _log_dump_script_dir /opt/etc/dump.d/module.d

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
cmake . -DMC_PREFIX="%{MC_PREFIX}" \
	-DINSTALL_DIR="%{INSTALL_DIR}" \
	-DCMAKE_INSTALL_PREFIX="%{PREFIX}" \
	-DCMAKE_DESKTOP_ICON_DIR="%{DESKTOP_DIR}/icons/default/small" \
	-DDESKTOP_DIR="%{DESKTOP_DIR}" \
	-DPKG_NAME="%{PKG_NAME}" \
	-DSP_PKG_NAME="%{SP_PKG_NAME}" \
	-DDATA_PREFIX="%{DATA_PREFIX}" \
	-DSP_DATA_PREFIX="%{SP_DATA_PREFIX}" \
	-DCMAKE_LOG_DUMP_SCRIPT_DIR="%{_log_dump_script_dir}" \
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
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
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
#/usr/bin/vconftool set -t int memory/music/state 0 -i -f -g 5000 -s org.tizen.music-player

#/usr/bin/vconftool set -t string memory/private/org.tizen.music-player/pos "00:00" -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t double memory/private/org.tizen.music-player/progress_pos 0.0 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t double memory/private/org.tizen.music-player/position_changed 0.0 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t int memory/private/org.tizen.music-player/player_state 0 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t int memory/private/org.tizen.music-player/pd_genlist_clear 0 -i -f -g 5000 -s org.tizen.music-player

#/usr/bin/vconftool set -t bool db/private/org.tizen.music-player/shuffle 0 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t int db/private/org.tizen.music-player/repeat 1 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t int db/private/org.tizen.music-player/square_axis_val 0 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t int db/private/org.tizen.music-player/playlist 15 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t bool db/private/org.tizen.music-player/motion_asked 0 -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t bool db/private/org.tizen.music-player/square_asked 0 -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t bool db/private/org.tizen.music-player/smart_volume 0 -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t bool db/private/org.tizen.music-player/show_lyrics 1 -f -g 5000 -s org.tizen.music-player

#/usr/bin/vconftool set -t string db/private/org.tizen.music-player/tabs_order "1234567" -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t int db/private/org.tizen.music-player/tabs_select 63 -i -f -g 5000 -s org.tizen.music-player
#/usr/bin/vconftool set -t string db/private/org.tizen.music-player/playlist_order "1234" -i -f -g 5000 -s org.tizen.music-player

#/usr/bin/vconftool set -t int memory/private/org.tizen.music-player/playing_pid 0 -i -f -g 5000 -s org.tizen.music-player

#vconftool set -t int db/private/org.tizen.music-player/se_change 1 -g 5000 -s org.tizen.music-player
#vconftool set -t bool db/private/org.tizen.music-player/menu_change 1 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_1 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_2 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_3 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_4 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_5 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_6 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_7 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/eq_custom_8 0.5 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/user_audio_effect_3d 0.0 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/user_audio_effect_bass 0.0 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/user_audio_effect_room 0.0 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/user_audio_effect_reverb 0.0 -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/user_audio_effect_clarity 0.0 -g 5000 -s org.tizen.music-player
#vconftool set -t int memory/private/org.tizen.music-player/auto_off_time_val 0 -i -f -g 5000 -s org.tizen.music-player
#vconftool set -t int memory/private/org.tizen.music-player/auto_off_custom_time 0 -i -f -g 5000 -s org.tizen.music-player
#vconftool set -t int memory/private/org.tizen.music-player/auto_off_type_val 0 -i -f -g 5000 -s org.tizen.music-player
#vconftool set -t int memory/private/org.tizen.music-player/sa_user_change 0 -i -f -g 5000 -s org.tizen.music-player
#vconftool set -t double db/private/org.tizen.music-player/playspeed 1.0 -i -f -g 5000 -s org.tizen.music-player

#vconftool set -t bool db/private/org.tizen.music-player/personal_no_ask_again 0  -i -f -g 5000 -s org.tizen.music-player

#/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform  %{PREFIX}

touch /opt/usr/apps/org.tizen.music-player/shared/data/MusicPlayStatus.ini
chown 5000:5000 /opt/usr/apps/org.tizen.music-player/shared/data/MusicPlayStatus.ini

touch /opt/usr/apps/org.tizen.music-player/shared/data/NowPlayingId.ini
chown 5000:5000 /opt/usr/apps/org.tizen.music-player/shared/data/NowPlayingId.ini

%files
%manifest %{name}.manifest
%{DESKTOP_DIR}/packages/%{name}.xml
%{DESKTOP_DIR}/icons/default/small/%{name}.png
%{DESKTOP_DIR}/icons/default/small/preview_music_4x2.png
%{PREFIX}/bin/*
%{PREFIX}/res/locale/*/LC_MESSAGES/*.mo
%{PREFIX}/res/images/*
%{PREFIX}/res/edje/*.edj
%{PREFIX}/shared/res/*
%attr(-,app,app) %dir %{DATA_PREFIX}/data
%{PREFIX}/lib/*.so*
#/opt/usr/share/sstream-plugins/*.so
/usr/share/license/%{name}
%attr(-,app,app) %dir %{DATA_PREFIX}/shared/data
%attr(0744,root,root) %{_log_dump_script_dir}/dump_%{name}.sh

%{DESKTOP_DIR}/icons/default/small/%{SP_PKG_NAME}.png
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
/usr/share/icons/default/small/music-chooser.png

#END_START_PUBLIC_REMOVED_STRING

/usr/apps/org.tizen.music-player/shared/res/*
