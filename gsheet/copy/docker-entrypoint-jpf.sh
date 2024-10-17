#!/bin/bash


sockexec /tmp/exec.sock &
chown www-data:www-data /tmp/exec.sock

nginx -g "daemon off;"


