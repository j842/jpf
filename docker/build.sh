#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )

echo "------------------------------------------------------------------"
echo "                    BUILDING build image"
echo "------------------------------------------------------------------"

docker build -f Dockerfile.build ${SCRIPTDIR} -t j842/jpfbuild

echo "------------------------------------------------------------------"
echo "                    COMPILING jpf"
echo "------------------------------------------------------------------"

rm -rf "${SCRIPTDIR}/copy/temp"
mkdir -p "${SCRIPTDIR}/copy/temp"

docker run --rm -v ${SCRIPTDIR}/../:/code j842/jpfbuild

cp "${SCRIPTDIR}/../src/build/jpf" "${SCRIPTDIR}/copy/temp/"
cp -r "${SCRIPTDIR}/../website" "${SCRIPTDIR}/copy/temp/"

echo "------------------------------------------------------------------"
echo "                    BUILDING jpf image"
echo "------------------------------------------------------------------"

docker build -f Dockerfile.gsheet ${SCRIPTDIR} -t j842/jpf

rm -rf "${SCRIPTDIR}/copy/temp"
