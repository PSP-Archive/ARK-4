#!/usr/bin/env bash


####### ARK Builder Script ##########
#                         			#
# Author  : Krazynez      			#
#                         			#
# Date    : 2022-03-21    			#
#                         			#
#####################################

# Usually I do this but to keep file permissions sane I will avoid running as root until needed 
#if [[ $EUID -ne 0 ]] ; then
#    printf "ERROR: Need to run as root!\n"
#    exit 1;
#fi


function elevatePrivs {
	eval sudo "$@"
}


export -f elevatePrivs


read -p "
This script will setup the correct SDK to build ARK, get sign_np
dependency and temporarly setup the enivorment to build ARK-4. 

Press Enter to continue..."


if [ -d "/usr/local/pspdev" ] ; then
	clear
    read -p "You seem to already have the SDK installed. Do you want to reinstall or continue? (y)/(n)/(c)ontinue 

if you continue ARK will try to build with already installed SDK: " input

    if [[ ! "$input" =~ ^(Y|Yes|YEs|YES|yES|yeS|yes|y|c|C)$ ]] ; then
        printf "Exiting....\n"
		exit 0;
    fi

    if [[ ! -f "/lib/libmpfr.so.4" ]] ; then
        if [[ -f "/lib/libmpfr.so" ]] ; then
            elevatePrivs ln -s /lib/libmpfr.so /lib/libmpfr.so.4
        elif [[ -f "/lib/libmpfr.so*" ]] ; then 
            elevatePrivs ln -s /lib/libmpfr.so* /lib/libmpfr.so.4
        else
            printf "libmpfr is not installed. Please install before continuing.\n"
            exit 1;
        fi
    fi
fi

    if [[ ! $input =~ ^(c|C)$ ]] ; then
        elevatePrivs 7z x ./contrib/PC/PSPSDK/pspdev.7z -o"/usr/local"
    fi

    # Should be added to .bashrc or somthing to make it static, but for now I will leave it just for the session
    export PSPDEV=/usr/local/pspdev
    export PATH=$PATH:$PSPDEV/bin
	elevatePrivs chown -R $USER:$USER $PSPDEV 

    # Signs eboots needed for ARK Loader
    if [[ ! -f $PSPDEV/bin/sign_np ]] ; then
        git clone https://github.com/swarzesherz/sign_np.git

        pushd sign_np

        eval make

        mv sign_np $PSPDEV/bin

        popd 

        rm -rf sign_np

        pushd ARK-4
    fi

    eval make
