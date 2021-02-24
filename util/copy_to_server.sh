#!/bin/bash


rsync -rav -e ssh --exclude='data/*' ../../MarketAnalysis/ tim@192.168.172.236:/home/tim/MarketAnalysis/