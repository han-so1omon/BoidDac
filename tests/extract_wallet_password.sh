#!/bin/bash

key="$1"

case $key in
  -f|--file)
  FILE="$2"
  shift
  shift
  ;;
esac

PASSWORD=`awk -F'"' '{print $2}' $FILE`

echo $PASSWORD
