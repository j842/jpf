#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )
GROUPID=$(id -g)

function showvar {
    filename="$SCRIPTDIR/../src/versions.makefile"
    echo "$(grep -m 1 ${1} $filename | sed 's/^.*= //g')"
}
JPF_VERSION="$(showvar JPF_VERSION)-$(showvar JPF_RELEASE)"

echo "------------------------------------------------------------------"
echo "                    BUILDING build image"
echo "------------------------------------------------------------------"

docker build -f Dockerfile.jpfbuild ${SCRIPTDIR} -t j842/jpfbuild:latest

echo "------------------------------------------------------------------"
echo "                    COMPILING jpf ${JPF_VERSION}"
echo "------------------------------------------------------------------"

rm -rf "${SCRIPTDIR}/copy/temp"
mkdir -p "${SCRIPTDIR}/copy/temp"

docker run --rm -v ${SCRIPTDIR}/../:/code j842/jpfbuild:latest
docker run --entrypoint "/bin/bash" --rm \
  -v ${SCRIPTDIR}/../:/code -it j842/jpfbuild \
  -c "chown -R ${UID}:${GROUPID} /code/src/build"


cp "${SCRIPTDIR}/../src/build/jpf" "${SCRIPTDIR}/copy/temp/"
cp -r "${SCRIPTDIR}/../website" "${SCRIPTDIR}/copy/temp/"

echo "------------------------------------------------------------------"
echo "                    BUILDING jpf image ${JPF_VERSION}"
echo "------------------------------------------------------------------"

docker build -f Dockerfile.jpf ${SCRIPTDIR} -t j842/jpf:latest -t "j842/jpf:${JPF_VERSION}"
