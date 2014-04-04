#!/usr/bin/gnuplot

filename = "keystroke-timestamps.log"
bucket_size = 300            # Display resolution, in seconds
timezone_offset = -7 * 3600  # UTC-7, North American Pacific Daylight Time

set timefmt "%s"
set xdata time
set grid
set autoscale fix
unset key

plot "< awk '{ print int($1 / ".bucket_size.") }' \"".filename."\" \
  | uniq -c \
  | awk '{ print $2 * ".bucket_size.", $1 }' \
  | fill-in ".bucket_size \
  using ($1 + timezone_offset):2 with lines

# (fill-in is https://chkno.net/fill-in )

pause -1 "Press enter to dismiss"
