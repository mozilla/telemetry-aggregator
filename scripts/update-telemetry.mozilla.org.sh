#!/bin/bash

cd ~/telemetry.mozilla.org;
git pull;
aws s3 sync . s3://telemetry.mozilla.org/ --delete --region us-east-1 --exclude ".*";
cd -;

exit 1;
