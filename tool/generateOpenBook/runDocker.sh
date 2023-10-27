#!/bin/bash

IMAGE=hillwater/gomoku-generate-openbook:1.0.0

REDIS_HOST=172.17.0.1

docker login

docker run -e "REDIS_HOST=$REDIS_HOST" -d $IMAGE