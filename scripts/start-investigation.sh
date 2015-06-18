#!/bin/bash

cd ~;

# Launch tasks
cd telemetry-server;
python -m analysis.launcher -t jonasfj-investigation -q 'telemetry-analysis-telemetryAnalysisInput-1UEVX92B5VETU' ../telemetry-aggregator/build/aggregator_bundle.tar.gz  -f ../filter.json -n "telemetry-dashboard" -o "jojensen@mozilla.com";
cd -;

