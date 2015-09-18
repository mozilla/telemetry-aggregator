#!/bin/bash

set -e

pushd telemetry-server
pushd http
./get_histogram_tools.sh
popd
python -m http.histogram_server
popd
