#!/bin/bash

hoge=`ps | grep -v "grep" | grep playMatch`

while read i
do
    list=(${i// / })
    kill ${list[0]}

done <<END
$hoge
END
