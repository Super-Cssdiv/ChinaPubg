#!/bin/bash
MODDIR=${0%/*}
cd "$MODDIR"
process=false
packageName="com.tencent.tmgp.pubgmhd"
injectPath="$MODDIR/libs/PeaceTools"

while true
do
  sleep 1
  count=`ps -ef | grep $packageName | grep -v "grep" | wc -l`
  if [[ $count -gt 0 && $process == false ]];then
    process=true
    chmod -R 0777 "$injectPath"
    $injectPath $MODDIR/libs/arm64-v8a.so $packageName
  fi

  if [[ $count -le 0 && $process == true ]];then
    process=false
  fi
done