import re
import sys
import os
import subprocess
import argparse
import string

numDebug =0
def getLoadMiss(strIPFile, strPerfScriptFile):
  print('Running match for ', strIPFile )
  dictFunIPCount = {}

  with open(strIPFile) as f:
    for fileLine in f:
        dictFunIPCount[fileLine.strip()] = 0 
  f.close()
  #print (dictFunIPCount)

  with open(strPerfScriptFile) as f:
    for fileLine in f:
      data=fileLine.strip().split(' ')
      data = [x for x in re.split(r"\s+",fileLine) if x]
      data=fileLine.strip().split()
      #print (data[3], data[5])
      strIP = data[5]
      if strIP in dictFunIPCount:
        dictFunIPCount[strIP] = int(dictFunIPCount[strIP]) + int(data[3])
        #print (data[3], strIP, dictFunIPCount[strIP])
  f.close()
  #print (dictFunIPCount)
  # Sort by value
  sorted_dict = dict(sorted(dictFunIPCount.items(), key=lambda item: item[1], reverse=True))

  #print(sorted_dict)
  #for key, value in sorted_dict.items():
  #  print(f"{key}: {value}")
  top_20 = dict(sorted(dictFunIPCount.items(), key=lambda item: item[1], reverse=True)[:20])
  #print(top_20)
  #for key, value in top_20.items():
  #  print(f"{key}: {value}")
  return sorted_dict

# getLoadMiss(strIPFile, strPerfScriptFile)
dictMiss = getLoadMiss('./grid-llc-script-LLC-miss-func-ip.txt', './grid-llc-script-LLC-miss-func.txt')
dictLoads = getLoadMiss('./grid-llc-script-LLC-loads-func-ip.txt', './grid-llc-script-LLC-loads-func.txt')
keysIPLoads = list(dictLoads)
print ( "Inst \t loads \t misses \t percentage" )
for ipValue in keysIPLoads :
    if (ipValue in dictMiss):
      print ( ipValue , dictLoads[ipValue], dictMiss[ipValue], round((float(dictMiss[ipValue])/float(dictLoads[ipValue])*100), 2) )
