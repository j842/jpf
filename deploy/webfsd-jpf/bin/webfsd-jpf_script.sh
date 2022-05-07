#!/bin/sh
fpm -s dir -t deb -p /opt/bin/webfsd-jpf-0.0.2-any.deb --name webfsd-jpf --license Artistic-2.0 --version 0.0.2 --architecture all -d debconf -d libc6 -d libgnutls30 -d lsb-base -d ucf --description "Repackaged webfsd for jpf" --url "https://github.com/j842/jpf" --maintainer "John Enlow" /opt/webfsd-jpf=/usr/bin/webfsd-jpf 
