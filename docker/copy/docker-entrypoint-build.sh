#!/bin/bash

cd /code/src && make clean && make && make check

chmod 777 /code/src/build/jpf
