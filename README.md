# Gomoku Engine

## Overview
it's a backend gomoku engine, 
the front side is https://github.com/hillwater/bamboo-ai 
you can have a play on http://www.hillwater.xyz/gomoku 



## how to run

### version requirement
- node 16.15.1
- node-gyp 9.4.0
- gcc 8.3.1
- python 3.6.8


````
docker run -e "REDIS_HOST=localhost" -d hillwater/gomoku-engine:3.0.0
````

## what's inside
it's a node js service.
consume message from redis list queue,
the message is a gomoku board state.
this service use an C++ addon module to calculate the target smart position.
write the result to redis.
this service only do calcuation, so it's stateless.

The front side, should handle the UI request, and send gomoku board state to redis list.
the JS side every 1 second to poll the caculation result.
