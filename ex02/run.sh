#!/bin/sh

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TARGET_DIR="${SCRIPT_DIR}/target"
RESOURCES_DIR="${SCRIPT_DIR}/src/main/resources"
EXECUTABLE=$(ls ${TARGET_DIR}/dip2)

cd "${SCRIPT_DIR}"

if [ ! -f "${EXECUTABLE}" ]; then
	./build.sh
fi

if [ ! -f "${RESOURCES_DIR}/img/input.jpg" ]; then
	mkdir -p "${RESOURCES_DIR}/img"
	cd "${RESOURCES_DIR}/img/"
	wget "http://www.lolhome.com/img_big/funny-picture-9979524311.jpg"
	mv *.jpg input.jpg
fi

cd "${TARGET_DIR}"

ARGUMENTS="generate ${RESOURCES_DIR}/img/input.jpg"

# generate noisy images
if [ ! -f "original.jpg" ]; then
	"${EXECUTABLE}" ${ARGUMENTS}
fi

ARGUMENTS="restorate"

if [ "${2}" = "debug" ]; then
	echo "ARGUMENTS: ${ARGUMENTS}"
	ddd "${EXECUTABLE}" &
else
	"${EXECUTABLE}" ${ARGUMENTS}
fi

