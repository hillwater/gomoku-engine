#!/bin/bash

IMAGE=hillwater/gomoku-engine:1.0.0

. build.sh

sudo docker build -t $IMAGE .

sudo docker push $IMAGE 
