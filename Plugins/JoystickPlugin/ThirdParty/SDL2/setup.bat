@echo off

if not exist SDL hg clone http://hg.libsdl.org/SDL

cd SDL
hg update
cd ..

echo "finish."
pause
