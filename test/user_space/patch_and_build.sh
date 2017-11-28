#!/bin/sh
source ./config
MC_HOME=${SRC_PATH}
cd $MC_HOME

if [ $# -eq 1 ]; then
	PATCH_NAME=$1
	git reset --hard HEAD
	patch -p1 < ${TEST_DIR}/patch/${PATCH_NAME}
else 
	git reset --hard HEAD
fi
cd $MC_HOME
make clean
./configure --prefix=${MCMOD_DIR} --with-target=smp-x86 --with-mpi=/usr/lib64/mpich-3.2 --enable-qlmpi $* >/tmp/install.log
make >> /tmp/install.log
make install >> /tmp/install.log

# for wallaby
chmod 777 ${MCMOD_DIR}/etc

