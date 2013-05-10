#!/bin/sh

SCRIPT_DIR=$(cd $(dirname $0); pwd)
TARGET_DIR="${SCRIPT_DIR}/target"
DOC_DIR="${SCRIPT_DIR}/src/main/doc"
EX_NUM=03

cd "${SCRIPT_DIR}"

#./run.sh

cd "${DOC_DIR}"
pdflatex \
		-interaction=nonstopmode \
		-output-format=pdf \
		-output-directory="${TARGET_DIR}" \
		doc.tex

cd "${TARGET_DIR}"
mv doc.pdf DIP_SS13_GruppeF_Ex${EX_NUM}.pdf

