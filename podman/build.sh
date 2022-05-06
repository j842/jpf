#!/bin/sh

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

podman build ${SCRIPTPATH}/packagecloud -t packagecloud
podman build ${SCRIPTPATH}/fpm -t fpm

