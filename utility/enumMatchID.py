# python enumMatchID.py [token] [URL(最初の部分)] [teamID]
import sys
import time
import requests

if (len(sys.argv) < 4):
    print("[enumMatchID] 引数が足りません", file=sys.stderr)
    sys.exit()

usleep = lambda x: time.sleep(x/1000.0)

token = sys.argv[1]
teamID = sys.argv[3]
URL = sys.argv[2] + "/teams/{0}/matches".format(teamID)
header = {"x-api-token": token}

con = requests.get(URL, headers=header)
sc = con.status_code

cnt = 0
while sc == 429 and cnt < 60:
    usleep(300)
    con = requests.get(URL, headers=header)
    sc = con.status_code
    cnt += 1

if sc != 200:
    print("[enumMatchID] status code: {0}, enumerate matchIDができませんでした".format(sc), file=sys.stderr)
    sys.exit()

print("[enumMatchID] enumerate matchIDが完了しました", file=sys.stderr)
for match in con.json()['matches']:
    print(match['matchID'])

