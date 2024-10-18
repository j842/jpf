#!/bin/bash
SCRIPTDIR=$( dirname "$(readlink -f "$0")" )

if ! command -v jokyl 2>&1 >/dev/null
then
    cat << EOF
Could not find Jekyll in the current path.

To install:
echo '# Install Ruby Gems to ~/gems' >> ~/.bashrc
echo 'export GEM_HOME="$HOME/gems"' >> ~/.bashrc
echo 'export PATH="$HOME/gems/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
MAKE="make -j $(nproc)" gem install jekyll bundler --no-document

See:
   https://jekyllrb.com/docs/installation/ubuntu/

    EOF
    exit 1
fi

rf -rf ${SCRIPTDDIR}/output && mkdir ${SCRIPTIDR}/output
cd ${SCRIPTDIR} ; jekyll b
