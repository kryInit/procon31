## visualizerを使う場合  
1 `/visualizer/src/visualizer` 内のコードからsiv3Dのapplicationを作成, `/visualizer/visualizer.app`として配置  
2 `/visualizer/src/simpleServer` 内のコードから(compile.shを使って)simpleServerを作成, `/visualizer/simpleServer`として配置  
3 `/visualizer/visualizer.app`を開く  
  
## solverとplayMatchを作る  
1 `mkdir algo/src/build`  
2 `cd algo/src/build/`  
3 `cmake ..`  
4 `make`  
  
## 競技
`./procon31.sh [token] [URL]`で全部いける(はず...)  
詳しくいうと/teams/meからteamIDを取得、/teams/{teamID}/matchesから見れる試合全てに対してplayMatchを実行している
出力が多いので` 2> errorLog`とかつけると良いかも  

### visualizerを使いたい
`procon31.sh`の実行でvisualizerは起動すると思うので、あとは最初の方に出力されているmatchIDを入力してregisterボタン(or ctrl+rとか非入力状態でrとか)でmatchIDを登録、tabで切り替えて見ることができる  
実はdualshock4が使えたり使えなかったりする(他のコントローラーはどうなるかわからない)  

  
