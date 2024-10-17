#!/bin/bash

echo "--- Stopping JPF"
docker stop jpf

echo "--- Running new JPF"
docker run --name jpf --rm -d -v /home/j/jpf_config:/config -p 8080:80 j842/jpf
