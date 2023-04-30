#!/usr/bin/env bash

# PSPSDK installer script by Yoti for ARK-4 project
# 2022-07-01: initial release
# 2022-07-03: python2 + chown
# 2022-09-05: use local file
# 2022-12-19: fix for chown

if [ -d "/usr/local/pspdev" ]; then
    echo "Error: PSPSDK is already installed!"
    echo "You may remove it using \"sudo rm -rf /usr/local/pspdev\" command"
    exit
fi

sudo apt update
pkgs="autoconf automake bison flex gcc libmpfr-dev libncurses5-dev libreadline-dev libusb-dev make p7zip-full patch python3 subversion texinfo wget zlib1g-dev"
for pkg in $pkgs; do
    echo "Installing $pkg..."
    sudo apt install $pkg -y
done

if [ ! -f "./pspdev.7z" ]; then
    link="https://github.com/PSP-Archive/ARK-4/raw/main/contrib/PC/PSPSDK/pspdev.7z"
    #7z program can't read 7z archives from stdin
    #wget -qO- $link | 7z x -si -o/usr/local/
    wget -q --show-progress $link
    sudo 7z x pspdev.7z -o/usr/local/
    rm pspdev.7z
else
    sudo 7z x pspdev.7z -o/usr/local/
fi
sudo chown -R $USER:$USER /usr/local/pspdev/

if [ -f "/usr/lib/x86_64-linux-gnu/libmpfr.so.6" ]; then
    if [ ! -f "/usr/lib/x86_64-linux-gnu/libmpfr.so.4" ]; then
        sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4
    else
        echo "Done: libmpfr.so.4 is present!"
    fi
else
    echo "Error: libmpfr.so.6 is missing!"
fi

sign="https://github.com/swarzesherz/sign_np"
git clone $sign
if [ -d "sign_np" ]; then
    cd sign_np
    make
    sudo cp sign_np /usr/local/pspdev/bin/
    cd ..
    rm -rf sign_np
else
    echo "Error: can\'t download sign_np!"
fi

if [ -z "$PSPDEV" ]; then
    echo "Error: You must add PSPSDK to the \$PATH!"
    echo "Add this lines to your .bashrc..."
    echo "export PSPDEV=/usr/local/pspdev"
    echo "export PATH=\$PATH:\$PSPDEV/bin"
    echo "...and then restart your terminal!"
fi

echo "Done!"
