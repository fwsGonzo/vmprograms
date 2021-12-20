#!/usr/bin/env bash
set -e

mkdir -p build
pushd build

cmake .. -G Ninja -DMULTIPROCESS=OFF
ninja

popd
