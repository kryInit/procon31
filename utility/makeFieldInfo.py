# python makeFieldInfo.py [token] [URL(最初の部分)] [teamID] [matchID] 
import os
import sys
import time
import requests

if (len(sys.argv) < 5):
    print("[makeFieldInfo] 引数が足りません", file=sys.stderr)
    sys.exit()

usleep = lambda x: time.sleep(x/1000.0)

token = sys.argv[1]
teamID = sys.argv[3]
matchID = sys.argv[4]
URL = sys.argv[2] + "/matches/{0}".format(matchID)
header = {"x-api-token": token}

con = requests.get(URL, headers=header)
sc = con.status_code

if sc == 425:
    usleep((int(con.headers['retry-after'])-1)*1000)

cnt = 0
while (sc == 400 or sc == 425 or sc == 429) and cnt < 60:
    usleep(300);
    con = requests.get(URL, headers=header)
    sc = con.status_code
    cnt += 1

if sc != 200:
    print("[makeFieldInfo] matchID: {0}, status code: {1}, fieldInfoの取得ができませんでした".format(matchID, sc), file=sys.stderr)
    sys.exit()

filePath = os.path.dirname(__file__)
filePath = (filePath if filePath else '.') + "/../data/{0}/fieldInfo".format(matchID)

with open(filePath, mode='w') as f:
    conj = con.json()
    walls = conj['walls']
    areas = conj['areas']
    points = conj['points']
    teams  = conj['teams']
    actions = conj['actions']

    print(conj['height'], conj['width'], conj['turn'], conj['startedAtUnixTime'], file=f);

    for wall in walls:
        print(*wall, file=f)

    for area in areas:
        print(*area, file=f)

    for point in points:
        print(*point, file=f)

    for i in range(len(teams)):
        team = teams[i]
        if ((i == 0 and int(teams[i]['teamID']) != int(teamID)) or (i == 1 and int(teams[i]['teamID']) == int(teamID))):
            team = teams[(i^1)]
        print(team['teamID'], team['agent'], file=f)
        print(team['areaPoint'], team['wallPoint'], file=f)

        agents = team['agents']
        for agent in agents:
            print(agent['agentID'], agent['x']-1, agent['y']-1, file=f)

    # print(len(actions), file=f)

    # for action in actions:
    #     print(action['x']-1, action['y']-1, action['agentID'], action['turn'], action['apply'], action['type'], file=f)

        
print("[makeFieldInfo] matchID: {0}, fieldInfoの取得が完了しました".format(matchID), file=sys.stderr)

