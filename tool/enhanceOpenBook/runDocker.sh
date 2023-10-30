#!/bin/bash

IMAGE=hillwater/gomoku-enhance-openbook:1.0.0

REDIS_HOST=172.18.250.131

docker login

docker run -e "REDIS_HOST=$REDIS_HOST" -d $IMAGE
