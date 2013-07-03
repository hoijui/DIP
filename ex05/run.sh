#!/bin/sh

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TARGET_DIR="${SCRIPT_DIR}/target"
RESOURCES_DIR="${SCRIPT_DIR}/src/main/resources"
EXECUTABLE=$(ls ${TARGET_DIR}/dip5)
#SRC_IMG_URL=http://www.lolhome.com/img_big/funny-picture-9979524311.jpg
#SRC_IMG_URL=http://www.thewallpapers.org/wallpapers/14/1420/thumb/600_los_angeles_skyline-001.jpg
#SRC_IMG_URL=http://c1.wall-art.de/img/Filzdesign_strichmaennchen_einzel_web.jpg
#SRC_IMG_URL=http://netfamilie.de/wp-content/uploads/548847_R_K_B_by_Wilhelmine-Wulff_pixelio.de_.jpg
SRC_IMG_URL=http://number1-neo.de/media/images/popup/441745.jpg
#SRC_IMG_URL=http://image.spreadshirt.net/image-server/v1/designs/13192802,width=178,height=178/Strichmaennchen-Paar---line.png

cd "${SCRIPT_DIR}"

if [ ! -f "${EXECUTABLE}" ]; then
	./build.sh
fi

if [ ! -f "${RESOURCES_DIR}/img/input.jpg" ]; then
	mkdir -p "${RESOURCES_DIR}/img"
	cd "${RESOURCES_DIR}/img/"
	wget "${SRC_IMG_URL}"
	mv $(basename ${SRC_IMG_URL}) "input.jpg"
fi

if [ ! -f "${TARGET_DIR}/input.jpg" ]; then
	cp "${RESOURCES_DIR}/img/input.jpg" "${TARGET_DIR}"
fi

cd "${TARGET_DIR}"

ARGUMENTS="${TARGET_DIR}/input.jpg"

if [ "${2}" = "debug" ]; then
	echo "ARGUMENTS: ${ARGUMENTS}"
	ddd "${EXECUTABLE}" &
else
	"${EXECUTABLE}" ${ARGUMENTS}
fi

