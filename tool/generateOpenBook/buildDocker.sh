#!/bin/bash

IMAGE=hillwater/gomoku-generate-openbook:1.0.0

. ../../build.sh

docker build -t $IMAGE .

docker push $IMAGE 
