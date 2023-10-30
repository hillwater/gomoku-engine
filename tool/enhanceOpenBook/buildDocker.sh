#!/bin/bash

IMAGE=hillwater/gomoku-enhance-openbook:1.0.0

docker build -f ./EnhanceOpenBookDockerfile -t $IMAGE .

docker push $IMAGE 
