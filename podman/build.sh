#!/bin/sh

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
JPFPATH=${SCRIPTPATH}/jpf
JBUILD=${SCRIPTPATH}/jbuild

cp ${HOME}/.packagecloud ${JPFPATH}
podman build ${JPFPATH} -t jpf
rm ${JPFPATH}/.packagecloud

podman build ${JBUILD} -t jbuild

