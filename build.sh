#!/bin/bash

# for release
node-gyp configure build

# for debug
node-gyp configure build --debug
