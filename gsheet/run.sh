#!/bin/bash

echo "--- Stopping JPF"
name="jpf"
docker ps -a -q --filter "name=$name" | xargs -r docker stop
until [ -n '$(docker ps -a -q --filter "name=$name")' ]
do
    echo "Container still exists... waiting..."
    sleep 0.5
done

echo "--- Running new JPF"
docker run --name jpf --rm -d -v /home/j/jpf_config:/config -p 8080:80 j842/jpf
#docker run --name jpf -d -v /home/j/jpf_config:/config -p 8080:80 j842/jpf
