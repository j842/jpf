#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )

GROUPID=$(id -g)

docker run --entrypoint "/bin/bash" --rm \
  --volume="$SCRIPTDIR:/j:Z" \
  -it j842/jpf:latest \
  -c "cp -R /j/* /jpftemp  &&  jpf /jpftemp  &&  chown -R ${UID}:${GROUPID} /jpftemp/output &&  cp -R /jpftemp/output/ /j/output"
