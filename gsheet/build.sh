#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )

cp "${SCRIPTDIR}/../src/build/jpf" "${SCRIPTDIR}/copy/"

docker build ${SCRIPTDIR} -t j842/gsheet_jpf

