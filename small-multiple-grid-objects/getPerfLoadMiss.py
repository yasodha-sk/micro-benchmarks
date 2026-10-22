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
def main():
  print('in main')
  if len(sys.argv) < 2:
    print("Error: Please provide at least one argument.")
    sys.exit(1)

  objFileName = sys.argv[1]
  print("Using object dump file ", objFileName)

  dictMiss = getLoadMiss('./grid-llc-script-LLC-miss-func-ip.txt', './grid-llc-script-LLC-miss-func.txt')
  dictLoad = getLoadMiss('./grid-llc-script-LLC-loads-func-ip.txt', './grid-llc-script-LLC-loads-func.txt')
  keysIPLoad = list(dictLoad)
  keysIPMiss  = list(dictMiss)
  result = list(dict.fromkeys(keysIPLoad + keysIPMiss))

  print ( "Inst \t loads \t misses \t percentage" )
  for ipValue in result :
    command = 'grep -B 5 ' +ipValue+ ': '+objFileName+' | grep \/'
    #print (command)
    strCodeLine =''
    try:
      strResult = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT,universal_newlines=True)
    except subprocess.CalledProcessError as grepexc:
      #print("error code ", command, grepexc.returncode, grepexc.output) 
      strCodeLine = ''
    if (strResult !=''):
      strResult = str(strResult.strip())
      if('\n' in strResult):
        arrStrMap= strResult.split('\n')
        strResult = arrStrMap[len(arrStrMap)-1]
      strCodeLine = strResult.split('/').pop()
    if (ipValue in dictMiss) and (ipValue in dictLoad):
      print ( ipValue , dictLoad[ipValue], dictMiss[ipValue], round((float(dictMiss[ipValue])/float(dictLoad[ipValue])*100), 2) , strCodeLine)
    elif (ipValue in dictMiss) :
      print ( ipValue , 0, dictMiss[ipValue], "NA", strCodeLine) 
    elif (ipValue in dictLoad) :
      print ( ipValue , dictLoad[ipValue], 0, 0 , strCodeLine)

if __name__ == "__main__":
  main()
