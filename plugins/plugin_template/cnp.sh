#! /bin/bash

echo  -e "\033[32;1m"
echo "********** create new MARS plugin **********"
echo -e "\033[0m"
echo -n "insert project name: "
read name

# sed only supports the \u operator since version 4.x and MSYS ships 3.y
if [[ `sed --version | grep -i "version"  | sed -e 's/.*[vV]ersion \([0-9]\).*/\1/g'` == "4" ]]; then
    classname=`sed -e 's/_\([a-z]\)/\u\1/g' -e 's/^\([a-z]\)/\u\1/g' <<< $name`;
elif [[ -x `which perl` ]]; then
    classname=`perl -pe 's/_([a-z])/\u\1/g;' -pe 's/^([a-z])/\u\1/;' <<< $name`;
else
    echo -n "insert class name: "
    read classname
fi

echo -n "insert description: "
read description

echo -n "does it have a GUI? (y/n): "
read gui

#echo -n "insert header define: "
#read header_def
header_def="MARS_PLUGINS_"
header_def=${header_def}"`echo $name | tr '[a-z]' '[A-Z]'`"
header_def=${header_def}"_H"

#echo -n "insert namespace: "
#read namespace
namespace=$name


echo -n "insert author name: "
read author

echo -n "insert author's email: "
read email

if [ $gui = "y" ]; then
  ./create_new_project.sh "$name" "$description" "$header_def" "$namespace" "$author" "$email" "$classname" y
else
  ./create_new_project.sh "$name" "$description" "$header_def" "$namespace" "$author" "$email" "$classname"
fi