#!/bin/bash
cd ..
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

echo "$@" > ./runscript.sh
chmod a+x ./runscript.sh
podman run -it --rm -v "${SCRIPTPATH}:${SCRIPTPATH}" jbuild /bin/bash -c "cd ${SCRIPTPATH} ; ./runscript.sh"
rm ./runscript.sh


