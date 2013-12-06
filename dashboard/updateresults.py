import simplejson as json
from urllib2 import urlopen, HTTPError
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
import sys, os, gzip
from utils import mkdirp
from auxiliary import replace_nan_inf
from multiprocessing import Process, Queue, cpu_count
from boto.s3 import connect_to_region as s3_connect
from boto.s3.key import Key
from boto.s3.prefix import Prefix
import os, sys, gzip
from shutil import rmtree
from cStringIO import StringIO
from s3get import s3get
from s3put import s3put
from results2disk import ChannelVersionManager
from datetime import datetime
from time import sleep

def s3get_json(s3_bucket, prefix, decompress, fallback_value = None):
    k = s3_bucket.get_key(prefix)
    if k is None:
        return fallback_value
    data = k.get_contents_as_string()
    if decompress:
        fobj = StringIO(data)
        with gzip.GzipFile(mode = 'rb', fileobj = fobj) as zobj:
            data = zobj.read()
        fobj.close()
    return json.loads(data)

def s3put_json(s3_bucket, prefix, compress, value):
    k = Key(s3_bucket)
    k.key = prefix
    data = json.dumps(value)
    headers = {
        'Content-Type':     'application/json'
    }
    if compress:
        fobj = StringIO()
        with gzip.GzipFile(mode = 'wb', fileobj = fobj) as zobj:
            zobj.write(data)
        data = fobj.getvalue()
        fobj.close()
        headers['Content-Encoding'] = 'gzip'
    k.set_contents_from_string(data, headers = headers)

def updateresults(input_folder, work_folder, bucket, prefix, cache_folder,
                  region, aws_cred, nb_workers):
    # Find input files
    input_files = []
    for path, folders, files in os.walk(input_folder):
        for f in files:
            # Get channel version
            cv = os.path.relpath(os.path.join(path, f), input_folder)
            input_files.append(cv)

    # Sanitize prefix
    if prefix[-1] != '/':
        prefix += '/'

    # Connect to s3
    s3 = s3_connect(region, **aws_cred)
    s3_bucket = s3.get_bucket(bucket, validate = False)

    # Download versions.json if not in cache and load versions
    versions_json = os.path.join(cache_folder, 'versions.json')
    if not os.path.isfile(versions_json):
        versions = s3get_json(s3_bucket, prefix + 'versions.json', True, {})
        with open(versions_json, 'w') as f:
            json.dump(versions, f)
    else:
        with open(versions_json, 'r') as f:
            versions = json.load(f)

    # Update results in bucket
    for channel_version in input_files:
        print "### Updating: " + channel_version

        # Download all files for channel_version to disk
        rmtree(work_folder, ignore_errors = True)
        data_folder = os.path.join(work_folder, channel_version)
        mkdirp(data_folder)
        snapshot = versions.get(channel_version, None)
        if snapshot:
            fetched = False
            while not fetched:
                fetched = s3get(bucket, prefix + snapshot, data_folder, True,
                                False, region, aws_cred)
                if not fetched:
                    print >> sys.stderr, "Failed to download %s" % snapshot
                    sleep(5 * 60)
            print " - downloaded " + snapshot

        # Create ChannelVersionManager
        channel, version = channel_version.split('/')
        manager = ChannelVersionManager(work_folder, channel, version, False,
                  False, False)

        # Feed it with rows from input_file
        rows = 0
        with open(os.path.join(input_folder, channel_version), 'r') as f:
            for line in f:
                filePath, blob = line.split('\t')
                channel_, version_, measure, byDateType = filePath.split('/')
                blob = json.loads(blob)
                if channel_ != channel or version_ != version:
                    print >> sys.stderr, ("Error: Found %s/%s within a %s file!"
                                        % (channel_, version_, channel_version))
                    continue
                manager.merge_in_blob(measure, byDateType, blob)
                rows += 1
        manager.flush()

        print " - merged rows %i" % rows

        # Upload updated files to S3
        date = datetime.utcnow().strftime("%Y%m%d%H%M%S")
        cv_prefix = "%s-%s-%s" % (date, version, channel)
        uploaded = False
        while not uploaded:
            uploaded = s3put(data_folder, bucket, prefix + cv_prefix, False,
                             True, region, aws_cred, nb_workers)
            if not uploaded:
                print >> sys.stderr, "Failed to upload '%s'" % cv_prefix
                sleep(5 * 60)

        print " - uploaded to " + cv_prefix

        # Store changes in versions
        versions[channel_version] = cv_prefix

    # Upload new versions.json and write to cache
    s3put_json(s3_bucket, prefix + 'versions.json', True, versions)
    with open(versions_json, 'w') as f:
            json.dump(versions, f)

    print "### New snapshot uploaded"

    try:
        # Garbage collect old channel/version folders on S3
        collect_garbage(bucket, prefix, cache_folder, region, aws_cred, nb_workers)
    except:
        print >> sys.stderr, "Failed to collect garbage on S3"

def collect_garbage(bucket, prefix, cache_folder, region, aws_cred, nb_workers):
    # Sanitize prefix
    if prefix[-1] != '/':
        prefix += '/'

    # Connect to s3
    s3 = s3_connect(region, **aws_cred)
    s3_bucket = s3.get_bucket(bucket, validate = False)

    # Download versions.json if not in cache and load versions
    versions_json = os.path.join(cache_folder, 'versions.json')
    if not os.path.isfile(versions_json):
        versions = s3get_json(s3_bucket, prefix + 'versions.json', True, {})
        with open(versions_json, 'w') as f:
            json.dump(versions, f)
    else:
        with open(versions_json, 'r') as f:
            versions = json.load(f)

    print "### Collecting Garbage on S3"

    # List prefixes from bucket
    obsolete = []
    current = versions.values()
    for p in s3_bucket.list(prefix = prefix, delimiter = '/'):
        if isinstance(p, Prefix):
            if p.name[len(prefix):-1] not in current:
                obsolete.append(p.name)

    # For each obsolute prefix
    for old in obsolete:
        # List objects and delete
        deleted = 0
        keys = []
        for k in s3_bucket.list(prefix = old):
            if len(keys) >= 1000:
                try:
                    s3_bucket.delete_keys(keys)
                    deleted += len(keys)
                except:
                    print >> sys.stderr, ("Failed to delete %i objects from %s"
                                          % (len(keys), old))
                keys = []
            keys.append(k)
        if len(keys) > 0:
            try:
                s3_bucket.delete_keys(keys)
                deleted += len(keys)
            except:
                print >> sys.stderr, ("Failed to delete %i objects from %s"
                                      % (len(keys), old))
        print " - Deleted %i objects from %s" % (deleted, old)

def main():
    p = ArgumentParser(
        description = 'Update results in bucket',
        formatter_class = ArgumentDefaultsHelpFormatter
    )
    p.add_argument(
        "-f", "--input-folder",
        help = "Folder with input files to update from",
        required = True
    )
    p.add_argument(
        "-w", "--work-folder",
        help = "Work folder for temporary files",
        required = True
    )
    p.add_argument(
        "-b", "--bucket",
        help = "Bucket to update results in",
        required = True
    )
    p.add_argument(
        "-p", "--prefix",
        help = "Prefix in bucket to update",
        required = True
    )
    p.add_argument(
        "-c", "--cache-folder",
        help = "Cache folder, files in here are assumed up to date",
        required = True
    )
    p.add_argument(
        "-k", "--aws-key",
        help = "AWS Key"
    )
    p.add_argument(
        "-s", "--aws-secret-key",
        help = "AWS Secret Key"
    )
    p.add_argument(
        "-r", "--region",
        help = "AWS region to connect to",
        default = 'us-west-2'
    )
    p.add_argument(
        "-j", "--nb-workers",
        help = "Number of parallel workers",
        default = "4 x cpu-count"
    )

    cfg = p.parse_args()
    aws_cred = {
        'aws_access_key_id':        cfg.aws_key,
        'aws_secret_access_key':    cfg.aws_secret_key
    }

    nb_workers = None
    try:
        nb_workers = int(cfg.nb_workers)
    except ValueError:
        nb_workers = cpu_count() * 4

    updateresults(cfg.input_folder, cfg.work_folder, cfg.bucket, cfg.prefix,
                  cfg.cache_folder, cfg.region, aws_cred, nb_workers)


if __name__ == "__main__":
    sys.exit(main())