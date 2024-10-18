#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )

if ! command -v jekyll 2>&1 >/dev/null
then
    cat << EOF
------------------------------------------------------------------
------------------------------------------------------------------
ERROR: Could not find Jekyll in the current path.

To install:
echo '# Install Ruby Gems to ~/gems' >> ~/.bashrc
echo 'export GEM_HOME="$HOME/gems"' >> ~/.bashrc
echo 'export PATH="$HOME/gems/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
MAKE="make -j $(nproc)" gem install jekyll bundler --no-document

See:
   https://jekyllrb.com/docs/installation/ubuntu/

------------------------------------------------------------------
EOF
    exit 1
fi

JEKYLLDIR="${SCRIPTDIR}/output/.jekyll"
TARGETDIR="${SCRIPTDIR}/output"
echo "Jekyll temporary output: ${JEKYLLDIR}"

rm -rf ${TARGETDIR}
mkdir -p ${JEKYLLDIR}

cp -r ${SCRIPTDIR}/template/* ${JEKYLLDIR}

cd ${JEKYLLDIR} ; jekyll b

mv ${JEKYLLDIR}/_site ${TARGETDIR}/html
# "${TARGETDIR}/_site" "${TARGETDIR}/html"
