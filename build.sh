echo "export HL2SDKCS2=/home/metamod/hl2sdk-root/hl2sdk-cs2" >> ~/.bashrc
echo "export MMSOURCE112=/home/metamod/metamod-source" >> ~/.bashrc
. ~/.bashrc

rm -R build
mkdir build && cd build
CC=clang CXX=clang++ python3 ../configure.py -s cs2 --targets=x86_64 --plugin-name=fake_rcon --plugin-alias=fake_rcon --enable-optimize
ambuild
