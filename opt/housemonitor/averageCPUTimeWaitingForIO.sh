#!/bin/bash
iostat -x 4 30 | grep sda | awk '{print $12}' | tail -n 29 | awk '{ SUM += $1 } END { print SUM/NR }'

