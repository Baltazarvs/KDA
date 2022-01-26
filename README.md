# KD Calculator
Use this program to calculate your K/D or KDA ratio in games.

# Building
<kbd>
  windres resource.rc -o resource.o <br>
  g++ -o KDAR Source.cpp resource.o -std=c++17 -lgdi32 -mwindows
</kbd>
<img src="https://i.imgur.com/mmqAn4h.png"/>

  - -std=c++17 is optional, use 11 or 14 if you want.
  - KDAR is name of executable. It must be KDAR if you want visual styles enabled.
