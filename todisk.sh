#killall fs-uae 
mkdir -p ./disk
rm -rf ./disk/*
mkdir -p ./disk/data
cp -r ./data/* ./disk/data/
cp MOD.rubbur.smps ./disk/
cp MOD.rubbur.song ./disk/
#cp rubbur.mod ./disk/
rm ./rubbur.exe
cp a ./rubbur.exe
./tools/mac/exe2adf -i rubbur.exe -a rubbur.adf -d ./disk/ -l RSYNC -t wait.txt -b SpaceBoot.bin