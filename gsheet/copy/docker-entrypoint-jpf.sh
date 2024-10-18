#!/bin/bash


sockexec /tmp/exec.sock &
chown nobody:www-data /tmp/exec.sock
chmod a+rw /tmp/exec.sock

nginx -g "daemon off;"


