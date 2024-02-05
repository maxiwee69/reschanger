to add a game add it behind line 12 like this 
```
{"game_name.exe", false}
```
if you want to create a pull request with your game added, 


to compile run 
```
g++ main.cpp -o reschanger.exe -mwindows
```
if you want g++ to add some code optimization which i recommend to do
```
g++ main.cpp -o reschanger.exe -mwindows -O3 -DNDEBUG
```
