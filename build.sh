#!/usr/bin/env bash


####### ARK Builder Script ##########
#                                   #
# Author  : Krazynez                #
#                                   #
# Date    : 2022-03-23              #
#                                   #
# Script Version : 0.3              #
#                                   #
#####################################

# Usually I do this but to keep file permissions sane I will avoid running as root until needed 
#if [[ $EUID -ne 0 ]] ; then
#    printf "ERROR: Need to run as root!\n"
#    exit 1;
#fi
export PSPDEV=/usr/local/pspdev && export PATH=$PATH:$PSPDEV/bin 

function elevatePrivs {
	if [[ ! '/usr/bin/dialog' ]] ; then
		eval sudo "$@"
	else
		t=$(dialog --insecure --passwordbox "Password" 10 25  3>&1 1>&2 2>&3)
		printf $t > $(pwd)/pass.txt
		sudo -S "$@" <pass.txt
		rm -rf pass.txt
	fi
}


export -f elevatePrivs

function original {
	read -p "
	This script will setup the correct SDK to build ARK, get sign_np
	dependency and temporarly setup the enivorment to build ARK-4. 
	
	Press Enter to continue..."
	
	
	if [ -d "/usr/local/pspdev" ] ; then
		clear
	    read -p "You seem to already have the SDK installed. Do you want to reinstall or continue? (y)es/(n)o/(c)ontinue 
	
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
		elevatePrivs chown -R $USER:$USER $PSPDEV 
	
	    # Signs eboots needed for ARK Loader
	    if [[ ! -f $PSPDEV/bin/sign_np ]] ; then
	        git clone https://github.com/swarzesherz/sign_np.git
	
	        pushd sign_np
	
	        eval make
	
	        mv sign_np $PSPDEV/bin
	
	        popd 
	
	        rm -rf sign_np
	
	    fi

		if [[ ${BASH_ARGV} == "--debug" ]] ; then
			dialog --colors --title "\Z1DEBUG ISSUE\Z0" --infobox "\nCurrently DEBUG mode is not working properly. I Will re-add when working.\n\nRunning normal \`make\`" 10 50
			sleep 2;
			dialog --clear
			#eval make CFLAGS=-DDEBUG=1
			eval make

		else	
	    	eval make
		fi
}

export -f original


function withDialog {
	if [[ ! -f '/usr/bin/dialog' ]] ; then
		original
		exit 0;
	fi

	dialog \
		--title "Welcome to the ARK Compiler" \
		--backtitle "Script created by Krazynez (W.I.P)" \
		--msgbox "This script will setup the correct SDK to build ARK, get sign_np dependency and temporarly setup the enivorment to build ARK-4." 10 80 

$
	dialog 	--title "Checking for existitng SDK" 

	if [[ -d "/usr/local/pspdev" ]] ; then
		response=$(dialog \
			--title "EXISITING PSDSDK!" \
			--no-cancel \
			--radiolist \
			"Choose an Option" \
			15 80 15  \
			1 "Reinstall" "off" \
			2 "Continue" "off" \
			3 "Quit?" "on" 3>&1 1>&2 2>&3)


		case $response in
			1)
			 	elevatePrivs rm -rf /usr/local/pspdev && sudo 7z x ./contrib/PC/PSPSDK/pspdev.7z -o"/usr/local" && sudo chown -R $USER:$USER $PSPDEV ;;
			2)
				elevatePrivs chown -R $USER:$USER $PSPDEV ;;
			*)
				dialog --infobox "Exiting...\n\nPlease Wait..." 5 20 && sleep 2 && dialog --clear && exit 0 ;;
		esac
	else
		elevatePrivs 7z x ./contrib/PC/PSPSDK/pspdev.7z -o"/usr/local" && sudo chown -R $USER:$USER $PSPDEV
	fi

	if [[ ! -f "/lib/libmpfr.so.4" ]] ; then
		if [[ -f "/lib/libmpfr.so" ]] ; then
	         elevatePrivs ln -s /lib/libmpfr.so /lib/libmpfr.so.4
	      elif [[ -f "/lib/libmpfr.so*" ]] ; then 
	         elevatePrivs ln -s /lib/libmpfr.so* /lib/libmpfr.so.4
	      else
	          dialog --colors --title "\Zb\Z1 ! ERROR !\ZB" --infobox "libmpfr is not installed. Please install before continuing.\n" 10 40
			  sleep 3;
			  dialog --clear
	          exit 1;
	    fi
	fi
	
	if [[ ! -f $PSPDEV/bin/sign_np ]] ; then
		git clone https://github.com/swarzesherz/sign_np.git #| dialog --progressbox "Cloning sign_np" 10 20 
		dialog --clear
		
		pushd sign_np

		eval make

		mv sign_np $PSPDEV/bin

		popd

		rm -rf sign_np
	fi
	
	if [[ ${BASH_ARGV} == "--debug" ]] ; then
			eval make DEBUG=1
	else	
	    	eval make
	fi


}

export -f withDialog

withDialog
