#!/usr/bin/env python

import subprocess, sys

if len(sys.argv) === 1:
    print "Usage: {} SOME_COMMAND SOME_COMMAND_ARGUMENTS".format(sys.argv[0])
    print "    If SOME_COMMAND exits with a non-zero error code, or results in any output, emails telemetry-alerts@mozilla.com with the exit code and the output."
    print "    Useful for cron jobs."

import mail

def alert(subject, body):
    mail.send_ses("telemetry-alerts@mozilla.com", subject, body, "telemetry-alerts@mozilla.com")

command = sys.argv[1:]
try:
    output = subprocess.check_output(command, stderr=subprocess.STDOUT)
    if output != "": raise subprocess.CalledProcessError(0, command, output)
except subprocess.CalledProcessError, e:
    alert("Command Failure", "Command failed: {}\r\n\r\nExit code: {}\r\n\r\nOutput:\r\n{}".format(command, e.returncode, e.output))
