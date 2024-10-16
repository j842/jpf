#!/bin/bash

if [[ $(id -u) -ne 0 ]] ; then echo "Please run as root" ; exit 1 ; fi


if ! command -v gem &> /dev/null
then
    echo "Ruby's gem command could not be found - aborting!"
    exit 1
fi


if ! { [ -f "/usr/bin/jekyll" ] || [ -f "/usr/local/bin/jekyll" ]; }; then
    echo "Jekyll could not be found - installing (this will take several minutes)"

    gem install ffi -v 1.16.3
    gem install bundler -v 2.4.22
    MAKE="make -j $(nproc)" gem install jekyll -v 4.3.2 --no-document
fi

if ! { [ -f "/usr/bin/jekyll" ] || [ -f "/usr/local/bin/jekyll" ]; }; then
    echo "Jekyll install failed!"
    exit -1
fi

echo "Jekyll is available."

#gem update jekyll

exit 0

