# python comms.py [token] [URL(最初の部分)] [teamID] [matchID] 
# teamIDは使わないけどmake*Info.pyと合わせるために
import os
import sys
import time
import json
import requests

if (len(sys.argv) < 5):
    print("[comms] 引数が足りません", file=sys.stderr)
    sys.exit()

usleep = lambda x: time.sleep(x/1000.0)

token = sys.argv[1]
# teamID = sys.argv[3]
matchID = sys.argv[4]
URL = sys.argv[2] + "/matches/{}/action".format(matchID)
headers = {"x-api-token": token, "Content-Type": "application/json"}
data = {"actions": []}
acted = set()

dataPath = os.path.dirname(__file__)
dataPath = (dataPath if dataPath else '.') + "/../data/{0}".format(matchID)

with open(dataPath+"/gameInfo", mode='r') as f:
    con = f.readlines()
    operationMillis = float(con[3])
    transitionMillis = float(con[4])
with open(dataPath+"/fieldInfo", mode='r') as f:
    con = f.readlines()[0].split()
    turn = float(con[2])
    startedAtUnixTime = float(con[3])

t = max(0., startedAtUnixTime+((operationMillis+transitionMillis)/1000.)*turn + operationMillis/1000. - 0.5 - time.time())*1000.
usleep(t)

with open(dataPath+"/forcedAction", mode='r') as f:
    con = f.readlines()
    for l in con[1:]:
        ll = l.split()
        if not int(ll[0]) in acted:
            acted.add(int(ll[0]))
            data['actions'].append({'agentID': int(ll[0]), 'x': int(ll[1])+1, 'y': int(ll[2])+1, 'type': (ll[3] if str(ll[3]) != 'set' else 'put')})

with open(dataPath+"/action", mode='r') as f:
    con = f.readlines()
    for l in con[1:]:
        ll = l.split()
        if not int(ll[0]) in acted:
            acted.add(int(ll[0]))
            data['actions'].append({'agentID': int(ll[0]), 'x': int(ll[1])+1, 'y': int(ll[2])+1, 'type': (ll[3] if str(ll[3]) != 'set' else 'put')})

response = requests.post(URL, data=json.dumps(data), headers=headers)
sc = response.status_code

cnt=0
while (sc == 400 or sc == 429) and cnt < 60:
    response = requests.post(URL, data=data, headers=headers)
    sc = response.status_code
    cnt += 1

if sc == 201 or sc == 202:
    print("[comms] Actionの送信が完了しました", file=sys.stderr)
    with open(dataPath+"/action", mode='w') as f:
        print(0, file=f)
    with open(dataPath+"/forcedAction", mode='w') as f:
        print(0, file=f)
else:
    print("[comms] status code: {}, Actionの送信が行えませんでした".format(sc), file=sys.stderr)

nt = max(0., startedAtUnixTime+((operationMillis+transitionMillis)/1000.)*(turn+1)  - 1.0 - time.time())*1000.
usleep(nt)

