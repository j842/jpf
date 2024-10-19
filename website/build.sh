#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )


JEKYLLDIR="${SCRIPTDIR}/output/.jekyll"
TARGETDIR="${SCRIPTDIR}/output"
echo "Jekyll temporary output: ${JEKYLLDIR}"

rm -rf ${TARGETDIR}
mkdir -p ${JEKYLLDIR}

cp -r ${SCRIPTDIR}/template/* ${JEKYLLDIR}

export JEKYLL_VERSION=3.8
docker run --rm \
  --volume="$JEKYLLDIR:/srv/jekyll:Z" \
  -it jekyll/jekyll:$JEKYLL_VERSION \
  jekyll build


mv ${JEKYLLDIR}/_site ${TARGETDIR}/html
# "${TARGETDIR}/_site" "${TARGETDIR}/html"
