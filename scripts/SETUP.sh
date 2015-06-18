#!/usr/bin/env bash

# RUN THIS ON A FRESH UBUNTU MACHINE TO SET IT UP AS A TELEMETRY DASHBOARD MASTER NODE

set -e

sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install doxygen
sudo apt-get install subversion git
sudo apt-get install python-setuptools python-pip
sudo pip install simplejson
apt-get install cmake libprotoc-dev zlib1g-dev libboost-system1.54-dev \
  libboost-filesystem1.54-dev libboost-thread1.54-dev libboost-test1.54-dev \
  libboost-log1.54-dev libboost-regex1.54-dev protobuf-compiler libssl-dev \
  liblzma-dev xz-utils

# setup up aggregator
git clone https://github.com/mozilla/telemetry-aggregator.git telemetry-aggregator
pushd telemetry-aggregator/
sudo python setup.py install # set up python files
cmake . && make # compile aggregator binaries
popd

# set up telemetry server
git clone https://github.com/mozilla/telemetry-server.git telemetry-server
pushd telemetry-server/
cmake . && make # compile server binaries
popd

# setup dashboard files
git clone https://github.com/mozilla/telemetry-dashboard.git telemetry.mozilla.org
