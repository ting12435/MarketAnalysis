#!/bin/bash

# only for linux

# usage:
#	sh get_data.sh
#	sh get_data.sh TSE 20210127
#	sh get_data.sh TSE 20210127 20210202

if [ "$#" = 2 ]
then
	MARKET=$1
	# tmp=$2
	# ST_DATE_STR=${tmp:0:4}-${tmp:4:2}-${tmp:6:2}
	# ED_DATE_STR=${tmp:0:4}-${tmp:4:2}-${tmp:6:2}
	ST_DATE=$2
	ED_DATE=$2
elif [ "$#" = 3 ]
then
	MARKET=$1
	# tmp=$2
	# ST_DATE_STR=${tmp:0:4}-${tmp:4:2}-${tmp:6:2}
	# tmp=$3
	# ED_DATE_STR=${tmp:0:4}-${tmp:4:2}-${tmp:6:2}
	ST_DATE=$2
	ED_DATE=$3
else
	echo "usage:"
	echo " e.g."
	echo "  sh get_data.sh TSE 20210127"
	echo "  sh get_data.sh TSE 20210127 20210202"
	exit
fi

get_date() {
	# date --date="$1" +"%Y%m%d"
	date -d "$1" +"%Y%m%d"
}

####
# get_date() {
#     date --utc --date="$1" +"%Y-%m-%d %H:%M:%S"
# }
# [[ $(get_date "2014-12-02T14:00:00+00:00") < $(get_date "2014-12-01T12:00:00-05:00") ]] && echo it works
# [[ $(date --utc --date="2014-12-01T14:00:00+00:00" +"%Y-%m-%d %H:%M:%S") < $(date --utc --date="2014-12-01T12:00:00-05:00" +"%Y-%m-%d %H:%M:%S") ]] && echo it works
# if [[ $(date --utc --date="2014-12-02T14:00:00+00:00" +"%Y-%m-%d %H:%M:%S") < $(date --utc --date="2014-12-01T12:00:00-05:00" +"%Y-%m-%d %H:%M:%S") ]] 
# if [[ $(date --date="2014-11-30" +"%Y-%m-%d") < $(date --date="2014-12-01" +"%Y-%m-%d") ]] 
# if [[ $(date --date=$ST_DATE_STR +"%Y%m%d") < $(date --date=$ED_DATE_STR +"%Y%m%d") ]] 
# then
# 	echo "aaa"
# else
# 	echo "bbb"
# fi
###

# echo $(date --date=$ST_DATE_STR +"%Y%m%d")
# echo $(date -d $ST_DATE_STR" + 0 days" +"%Y%m%d")
# echo $(date -d $ST_DATE_STR" + 1 days" +"%Y%m%d")
# echo $(date -d $ST_DATE_STR" + 30 days" +"%Y%m%d")

echo "get data from" $ST_DATE "to" $ED_DATE

# if [[ $(get_date $ST_DATE) > $(get_date $ED_DATE) ]]
# if [[ $(get_date $ST_DATE) -le $(get_date $ED_DATE) ]]
# then
# 	echo "true"
# else
# 	echo "false"
# fi

SERVER=192.168.172.251
SERVER_USERNAME=tim

# SERVER_PATH=/data/database/daily_fetch/${MARKET}/${DATA_DATE}/RPT/BIG5
DEST_FOLDER=../data/BuySellDailyReport/${MARKET}

CUR_DATE=$ST_DATE
while [[ $(get_date $CUR_DATE) -le $(get_date $ED_DATE) ]]
do
	# echo $CUR_DATE
	SERVER_PATH=/data/database/daily_fetch/${MARKET}/$(date -d $CUR_DATE +"%Y-%m-%d")/RPT/BIG5/

	echo "get data from ${SERVER_USERNAME}@${SERVER}:${SERVER_PATH}"
	scp -rq ${SERVER_USERNAME}@${SERVER}:${SERVER_PATH} ${DEST_FOLDER}/$(date -d $CUR_DATE +"%Y-%m-%d")

	CUR_DATE=$(date -d $CUR_DATE" + 1 days" +"%Y%m%d")
done






