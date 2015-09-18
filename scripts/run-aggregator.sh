#!/bin/bash

set -e

cd telemetry-aggregator;
mkdir --parents /var/tmp/aggregator-work-folder/data;
python -m dashboard.aggregator -w /var/tmp/aggregator-work-folder/data --bucket telemetry-dashboard --prefix test telemetry-analysis-dashboard-aggregates-test;
cd -;
