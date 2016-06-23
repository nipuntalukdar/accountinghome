#!/bin/bash

check() {
    if [ $? -ne 0 ] ; then 
        echo "Something failed "
        exit $?
    fi
}

autoreconf --force --install 
check
chmod 0744 configure
./configure
check
automake
check
tput bold
echo "Now run"
echo "              make install     "
echo "to install the program"
echo "or run"
echo "              make uninstall   "
echo "to uninstall the program"
tput sgr0
