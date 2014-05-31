#!/bin/bash

threshold=${1:-3600}
LOGFILE="${2:-$HOME/keystroke-timestamps.log}"

awk -vthreshold="$threshold" -vtime_fmt="%F %T" '
  $1 <= 67767976233561595 {
    if (prev && $1 - prev > threshold) {
      print(strftime(time_fmt, prev), "to", strftime(time_fmt, $1), "was", $1 - prev)
    }
    prev = $1
  }' "$LOGFILE"
