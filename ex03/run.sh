#!/bin/sh

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TARGET_DIR="${SCRIPT_DIR}/target"
RESOURCES_DIR="${SCRIPT_DIR}/src/main/resources"
EXECUTABLE=$(ls ${TARGET_DIR}/dip3)
SRC_IMG_URL=http://www.lolhome.com/img_big/funny-picture-9979524311.jpg
#SRC_IMG_URL=http://www.thewallpapers.org/wallpapers/14/1420/thumb/600_los_angeles_skyline-001.jpg

cd "${SCRIPT_DIR}"

if [ ! -f "${EXECUTABLE}" ]; then
	./build.sh
fi

if [ ! -f "${RESOURCES_DIR}/img/input.jpg" ]; then
	mkdir -p "${RESOURCES_DIR}/img"
	cd "${RESOURCES_DIR}/img/"
	wget "${SRC_IMG_URL}"
	mv *.jpg input.jpg
fi

cd "${TARGET_DIR}"

ARGUMENTS="${RESOURCES_DIR}/img/input.jpg"

if [ "${2}" = "debug" ]; then
	echo "ARGUMENTS: ${ARGUMENTS}"
	ddd "${EXECUTABLE}" &
else
	"${EXECUTABLE}" ${ARGUMENTS}
fi

