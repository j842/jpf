#!/bin/bash


SCRIPTDIR=$( dirname "$(readlink -f "$0")" )

function showvar {
    filename="$SCRIPTDIR/../src/versions.makefile"
    echo "$(grep -m 1 ${1} $filename | sed 's/^.*= //g')"
}
JPF_VERSION="$(showvar JPF_VERSION)-$(showvar JPF_RELEASE)"


docker push "j842/jpf:${JPF_VERSION}"
docker push j842/jpf:latest
