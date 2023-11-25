#!/usr/bin/env bash


####### ARK Builder Script ##########
#                                   #
# Author  : Krazynez                #
#                                   #
# Date    : 2023-11-24              #
#                                   #
#####################################
version=0.9.0

if [[ -z ${PSPDEV} ]]; then
	clear
	printf "\nYou don't seem to have PSPDEV setup. Probably should fix that first.\n\n"
	read -p "Would you like to add one now? y/N: " userInput
	if [[ "$userInput" =~ ^(Y|y|yes|YES)$ ]]; then
		printf "\n"
		read -p "PSPDEV path (ex: /usr/local/pspdev): " getPath
		export PSPDEV="$getPath" && export PATH="$PATH:$PSPDEV/bin"
	fi
fi


dialogCheck=$(command -v dialog 2>/dev/null)

function checkDepends {

	sudo apt install -y build-essential mkisofs python3-pip p7zip-full zlib1g-dev libmpfr-dev python3-pycryptodome
	pip3 install ecdsa
}

export -f checkDepends


function elevatePrivs {
	if [[ ! -f '/usr/bin/dialog' ]] ; then
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
	
	if [[ "$1" == "--docker" ]]; then
		clear
		read -p "
		This script will setup the correct SDK to build ARK, get sign_np
		dependency and temporarly setup the enivorment to build ARK-4. 
		
		Press Enter to continue..."
		
		
		if [ -d "$PSPDEV" ] ; then
			clear
			read -p "You seem to already have the SDK installed. Do you want to reinstall or continue? (y)es/(n)o/(c)ontinue 
		
		if you continue ARK will try to build with already installed SDK: " input
		fi
		
		if [[ ! "$input" =~ ^(Y|Yes|YEs|YES|yES|yeS|yes|y|c|C)$ ]] ; then
			printf "Exiting....\n"
			exit 0;
		fi
	fi
	
	if [[ ! -f "/lib/libmpfr.so.4" ]] ; then
		if [[ "$1" == "--docker" ]]; then
			elevatePrivs ln -s '/usr/lib/x86_64-linux-gnu/libmpfr.so.6.1.0' /lib/libmpfr.so.4
		else if [[ -f "/usr/lib/x86_64-linux-gnu/libmpfr.so.4" ]] ; then
			printf "Already Exist\n"
		elif [[ -f "/usr/lib/x86_64-linux-gnu/libmpfr.so" ]] ; then
			elevatePrivs ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so /usr/lib/x86_64-linux-gnu/libmpfr.so.4
	    elif [[ -f "/lib/libmpfr.so" ]] ; then
	        elevatePrivs ln -s /lib/libmpfr.so /lib/libmpfr.so.4
	    elif [[ -f "/lib/libmpfr.so*" ]] ; then 
	        elevatePrivs ln -s /lib/libmpfr.so* /lib/libmpfr.so.4
	    else
	        printf "\nlibmpfr is not installed. Please install before continuing.\n"
	        exit 1;
	        fi
	    fi
	fi

	    if [[ ! $input =~ ^(c|C)$ || "$1" == "--docker" ]] ; then
	        elevatePrivs 7z -aoa x ./contrib/PC/PSPSDK/pspdev.7z -o"${PSPDEV:0:-7}"
	    fi
	
	    # Should be added to .bashrc or somthing to make it static, but for now I will leave it just for the session
		elevatePrivs chown -R $USER:$USER $PSPDEV 

		# downloads mkpsxsio and installs
		if [[ ! -f "/usr/bin/mkpsxiso" ]]; then
				check_curl=$(command -v curl)
				curl_ret=$?
				check_wget=$(command -v wget)
				wget_ret=$?
			if [[ -f "/usr/bin/apt" ]]; then
				if [[ $curl_ret == 0 ]]; then
					$check_curl -OJL "https://github.com/Lameguy64/mkpsxiso/releases/download/v2.03/mkpsxiso-2.03-Linux.deb" && elevatePrivs apt install ./mkpsxiso-2.03-Linux.deb
				elif [[ $wget_ret == 0 ]]; then
					$check_wget -O $(pwd)/mkpsxiso-2.03-Linux.deb "https://github.com/Lameguy64/mkpsxiso/releases/download/v2.03/mkpsxiso-2.03-Linux.deb" && elevatePrivs apt install ./mkpsxiso-2.03-Linux.deb
				fi
			elif [[ -f "/usr/bin/dnf" ]]; then
				if [[ $curl_ret == 0 ]]; then
					$check_curl -OJL "https://github.com/Lameguy64/mkpsxiso/releases/download/v2.03/mkpsxiso-2.03-Linux.rpm" && elevatePrivs dnf install ./mkpsxiso-2.03-Linux.deb
				elif [[ $wget_ret == 0 ]]; then
					$check_wget -O $(pwd)/mkpsxiso-2.03-Linux.deb "https://github.com/Lameguy64/mkpsxiso/releases/download/v2.03/mkpsxiso-2.03-Linux.rpm "&& elevatePrivs dnf install ./mkpsxiso-2.03-Linux.deb
				fi

			fi

		fi



	
	    # Signs eboots needed for ARK Loader
	    if [[ ! -f $PSPDEV/bin/sign_np ]] ; then
	        git clone https://github.com/swarzesherz/sign_np.git
	
	        pushd sign_np
	
	        eval make
	
	        mv sign_np $PSPDEV/bin
	
	        popd 
	
	        rm -rf sign_np
	
	    fi

		if [[ $1 == "--debug" ]] ; then
			clear
			printf "Please Specifiy the debug level you would like:\n"
			printf "1 (enable BSoD and color debugging)\n"
			printf "2 (enable BSoD, color debugger and JAL tracer)\n"
			printf "3 (enable BSoD, color debugger, JAL tracer and file logging)\n"
			read -p "Choice: " debugLevel
			eval make clean
			eval make DEBUG=$debugLevel

		else	
	    	eval make
		fi
}

export -f original


function withDialog {

	# Check for python3 and Make first before moving onwards
	checkDepends 

	if [[ $1 == '-h' || $1 == '--help' ]] ; then
			echo "$0                | Compiles & Builds Release builds"
			echo "$0 --debug        | Allows different levels of debugging"
			echo "$0 --no-cfw-vita  | Automattically downloads Trinity PBOOT.PBP"
			echo "$0 --cfw-vita     | Automattically downloads ArkFast.vpk"
			echo "$0 --clean        | Runs \`make clean\` (in case your path is not setup correctly)"
			exit 0;
	fi

	if [[ $1 == '--clean' ]] ; then
			eval make clean
			exit 0;
	fi

	if [[ ! -f '/usr/bin/dialog' ]] ; then
		original $1
		exit 0;
	fi

	dialog \
		--title "Welcome to the ARK Compiler" \
		--backtitle "Script created by Krazynez Version: $version" \
		--msgbox "This script will setup the correct SDK to build ARK, get sign_np dependency and mkpsxiso and temporarly setup the enivorment to build ARK-4." 10 80 

$
	dialog 	--title "Checking for existitng SDK"

	if [[ -d "$PSPDEV" ]] ; then
		response=$(dialog \
			--title "EXISITING PSPSDK!" \
			--no-cancel \
			--radiolist \
			"Choose an Option" \
			15 80 15  \
			1 "Reinstall" "off" \
			2 "Continue" "off" \
			3 "Quit?" "on" 3>&1 1>&2 2>&3)


		case $response in
			1)
			 	elevatePrivs rm -rf $PSPDEV && sudo 7z x ./contrib/PC/PSPSDK/pspdev.7z -o"${PSPDEV:0:-7}" && sudo chown -R $USER:$USER $PSPDEV ;;
			2)
				elevatePrivs chown -R $USER:$USER $PSPDEV ;;
			*)
				dialog --infobox "Exiting...\n\nPlease Wait..." 5 20 && sleep 2 && dialog --clear && exit 0 ;;
		esac
	else
		elevatePrivs 7z x ./contrib/PC/PSPSDK/pspdev.7z -o"${PSPDEV:0:-7}" && sudo chown -R $USER:$USER $PSPDEV
	fi

	if [[ ! -f "/lib/libmpfr.so.4" ]] ; then
		if [[ -f "/usr/lib/x86_64-linux-gnu/libmpfr.so" ]] ; then
			elevatePrivs ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so /usr/lib/x86_64-linux-gnu/libmpfr.so.4
		elif [[ -f "/lib/libmpfr.so" ]] ; then
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
	
	if [[ $1 == "--debug" ]] ; then
			eval make clean
			debugLevel=$(dialog --colors --radiolist "\Z1DEBUG Types/Levels\Z0" 10 80 3 1 "DEBUG 1 (enable BSoD and color debugging)" on 2 "DEBUG 2 (enable BSoD, color debugger and JAL tracer)" off 3 "DEBUG 3 (enable BSoD, color debugger, JAL tracer and file logging)" off 3>&1 1>&2 2>&3)
			eval make DEBUG=$debugLevel
	else	
	    	eval make
	fi

	if [[ $1 == "--cfw-vita" ]]; then
		check_curl=$(command -v curl)
		curl_ret=$?
		check_wget=$(command -v wget)
		wget_ret=$?

		if [ ! -d "dist/ArkFast" ]; then
			$(command -v mkdir) dist/ArkFast
		fi

	
		# ArkFast
		FastARK="https://github.com/Yoti/ArkFast-4/releases/download/42056/ArkFast_4.20.56.vpk"
		if [[ $curl_ret -eq 0 ]]; then
			${check_curl} -o dist/ArkFast/ArkFast.vpk -JL $FastARK
			exit
		elif [[ $wget_ret -eq 0 ]]; then
			${check_wget} -O dist/ArkFast/ArkFast.vpk $FastARK
			exit
		fi
	fi
}

export -f withDialog

if [ -z "$1" ]; then
	withDialog
else
	withDialog $1
fi
