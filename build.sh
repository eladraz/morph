#/bin/sh

if [ -z "$XSTL_PATH" ] ; then
        echo "Please specify XSTL_PATH"
elif [ -z "$PELIB_PATH" ] ; then
        echo "Please specify PELIB_PATH"
elif [ -z "$DISMOUNT_PATH" ] ; then
        echo "Please specify DISMOUNT_PATH"
elif [ -z "$ELFLIB_PATH" ] ; then
        echo "Please specify ELFLIB_PATH"
else
    PWD=`pwd`
    ./autogen.sh && ./configure --prefix=${PWD}/out --enable-debug --enable-unicode --with-xstl=${XSTL_PATH} --with-pelib=${PELIB_PATH} --with-elflib=${ELFLIB_PATH} --with-dismount=${DISMOUNT_PATH} && make -j4 && make install
fi 


