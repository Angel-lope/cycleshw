export CYCLES_PORT=3149

cat<<EOF> config.yaml
gameHeight: 700
gameWidth: 700
gameBannerHeight: 100
gridHeight: 100
gridWidth: 100
maxClients: 60
enablePostProcessing: false
EOF

./build/bin/server &
sleep 1

for i in {1..2}
do
./build/bin/client_randomio randomio$i &
done
./build/bin/client_angels angel
