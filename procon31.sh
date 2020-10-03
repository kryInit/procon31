#!/bin/bash
if [ $# -lt 2 ] ; then
    echo "[procon31.sh] 引数が足りません" >&2
    exit 1
fi

dirName=`dirname $0`
token=$1
URL=$2

teamID=`python $dirName/utility/initial.py $token $URL`
if [ -z $teamID ]; then
    echo "[procon31.sh] teamIDの値が不正です" >&2
    exit 1
fi

matchIDs=`python $dirName/utility/enumMatchID.py $token $URL $teamID`

echo -n "teamID: "
echo $teamID
echo -n "matchIDs: "
echo $matchIDs

for matchID in $matchIDs
do
    $dirName/playMatch $token $URL $teamID $matchID &
done

open $dirName/visualizer/visualizer.app

