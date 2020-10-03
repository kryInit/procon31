# python makeGameInfo.py [token] [URL(最初の部分)] [teamID] [matchID] 
import os
import sys
import time
import requests

if (len(sys.argv) < 5):
    print("[makeGameInfo] 引数が足りません", file=sys.stderr)
    sys.exit()

usleep = lambda x: time.sleep(x/1000.0)

token = sys.argv[1]
teamID = sys.argv[3]
matchID = sys.argv[4]
URL = sys.argv[2] + "/teams/{0}/matches".format(teamID)
header = {"x-api-token": token}

con = requests.get(URL, headers=header)
sc = con.status_code

cnt = 0
while (sc == 400 or sc == 429) and cnt < 60:
    usleep(300);
    con = requests.get(URL, headers=header)
    sc = con.status_code
    cnt += 1

if sc != 200:
    print("[makeGameInfo] matchID: {0}, status code: {1}, gameInfoの取得ができませんでした".format(matchID, sc), file=sys.stderr)
    sys.exit()

dirPath = os.path.dirname(__file__)
dirPath = (dirPath if dirPath else '.') + "/../data/{0}".format(matchID)
if not os.path.exists(dirPath):
    os.mkdir(dirPath)

if not os.path.exists(dirPath+"/action"):
    with open(dirPath+"/action", mode='w') as f:
        print(0, file=f)

if not os.path.exists(dirPath+"/forcedAction"):
    with open(dirPath+"/forcedAction", mode='w') as f:
        print(0, file=f)

filePath = dirPath + "/gameInfo"

for match in con.json()['matches']:
    if (int(match['matchID']) == int(matchID)):
        with open(filePath, mode='w') as f:
            print(matchID, file=f)
            print(teamID, file=f)
            print(match['turns'], file=f)
            print(match['operationMillis'], file=f)
            print(match['transitionMillis'], file=f)

        print("[makeGameInfo] matchID: {0}, gameInfoの取得が完了しました".format(matchID), file=sys.stderr)
        sys.exit()

print("[makeGameInfo] matchID: {0}, 与えられたmatchIDのmatchが見つかりません".format(matchID), file=sys.stderr)


