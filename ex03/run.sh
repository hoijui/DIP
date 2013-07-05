#!/bin/sh

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TARGET_DIR="${SCRIPT_DIR}/target"
RESOURCES_DIR="${SCRIPT_DIR}/src/main/resources"
EXECUTABLE=$(ls ${TARGET_DIR}/dip3)
IMG_FILE_NAME=input.jpg
#IMG_FILE_NAME=input.png
SRC_IMG_URL=http://www.lolhome.com/img_big/funny-picture-9979524311.jpg
#SRC_IMG_URL=http://www.thewallpapers.org/wallpapers/14/1420/thumb/600_los_angeles_skyline-001.jpg

cd "${SCRIPT_DIR}"

if [ ! -f "${EXECUTABLE}" ]; then
	./build.sh
fi

if [ ! -f "${RESOURCES_DIR}/img/${IMG_FILE_NAME}" ]; then
	mkdir -p "${RESOURCES_DIR}/img"
	cd "${RESOURCES_DIR}/img/"
	wget "${SRC_IMG_URL}"
	mv *.jpg "${IMG_FILE_NAME}"
fi

if [ ! -f "${TARGET_DIR}/${IMG_FILE_NAME}" ]; then
	cp "${RESOURCES_DIR}/img/${IMG_FILE_NAME}" "${TARGET_DIR}"
fi

cd "${TARGET_DIR}"

ARGUMENTS="${TARGET_DIR}/${IMG_FILE_NAME}"

if [ "${2}" = "plots" ]; then

	cp "${RESOURCES_DIR}/img/tst250k.png" "${TARGET_DIR}/${IMG_FILE_NAME}"
	"${EXECUTABLE}" ${ARGUMENTS}
	mv "${TARGET_DIR}/convolutionFrequencyDomain.txt" "${TARGET_DIR}/convolutionFrequencyDomain250k.txt"
	mv "${TARGET_DIR}/convolutionSpatialDomain.txt" "${TARGET_DIR}/convolutionSpatialDomain250k.txt"

	cp "${RESOURCES_DIR}/img/tst625k.png" "${TARGET_DIR}/${IMG_FILE_NAME}"
	"${EXECUTABLE}" ${ARGUMENTS}
	mv "${TARGET_DIR}/convolutionFrequencyDomain.txt" "${TARGET_DIR}/convolutionFrequencyDomain625k.txt"
	mv "${TARGET_DIR}/convolutionSpatialDomain.txt" "${TARGET_DIR}/convolutionSpatialDomain625k.txt"

	cp "${RESOURCES_DIR}/img/tst1000k.png" "${TARGET_DIR}/${IMG_FILE_NAME}"
	"${EXECUTABLE}" ${ARGUMENTS}
	mv "${TARGET_DIR}/convolutionFrequencyDomain.txt" "${TARGET_DIR}/convolutionFrequencyDomain1000k.txt"
	mv "${TARGET_DIR}/convolutionSpatialDomain.txt" "${TARGET_DIR}/convolutionSpatialDomain1000k.txt"

	exit 0
fi

if [ "${2}" = "debug" ]; then
	echo "ARGUMENTS: ${ARGUMENTS}"
	ddd "${EXECUTABLE}" &
else
	"${EXECUTABLE}" ${ARGUMENTS}
fi

