#! /bin/bash

echo -e "\033[0m"
echo "project:        $1"
echo "description:    $2"
echo "class:          $7"
echo "header define:  $3"
echo "namespace:      $4"
echo "author:         $5"
echo "email:          $6"
echo -e "\033[32;1m"
echo -n "are the arguments correct? (y/n) "
read answer

if [ $answer = "y" ]; then
  projDir=../$4
  projName=$1
  description=$2
  classname=$7
  namespace=$4
  headerDef=$3
  author=$5
  email=$6
  echo -e "\033[0m"
  echo "copy template ..."
  cp -r __project__ $projDir
  echo "rename files ..."
  mv $projDir/__project__.pc.in $projDir/$1.pc.in
  mv $projDir/src/__project__.h $projDir/src/${classname}.h
  mv $projDir/src/__project__.cpp $projDir/src/${classname}.cpp

  echo "replace variables ..."

  # project name in CMakeLists.txt
  mv $projDir/CMakeLists.txt tmp.txt
  sed "s/__project__/${projName}/g" tmp.txt > $projDir/CMakeLists.txt
  rm -f tmp.txt

  # class name in CMakeLists.txt
  mv $projDir/CMakeLists.txt tmp.txt
  sed "s/__classname__/${classname}/g" tmp.txt > $projDir/CMakeLists.txt
  rm -f tmp.txt

  # description in CMakeLists.txt
  mv $projDir/CMakeLists.txt tmp.txt
  sed "s/__description__/${description}/g" tmp.txt > $projDir/CMakeLists.txt
  rm -f tmp.txt

  # project name in manifest.xml
  mv $projDir/manifest.xml tmp.txt
  sed "s/__project__/${projName}/g" tmp.txt > $projDir/manifest.xml
  rm -f tmp.txt

  # description in manifest.xml
  mv $projDir/manifest.xml tmp.txt
  sed "s/__description__/${description}/g" tmp.txt > $projDir/manifest.xml
  rm -f tmp.txt

  # project name in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__project__/${projName}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # class name in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__classname__/${classname}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # description in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__description__/${description}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # namespace in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__namespace__/${namespace}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # author in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__author__/${author}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # email in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__email__/${email}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # header definition in .h
  mv $projDir/src/${classname}.h tmp.txt
  sed "s/__headerDef__/${headerDef}/g" tmp.txt > $projDir/src/${classname}.h
  rm -f tmp.txt

  # project name in .cpp
  mv $projDir/src/${classname}.cpp tmp.txt
  sed "s/__project__/${projName}/g" tmp.txt > $projDir/src/${classname}.cpp
  rm -f tmp.txt

  # project name in .cpp
  mv $projDir/src/${classname}.cpp tmp.txt
  sed "s/__classname__/${classname}/g" tmp.txt > $projDir/src/${classname}.cpp
  rm -f tmp.txt

  # description in .cpp
  mv $projDir/src/${classname}.cpp tmp.txt
  sed "s/__description__/${description}/g" tmp.txt > $projDir/src/${classname}.cpp
  rm -f tmp.txt

  # namespace in .cpp
  mv $projDir/src/${classname}.cpp tmp.txt
  sed "s/__namespace__/${namespace}/g" tmp.txt > $projDir/src/${classname}.cpp
  rm -f tmp.txt

  # author in .cpp
  mv $projDir/src/${classname}.cpp tmp.txt
  sed "s/__author__/${author}/g" tmp.txt > $projDir/src/${classname}.cpp
  rm -f tmp.txt

  # email in .cpp
  mv $projDir/src/${classname}.cpp tmp.txt
  sed "s/__email__/${email}/g" tmp.txt > $projDir/src/${classname}.cpp
  rm -f tmp.txt

  echo  -e "\033[32;1m"
  echo "********** done creating project **********"
  echo -e "\033[0m"

else
  echo  -e "\033[31;1m"
  echo "abort"
  echo -e "\033[0m"
fi