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
	wget "http://img3.etsystatic.com/000/0/6386326/il_fullxfull.300866075.jpg"
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

