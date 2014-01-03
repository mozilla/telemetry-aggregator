#!/usr/bin/env python

import sys, json

counts = [0] * 50

for line in sys.stdin:
  dim, data = line.split('\t')
  if "FIRSTPAINT" in dim:
    for k, v in json.loads(data).iteritems():
      for i, v in enumerate(v['values'][:-6]):
        counts[i] += v

for i, v in enumerate(counts):
  if v > 1:
    print " Two hits in bucket %i" % i
    sys.exit(1)

