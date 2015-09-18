#!/bin/bash

cd ~;

# We don't accept errors
set -e;

# Let's process yesterday
YESTERDAY=`date -d yesterday +%Y%m%d`;

# Print date for easier debugging
echo "Running on data from: $YESTERDAY";

# Create filter for current day
cat filter.json.in | sed -e s/DATE/$YESTERDAY/ > filter.json;

# Launch tasks
cd telemetry-server;
python -m analysis.launcher --target-queue telemetry-analysis-dashboard-aggregates-test --sqs-queue 'telemetry-analysis-v2-telemetryAnalysisInput-9GR13IZUZXZU' ../telemetry-aggregator/build/aggregator_bundle.tar.gz  -f ../filter.json -n "telemetry-dashboard" -o "jojensen@mozilla.com";
cd -;
