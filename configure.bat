rmdir /S /Q build
mkdir c:\install
rmdir /Q build
mkdir build
cd build
cmake ..\libio 
cmake --build .
cmake --install . --config Debug --prefix "c:/install"
::cd ..
::rmdir /S /Q build

::cmake ..\libraw -Dlibio_DIR=c:\install\lib\cmake\libio\






