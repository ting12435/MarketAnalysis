#!/bin/bash

# usage:
#	sh get_data.sh
#	sh get_data.sh TSE 20210127

if [ "$#" = 0 ]
then
	MARKET=TSE
	DATA_DATE=2021-01-28
elif [ "$#" = 2 ]
then
	MARKET=$1
	tmp=$2
	DATA_DATE=${tmp:0:4}-${tmp:4:2}-${tmp:6:2}
else
	echo "usage:"
	echo " e.g."
	echo "  sh get_data.sh"
	echo "  sh get_data.sh TSE 20210127"
	exit
fi

SERVER=192.168.172.251
SERVER_PATH=/data/database/daily_fetch/${MARKET}/${DATA_DATE}/RPT/BIG5
SERVER_USERNAME=tim

DEST_FOLDER=${MARKET}

echo "get data from ${SERVER_PATH}"

scp -r ${SERVER_USERNAME}@${SERVER}:${SERVER_PATH} ${DEST_FOLDER}/${DATA_DATE}