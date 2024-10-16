#!/bin/bash

docker run --name jpf --rm -d -v /home/j/jpf_config:/config -p 8080:80 j842/jpf
