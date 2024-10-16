#!/bin/bash

if [[ $(id -u) -ne 0 ]] ; then echo "Please run as root" ; exit 1 ; fi


if ! command -v gem &> /dev/null
then
    echo "Ruby's gem command could not be found - aborting!"
    exit 1
fi


if ! { [ -f "/usr/bin/jekyll" ] || [ -f "/usr/local/bin/jekyll" ]; }; then
    echo "Jekyll could not be found - installing (this will take several minutes)"
    MAKE="make -j $(nproc)" gem install jekyll bundler --no-document
fi

if ! { [ -f "/usr/bin/jekyll" ] || [ -f "/usr/local/bin/jekyll" ]; }; then
    echo "Jekyll install failed!"
    exit -1
fi

echo "Jekyll is available."

#gem update jekyll

exit 0

