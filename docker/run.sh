#!/bin/bash

# -- for testing image before pushing!

echo "--- Stopping JPF"
name="jpf"
docker ps -q --filter "name=$name" | xargs -r docker stop
until [ -n '$(docker ps -q --filter "name=$name")' ]
do
    echo "Container still running... waiting..."
    sleep 0.5
done
echo "--- Removing JPF"
docker ps -a -q --filter "name=$name" | xargs -r docker rm
until [ -n '$(docker ps -a -q --filter "name=$name")' ]
do
    echo "Container still exists... waiting..."
    sleep 0.5
done

echo "--- Running new JPF"
docker run --name jpf_dev --rm -d -v /home/j/jpf_config:/config -p 8080:80 j842/jpf
