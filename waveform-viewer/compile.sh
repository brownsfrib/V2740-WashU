#!/usr/bin/bash

if [ ! $# -gt 0 ]; then
	echo -e "\033[1;31mError: No argument passesd to compile script.\033[0m\n"
	return
fi

compileFile=$(echo -e "$1" | cut -d'.' -f1)
if [ ! -f $compileFile.cxx ]; then
	echo -e "\033[1;31mError: file $compileFile.cxx does not exist - exiting\033[0m\n";
	return
else
	#-std=c++11 ? \
	g++ -o ./bin/$compileFile $compileFile.cxx \
		$(root-config --cflags --glibs) \
		-I$DAQROOT/include \
		-L$DAQROOT/lib -ldataformat -ldaqio -lException -Wl,-rpath=$DAQROOT/lib

	if [ $? != 0 ]; then
		echo -e "\033[5;31mError in compilation of $compileFile.cxx - exiting\033[0m\n"
	else
		echo -e "\033[1;32mCompiled $compileFile.cxx into ./bin/$compileFile\033[0m\n"
	fi

fi

