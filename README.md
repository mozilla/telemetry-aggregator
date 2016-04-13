Telemetry Dashboard
===================
As of 2016-04-13, this code is deprecated. See the [python_mozaggregator](https://github.com/mozilla/python_mozaggregator) repo for the next iteration.

Telemetry dashboard is an analysis job that aggregates telemetry histograms and
simple measures, and offers an decent presentation. The default dashboard
developed in this repository is hosted at
[telemetry.mozilla.org](http://telemetry.mozilla.org). But the aggregated data
is also available for consumption by third-party applications, so you don't need
to do the aggregation on your own.

Consuming Telemetry Aggregations
--------------------------------
Include into your code `http://telemetry.mozilla.org/v1/telemetry.js` feel free
to use the other modules too.
Don't go about reading the raw JSON files, they are not designed for human
consumption!


Hacking Telemetry Dashboard
---------------------------
If you want to improve the user-interface for telemetry dashboard, clone this
repository, setup a static server that hosts the `html/` folder on our localhost
and start hacking. This is easy!

If you want to add new aggregations, or improve upon existing aggregations,
change the storage format, take a look at `Formats.mkd`. Talk to the guy who is
maintaining telemetry dashboard.

Basic flow is as follows:
  1. An `.egg` file is generated with `make egg`
  2. Analysis tasks are created with telemetry-server
  3. `DashboardProcessor` from `analysis.py` aggregated telemetry submissions,
     this process is driven by telemetry-server.
  4. `Aggregator` from `aggregator.py` collects results from analysis tasks, by:
    1. Downloads existing data from s3
    2. Fetch task finished messages from SQS
    3. Download `result.txt` files in parallel
    4. Updates results on disk
    5. Publishes updated results in a new subfolder of `current/` on s3, every
       once in a while.
    6. Check points all aggregated data to a subfolder of `check-points/` on s3,
       every once in a while.
    7. Repeat
