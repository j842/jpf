#!/bin/sh

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

cp ${HOME}/.packagecloud ${SCRIPTPATH}
podman build ${SCRIPTPATH} -t jpf
rm ${SCRIPTPATH}/.packagecloud
