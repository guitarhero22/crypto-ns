Remove-Item -r build
mkdir build
Set-Location build
cmake -G Ninja .. ; cmake --build . ; ./crypto-ns.exe 10 1000
Set-Location ..