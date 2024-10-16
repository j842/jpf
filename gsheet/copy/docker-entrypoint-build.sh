#!/bin/bash

#ENTRYPOINT ["cd /code && make clean && make"]

mkdir /build
cp -r /code/* /build/

cd /build/src && make clean && make && make check

cp /build/src/build/jpf /code/src/build/jpf
chmod 777 /code/src/build/jpf
