#include "Aggregate.h"

#include "../../stringencoders/modp_numtoa.h"

#include <stdio.h>
#include <math.h>
#include <limits>

using namespace std;
using namespace rapidjson;

InternedStringContext  Aggregate::_buildIdStringCtx;

InternedStringContext  Aggregate::_revisionStringCtx;


InternedString Aggregate::internRevisionString(const char* revision) {
  return _revisionStringCtx.createString(revision);
}

InternedString Aggregate::internBuildIdString(const char* buildId) {
  return _buildIdStringCtx.createString(buildId);
}


void Aggregate::mergeJSON(const Value& dump) {
  const Value::Member* jvalues    = dump.FindMember("values");
  const Value::Member* jrevision  = dump.FindMember("revision");
  const Value::Member* jbuildId   = dump.FindMember("buildId");
  if (!jvalues || !jvalues->value.IsArray()) {
    fprintf(stderr, "'values' in dump isn't an array\n");
    return;
  }
  if (!jrevision || !jrevision->value.IsString()) {
    fprintf(stderr, "'revision' in dump isn't a string\n");
    return;
  }
  if (!jbuildId || !jbuildId->value.IsString()) {
    fprintf(stderr, "'buildId' in dump isn't a string\n");
    return;
  }
  const char* buildId  = jbuildId->value.GetString();
  const char* revision = jrevision->value.GetString();
  const Value& values  = jvalues->value;
  size_t length        = values.Size();

  // Check length of values
  if(length == 0) {
    fprintf(stderr, "Empty 'values' array in dump!\n");
  }

  // Check that we have doubles
  for (size_t i = 0; i < length; i++) {
    if(!values[i].IsNumber()) {
      fprintf(stderr, "Array contains non-double value!\n");
      return;
    }
  }

  // Check if length matches
  if (_length != length) {
    // Replace if we have newer buildId or current length is zero
    if (_length == 0 || _buildId < buildId) {
      // Set buildId and revision
      _buildId  = _buildIdStringCtx.createString(buildId);
      _revision = _revisionStringCtx.createString(revision);
      _length   = length;

      // Free old values
      if (_values) {
        delete[] _values;
      }
      _values = new double[length];

      for (size_t i = 0; i < length; i++) {
        _values[i] = values[i].GetDouble();
      }
    }
  } else {
    // Update revision and buildId if we have a newer one
    if (_buildId < buildId) {
      _buildId  = _buildIdStringCtx.createString(buildId);
      _revision = _revisionStringCtx.createString(revision);
    }

    size_t i;
    for (i = 0; i < length - 6; i++) {
      _values[i] += values[i].GetDouble();
    }
    for (; i < length - 1; i++) {
      double val = values[i].GetDouble();
      // Do not accumulate -1 (this indicates missing entry)
      if (val == -1 && _values[i] == -1) {
        continue;
      }
      _values[i] += val;
    }
    _values[i] += values[i].GetDouble();
  }
}

void Aggregate::aggregate(const InternedString& revision,
                          const InternedString& buildId,
                          const rapidjson::Value& values) {
  if (!values.IsArray()) {
    fprintf(stderr, "'values' in histogram data isn't an array\n");
    return;
  }
  size_t length = values.Size() + 1; // Add count which is one
  // Check that we have doubles
  for (size_t i = 0; i < length - 1; i++) {
    if(!values[i].IsNumber()) {
      fprintf(stderr, "Array contains non-double value!\n");
      return;
    }
  }

  // Check if length matches
  if (_length != length) {
    // Replace if we have newer buildId or current length is zero
    if (_length == 0 || _buildId < buildId) {
      // Set buildId and revision
      _buildId  = buildId;
      _revision = revision;
      _length   = length;

      // Free old values
      if (_values) {
        delete[] _values;
      }
      _values = new double[length];

      for (size_t i = 0; i < length - 1; i++) {
        _values[i] = values[i].GetDouble();
      }
      _values[length - 1] = 1;
    }
  } else {
    // Update revision and buildId if we have a newer one
    if (_buildId < buildId) {
      _buildId  = buildId;
      _revision = revision;
    }

    size_t i;
    for (i = 0; i < length - 6; i++) {
      _values[i] += values[i].GetDouble();
    }
    for (; i < length - 1; i++) {
      double val = values[i].GetDouble();
      // Do not accumulate -1 (this indicates missing entry)
      if (val == -1 && _values[i] == -1) {
        continue;
      }
      _values[i] += val;
    }
    _values[i] += 1;
  }
}

// Pre-generated bucket index to bucket end map for simple measure aggregates
double simpleMeasureBuckets[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24, 29, 35, 43, 52, 63, 77, 94,
  115, 140, 171, 209, 255, 311, 379, 462, 564, 688, 839, 1023, 1248, 1522, 1857,
  265, 2763, 3370, 4111, 5015, 6118, 7463, 9104, 11106, 13548, 16527, 20161,
  24593, 30000
};

void Aggregate::aggregate(double simpleMeasure) {
  size_t length = 50 + 5 + 1; // 50 buckets, 5 stats, 1 count
  if (_length != length) {
    // Replace if we have newer buildId or current length is zero
    if (_length == 0 || _buildId < "0") {
      // Set buildId and revision
      _buildId  = _buildIdStringCtx.createString("0");
      _revision = _buildIdStringCtx.createString("simple-measures-hack");
      _length   = length;

      // Free old values
      if (_values) {
        delete[] _values;
      }
      _values = new double[length];

      // Set first 50 to zero
      for (size_t i = 0; i < 50; i++) {
        _values[i] = 0;
      }

      // Add one for the index this simple measurement falls into
      for (int i = 49; i >= 0; i--) {
        if (simpleMeasure >= simpleMeasureBuckets[i]) {
          _values[i] += 1;
          break;
        }
      }

      // Find log value:
      double log_val = log(fabs(simpleMeasure) + 1);

      // Set stats
      _values[length - 6] = simpleMeasure;
      _values[length - 5] = log_val;
      _values[length - 4] = log_val * log_val;
      _values[length - 3] = -1;
      _values[length - 2] = -1;

      // Set count
      _values[length - 1] = 1;
    }
  } else {
    // Update revision and buildId if we have a newer one
    if (_buildId < "0") {
      _buildId  = _buildIdStringCtx.createString("0");
      _revision = _buildIdStringCtx.createString("simple-measures-hack");
    }

    // Add one for the index this simple measurement falls into
    for (int i = 49; i >= 0; i--) {
      if (simpleMeasure >= simpleMeasureBuckets[i]) {
        _values[i] += 1;
        break;
      }
    }

    // Find log value:
    double log_val = log(simpleMeasure);

    // Update stats
    _values[length - 6] += simpleMeasure;
    _values[length - 5] += log_val;
    _values[length - 4] += log_val * log_val;

    // Update count
    _values[length - 1] += 1;
  }
}

void Aggregate::output(FILE* f) {
  fputs("{\"values\": [", f);
  if(_length > 0) {
    char b[64];
    double val = _values[0];
    // Nan Check
    if(val != val) {
      fputs("null", f);
    } else {
      modp_dtoa2(val, b, 9);
      fputs(b, f);
    }
    for(size_t i = 1; i < _length; i++) {
      fputc(',', f);
      val = _values[i];
      // NaN check
      if(val != val) {
        fputs("null", f);
      } else {
        // Infinity check
        if (isinf(val)) {
          if (val > 0) {
            val = numeric_limits<double>::max();
          } else {
            val = -numeric_limits<double>::max();
          }
        }
        modp_dtoa2(val, b, 9);
        fputs(b, f);
      }
    }
  }
  fputs("],\"buildId\":\"", f);
  fputs(_buildId.data(), f);
  fputs("\",\"revision\":\"", f);
  fputs(_revision.data(), f);
  fputs("\"}", f);
}

