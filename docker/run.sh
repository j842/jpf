#!/bin/bash

# -- for testing image before pushing!

CONTAINER_NAME="jpf_dev"

echo "--- Stopping JPF container if running"
if docker ps -q --filter "name=$CONTAINER_NAME" | grep -q .; then
    docker stop $CONTAINER_NAME
    echo "Container stopping..."
    
    # Wait until container is fully stopped
    while docker ps -q --filter "name=$CONTAINER_NAME" | grep -q .; do
        echo "Waiting for container to stop..."
        sleep 1
    done
    echo "Container stopped"
else
    echo "No running container found"
fi

echo "--- Removing JPF container if exists"
if docker ps -a -q --filter "name=$CONTAINER_NAME" | grep -q .; then
    docker rm $CONTAINER_NAME
    echo "Container removing..."
    
    # Wait until container is fully removed
    while docker ps -a -q --filter "name=$CONTAINER_NAME" | grep -q .; do
        echo "Waiting for container to be removed..."
        sleep 1
    done
    echo "Container removed"
else
    echo "No container to remove"
fi

echo "--- Running new JPF container"
docker run --name ${CONTAINER_NAME} --rm -d -v /usr/local/jpf_config:/config -p 8080:80 j842/jpf
