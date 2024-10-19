#!/bin/bash


sockexec /tmp/exec.sock &
chown nobody:www-data /tmp/exec.sock
chmod a+rw /tmp/exec.sock

resty /lua/update.lua

#nginx -g "daemon off;"

/usr/local/openresty/bin/openresty -g "daemon off;"
