#!/bin/bash

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
  -f|--file)
  FILE="$2"
  shift
  shift
  ;;
  -t|--type)
  TYPE="$2"
  shift
  shift
  ;;
esac
done

set -- "${POSITIONAL[@]}"

if [ $TYPE = "public" ]; then
  KEY=`awk '/Public key:/{print $3}' $FILE`
elif [ $TYPE = "private" ]; then
  KEY=`awk '/Private key:/{print $3}' $FILE`
else
  echo "error: must name key type {public|private}"
  exit 1
fi

echo $KEY
