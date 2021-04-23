SERVER=192.168.172.236
SERVER_PATH=/home/tim/MarketAnalysis/run/bsdr_analysis/output/
SERVER_USERNAME=tim

DEST_FOLDER=../run/bsdr_analysis/output/

echo "get data from ${SERVER_PATH}"

scp -r ${SERVER_USERNAME}@${SERVER}:${SERVER_PATH}* ${DEST_FOLDER}