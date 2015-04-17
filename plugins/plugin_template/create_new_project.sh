#! /bin/bash

gui="n"

if [ $# -eq 8 ]; then
  gui="y"
fi

echo -e "\033[0m"
echo "project:        $1"
echo "description:    $2"
echo "class:          $7"
echo "header define:  $3"
echo "namespace:      $4"
echo "author:         $5"
echo "email:          $6"
echo "use GUI:        $gui"
echo -e "\033[32;1m"
read -p "are the arguments correct? (y/n) " answer

if [ $answer = "y" ]; then
  projDir=../$4
  projName="$1"
  description="$2"
  classname="$7"
  namespace=$4
  headerDef=$3
  author="$5"
  email="$6"
  echo -e "\033[0m"
  echo "copy template ..."
  
  if [ $gui = "y" ]; then
    cp -r __gui_project__ $projDir
    mv $projDir/src/__project___MainWin.h $projDir/src/${classname}_MainWin.h
    mv $projDir/src/__project___MainWin.cpp $projDir/src/${classname}_MainWin.cpp
  else
    cp -r __project__ $projDir
  fi
  
  echo "rename files ..."
  mv $projDir/__project__.pc.in $projDir/$1.pc.in
  mv $projDir/src/__project__.h $projDir/src/${classname}.h
  mv $projDir/src/__project__.cpp $projDir/src/${classname}.cpp

  echo "replace variables ..."

  FILES="${projDir}/CMakeLists.txt
         ${projDir}/manifest.xml
         ${projDir}/src/${classname}.h
         ${projDir}/src/${classname}.cpp
         ${projDir}/src/${classname}_MainWin.h
         ${projDir}/src/${classname}_MainWin.cpp"
  SED_PAIRS="__project__:${projName}
             __classname__:${classname}
             __description__:${description}
             __namespace__:${namespace}
             __author__:${author}
             __email__:${email}
             __headerDef__:${headerDef}"

  for F in ${FILES}; do
      for PAIR in ${SED_PAIRS}; do
          SEARCH_STRING=${PAIR%:*};
          REPLACE_STRING=${PAIR#*:};
          mv "${F}" tmp.txt;
          sed "s/${SEARCH_STRING}/${REPLACE_STRING}/g" tmp.txt > "${F}";
          rm -f tmp.txt;
      done
  done

  echo  -e "\033[32;1m"
  echo "********** done creating project **********"
  echo -e "\033[0m"

else
  echo  -e "\033[31;1m"
  echo "abort"
  echo -e "\033[0m"
fi