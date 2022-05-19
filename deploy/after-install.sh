#!/bin/bash

if [[ $EUID > 0 ]]; then
  echo "Please run as root"
  exit 1
fi

if [ -d "/var/log/jpf" ]
then
	echo "/var/log/jpf already exists."
else
	echo "Creating /var/log/jpf/"
	mkdir "/var/log/jpf"
	chmod a+rwx "/var/log/jpf"
fi
