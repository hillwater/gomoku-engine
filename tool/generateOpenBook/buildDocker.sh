#!/bin/bash

IMAGE=hillwater/gomoku-generate-openbook:1.0.0

docker build -t $IMAGE .

docker push $IMAGE 
