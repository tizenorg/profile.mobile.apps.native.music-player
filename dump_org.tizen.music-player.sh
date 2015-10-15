#!/bin/sh

#--------------------------------------
#   com.samsung.music-player
#--------------------------------------


PKG_NAME=org.tizen.music-player

DUMP_DIR=$1/$PKG_NAME
/bin/mkdir -p $DUMP_DIR

# app data
APP_DATA_DIR=/opt/usr/apps/$PKG_NAME/data
if [ -d "$APP_DATA_DIR" ]
then
	/bin/echo "copy ${APP_DATA_DIR}"
	/bin/cp -a $APP_DATA_DIR $DUMP_DIR
fi

# media DB
MEDIA_DB=/opt/usr/dbspace/.media.db
if [ "$MEDIA_DB" ]
then
	/bin/echo "copy media DB ..."
	/bin/cp ${MEDIA_DB}* $DUMP_DIR
fi
