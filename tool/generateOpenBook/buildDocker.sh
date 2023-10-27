#!/bin/bash

IMAGE=hillwater/gomoku-generate-openbook:1.0.0

docker build -f ../../GenerateOpenBookDockerfile -t $IMAGE .

docker push $IMAGE 
