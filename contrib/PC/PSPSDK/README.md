- Extract to `/usr/local/`
- Install dependencies: `sudo apt-get install autoconf automake bison flex gcc libmpfr-dev 
                         libncurses5-dev libreadline-dev libusb-dev make patch subversion texinfo wget`
- Link missing file: `sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4`
- Install [sign_np](https://github.com/swarzesherz/sign_np/releases/tag/v1.0.2) to /usr/local/pspdev/bin
