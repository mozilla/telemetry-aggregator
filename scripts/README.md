EC2 Dashboard Master Node
=========================

* Responsible for performing output aggregation, dispatching analysis tasks to the SQS input task queue, and retrieving the results back from the done queue.
* Everything runs in screen, currently with three windows:
    * One window for general status and monitoring.
    * One window for the continuously running histogram server - see `run-histograms-server.sh`.
    * One window for the continuously running aggregator job - see `run-aggregator.sh`.
* There are also cron jobs that run daily:
    * One that runs daily to start the analysis tasks on spot workers.
    * One that runs daily to update the static files for [telemetry.mozilla.org].
    * Cron jobs are run with [Cronic](http://habilis.net/cronic/) to improve error handling.
* Various other files:
    * `start-investigation.sh` starts the analysis tasks with a dedicated output queue, for debugging purposes.
    * `filter.json.in` is used by `start-analysis-tasks.sh` as the template for the filter that will be used. In the file, the string "DATE" will be replaced by the actual date to use.

Run `SETUP.sh` in the home directory to download and compile all the necessary files.

Notes in overall structure:

* The new dashboard master node is at [ec2-54-188-186-192.us-west-2.compute.amazonaws.com]. This is accessible via SSH only. See [https://us-west-2.console.aws.amazon.com/ec2/v2/home] for more details.
* The old dashboard master node is at [ec2-54-202-211-22.us-west-2.compute.amazonaws.com], and also accessible via SSH only. See [https://us-west-2.console.aws.amazon.com/ec2/v2/home] for more details.
* The task queue is the queue of tasks that are created by the EC2 dashboard master node and consumed by EC2 spot workers. Currently, this is called `telemetry-analysis-v2-telemetryAnalysisInput-9GR13IZUZXZU`. See [https://console.aws.amazon.com/sqs/home] for more details.
* The output queue is the queue of results that are created by the EC2 spot workers and consumed by the EC2 dashboard master. Currently, this is called `telemetry-analysis-dashboard-aggregates-test` (originally `telemetry-analysis-dashboard-aggregates`). See [https://console.aws.amazon.com/sqs/home] for more details.
* Final output data is stored in the web-accessible S3 bucket `telemetry-dashboard`. See [https://console.aws.amazon.com/s3/home] for more details.
* See [https://etherpad.mozilla.org/jonasfj-telemetry-aggregator-setup] for more details.

The crontab (`/etc/crontab`) should look something like this:

    SHELL=/bin/sh
    PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin

    # m h dom mon dow user  command
    31 13    * * *   ubuntu  ~/cronic-email.py ~/start-analysis-tasks.sh
    22 12    * * *   ubuntu  ~/cronic-email.py ~/update-telemetry.mozilla.org.sh
    17 *    * * *   root    cd / && run-parts --report /etc/cron.hourly
    25 6    * * *   root    test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.daily )
    47 6    * * 7   root    test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.weekly )
    52 6    1 * *   root    test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.monthly )
    #
    MAILTO="telemetry-alerts@mozilla.com"

Notes on configuration:

* AWS configuration needs to be set up: see [http://docs.aws.amazon.com/cli/latest/userguide/cli-chap-getting-started.html] for setting up keys and other settings.
* An SQS queue needs to be created with the following settings: delivery delay of 2 minutes, visibility timeout of 6 hours, and message retention period of 4 days.
* A folder needs to be created in the telemetry-dashboard S3 bucket for the aggregator to post output in.
* These S3 settings need to be updated in `~/run-aggregator.sh`.
* These SQS settings need to be updated in `~/run-analysis-tasks.sh`.