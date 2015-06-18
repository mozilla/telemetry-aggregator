Telemetry Aggregator
====================
Telemetry Aggregator is an analysis job that aggregates telemetry histograms and simple measures. Aggregates are functions of histograms and simple measures, and include such things as the sum, count, and logarithmic sum.

These values are consumed by dashboards such as the one hosted at [telemetry.mozilla.org](http://telemetry.mozilla.org).

Consuming Telemetry Aggregations
--------------------------------
The aggregated data is available for third-party applications via the `telemetry.js` library - it is not necessary to run this analysis job yourself.

For consuming the results of the aggregations, see the [telemetry.js documentation](https://telemetry.mozilla.org/docs.html#Telemetry).

Avoid using or referencing the JSON data directly, as these may be changed or removed without warning.

Deploying Telemetry Aggregator/Telemetry Server
-----------------------------------------------

Telemetry Aggregator/Server is written for deployment on AWS.

* Provision an on-demand AWS machine (this document will assume a Ubuntu/Debian-derived medium instance).
* Copy the contents of `scripts/` onto the server into `~/`.
* See `~/README.md` for further setup instructions.

Hacking Telemetry Aggregator
----------------------------
To improve the user-interface for telemetry dashboard, see the README for the [Telemetry Dashboard repository](https://github.com/mozilla/telemetry-dashboard).

To add new aggregations, improve upon existing aggregations, or change the storage format, take a look at `Formats.mkd`. Additionally, make sure to talk to the telemetry dashboard maintainer(s).

Setting up the development environment:

  1. Install Subversion and LibLZMA.
    * On Debian-derived systems, simply run `apt-get install liblzma-dev subversion`.
    * On Windows, install [XZ-Utils](http://tukaani.org/xz/) and a Subversion distribution for Windows.
  2. Generate build files with CMake: in the project directory, run `cmake .`.
  3. Build all the targets by invoking `make`. These create executables that are invoked by the Python scripts.
  4. Set up the Python scripts using `python setup.py build`.

Primary workflow:

  1. Analysis tasks are created and run by [telemetry-server](https://github.com/mozilla/telemetry-server).
  2. The `DashboardProcessor` class in `dashboard/analysis.py` aggregates telemetry submissions. This is used in and driven by telemetry-server.
  3. The `Aggregator` class in `dashboard/aggregator.py` collects results from analysis tasks:
    1. Downloads existing data from S3.
    2. Fetches task finished messages from SQS.
    3. Downloads `result.txt` files in parallel.
    4. Updates results on disk.
    5. Publishes updated results in a new subfolder of `current/` on S3, every once in a while.
    6. Check points all aggregated data to a subfolder of `check-points/` on S3, every once in a while.
    7. Repeat.
