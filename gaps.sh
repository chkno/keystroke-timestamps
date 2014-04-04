#!/bin/bash

threshold=${1:-3600}
LOGFILE="$HOME/keystroke-timestamps.log"

awk -vthreshold="$threshold" '
  BEGIN {
    time_fmt = "%F %T"
  }
  {
    if (prev && $1 - prev > threshold) {
      print(strftime(time_fmt, prev), "to", strftime(time_fmt, $1), "was", $1 - prev)
    }
    prev = $1
  }' "$LOGFILE"
