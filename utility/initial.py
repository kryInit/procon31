# python initial.py [token] [URL(最初の部分)]
import sys
import time
import requests

if (len(sys.argv) < 3):
    print("[initial] 引数が足りません", file=sys.stderr)
    sys.exit()

usleep = lambda x: time.sleep(x/1000.0)

token = sys.argv[1]
URL = sys.argv[2] + "/teams/me"
header = {"x-api-token": token}

try:
    con = requests.get(URL, headers=header)
    sc = con.status_code
except :
    print("[initial] URL: {0}, サーバーに接続できませんでした".format(URL), file=sys.stderr)
    sys.exit()

cnt = 0
while sc == 429 and cnt < 60:
    usleep(300)
    con = requests.get(URL, headers=header)
    sc = con.status_code
    cnt += 1

if sc != 200:
    print("[initial] status code: {0}, teamIDの取得ができませんでした".format(sc), file=sys.stderr)
    sys.exit()


print("[initial] teamIDの取得が完了しました", file=sys.stderr)
print(con.json()['teamID'])

