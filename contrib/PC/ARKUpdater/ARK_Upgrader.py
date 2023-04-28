#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox as mb
from tkinter import DISABLED
import os
import usb
import hashlib
import platform
import psutil
import requests
import subprocess
import shutil
import sys
import time
import tempfile
import zipfile

root = tk.Tk()

if platform.system() == 'Linux':
    import pyudev
    tmp = '/tmp'
else:
    import win32api
    import win32file
    from device_manager import USBDeviceScanner
    tmp = tempfile.gettempdir()

def psp() -> int:
    try:
        if platform.system() == 'Linux':
            product = usb.core.find(idVendor=0x054c, idProduct=0x01c8).product
            manufacturer = usb.core.find(idVendor=0x054c, idProduct=0x01c8).manufacturer
            if " ".join((manufacturer, product[:3])) == 'Sony PSP':
                return 0 
        elif platform.system() == 'Windows':
            usb_scan = USBDeviceScanner()
            if len(usb_scan.find_devices(vendor_id=0x054c, product_id=0x01c8)) == 1:
                return 0
    except:
        return 1

def new_version(psp_path) -> None:
    wy = tk.Label(root, text="Copying files! Please Wait!")
    wy.grid(column=1, row=1)
    root.update()

    os.chdir('ARK')

    shutil.copytree('./ARK_01234', f'{psp_path}/PSP/SAVEDATA/ARK_01234', dirs_exist_ok=True)
    shutil.copytree('./ARK_Live', f'{psp_path}/PSP/GAME/ARK_Live', dirs_exist_ok=True)

    os.chdir('../')
    shutil.rmtree('ARK')
    os.remove('ARK4.zip')

    wy.config(text="Installed!", bg="#0c0",fg="#fff")

def download_latest_ARK():
    download_latest = requests.get('https://github.com/PSP-Archive/ARK-4/releases/latest')
    ver = download_latest.url.split('/')[-1]
    download_file = requests.get(f'https://github.com/PSP-Archive/ARK-4/releases/download/{ver}/ARK4.zip')
    open(f'{tmp}/ARK4.zip', 'wb').write(download_file.content)
    os.chdir(f'{tmp}')
    if os.path.isdir('ARK'):
        shutil.rmtree('ARK')
    os.mkdir('ARK')
    with zipfile.ZipFile('ARK4.zip', 'r') as ARK:
        ARK.extractall('./ARK/')

def setup_ark_dc(_path=None):
    # TODO: ARK-DC detect that ms0:/TM/DCARK/ exists
    # and then copy ARK_01234 to that folder
    # extract FLASH0.ARK into ms0:/TM/DCARK/kd/
    if not os.path.exists(f'{_path}/TM/DCARK/'):
        os.makedirs(f'{_path}/TM/DCARK/')
    download_latest_ARK()
    if not os.path.exists(f'{_path}/TM/DCARK/ARK_1234'):
        os.makedirs(f'{_path}/TM/DCARK/ARK_1234')
    shutil.copytree(f'{tmp}/ARK/ARK_01234', f'{_path}/TM/DCARK/ARK_1234', dirs_exist_ok=True)
    if not os.path.exists(f'{_path}/TM/DCARK/kd/'):
        os.makedirs(f'{_path}/TM/DCARK/kd/') 
    shutil.copyfile(f'{tmp}/ARK/ARK_01234/FLASH0.ARK', f'{_path}/TM/DCARK/kd/FLASH0.ARK')

def dropdown_update(val, force_upgrade=None, def_text=None, ARKDC=None) -> None:
    def_text.destroy()
    dropdown_val = val
    if dropdown_val is not None:
        os.chdir(dropdown_val)

        # ARK DC Setup
        if ARKDC == 1:
            ark_dc_text = tk.Label(root, text='Copying files for ARK-DC. Please Wait...', bg="#0c0",fg="#fff" ) 
            ark_dc_text.grid(column=1, row=2)
            root.update()
            setup_ark_dc(dropdown_val)
            ark_dc_text.config(text='Done.')
            return

        if os.path.isdir('PSP') and os.path.isdir('SEPLUGINS') or os.path.isdir('psp') and os.path.isdir('seplugins'):
            if not os.path.exists('./PSP/SAVEDATA/ARK_01234'):
                install = mb.askquestion('ARK does not seem to be installed', 'Would you like to install it?')
                if install == 'no':
                    root.destroy()
            if os.path.exists('./PSP/SAVEDATA/ARK_01234'):
                os.chdir('./PSP/SAVEDATA/ARK_01234')
                with open('FLASH0.ARK', 'rb') as local_version:
                    data = local_version.read()
                    md5 = hashlib.md5(data).hexdigest()
                    local_version.close()
                    download_latest_ARK()
                with open(f'{tmp}/ARK/ARK_01234/FLASH0.ARK', 'rb') as remote_version:
                    r_data = remote_version.read()
                    r_md5 = hashlib.md5(r_data).hexdigest()
                    remote_version.close()

                if md5 != r_md5 or force_upgrade == 1:
                    wx = tk.Label(root, text="Newer version available", bg="#0c0",fg="#fff")
                    wx.grid(column=0, row=1, padx=10)
                    root.update()
                    root.after(3000, new_version(dropdown_val))
                else:
                    wx = tk.Label(root, text="You have the latest version available.", bg='#0c0', fg='#fff')
                    wx.grid(column=0, row=1)
                    return
            elif install == 'yes':
                download_latest_ARK()
                root.update()
                root.after(3000, new_version(dropdown_val))

        else:
            err = tk.Label(root, text='ERR: INCORRECT DRIVE!!!!', fg='#f00', bg='#000')
            err.grid(column=0, row=1)

# For Close Button
def _close(win) -> None:
    win.destroy()

# For Refresh Button
def _refresh(win) -> None:
    win.destroy()
    if platform.system() == 'Linux':
        os.execv(sys.argv[0], sys.argv)
    else:
        subprocess.call(["python", __file__])

# List for Drives
def options(win=None, def_text=None) -> str:
    if platform.system() == 'Linux':
        cmd = "lsblk|awk '/[/]run/ || /[/]media/ {print $7}'"
        lst = subprocess.Popen(cmd,shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        lst2 = lst.communicate()[0]
        final = lst2.decode('utf-8').strip()
        final = final.split()
        _default = tk.StringVar()
        _default.set('Select a Drive')
        y = tk.Label(win, text=" "*4)
        y.grid(column=2, row=0)
        x = tk.Label(win, text='Drive:')
        x.grid(column=3, row=0)
        f_cb = tk.IntVar()
        ark_dc_cb = tk.IntVar()
        force = tk.Checkbutton(win, text='Force Upgrade', variable=f_cb, onvalue=1, offvalue=0)
        force.grid(column=4, row=1, sticky="w")
        ARKDC = tk.Checkbutton(win, text='ARK DC', variable=ark_dc_cb, onvalue=1, offvalue=0, state=DISABLED)
        ARKDC.grid(column=4, row=2, sticky="w")

        w = tk.OptionMenu(win, _default, *final, command=lambda x : dropdown_update(x, f_cb.get(), def_text, ark_dc_cb.get()))
        w.grid(column=4, row=0)
    else:
        drives = win32api.GetLogicalDriveStrings()
        final = drives.split('\x00')[:-1]
        final.remove('C:\\')
        _default = tk.StringVar()
        _default.set('Select a Drive')
        y = tk.Label(win, text=" "*4)
        y.grid(column=2, row=0)
        x = tk.Label(win, text='Drive:')
        x.grid(column=3, row=0)
        f_cb = tk.IntVar()
        force = tk.Checkbutton(win, text='Force Upgrade', variable=f_cb, onvalue=1, offvalue=0)
        force.grid(column=4, row=1, sticky="w")
        ARKDC = tk.Checkbutton(win, text='ARK DC', variable=ark_dc_cb, onvalue=1, offvalue=0, state=DISABLED)
        ARKDC.grid(column=4, row=2, sticky="w")

        w = tk.OptionMenu(win, _default, *final, command=lambda x : dropdown_update(x, f_cb.get(), def_text, ark_dc_cb.get()))
        w.grid(column=4, row=0)


def main() -> None:
    if platform.system() != 'Linux' and platform.system() != 'Windows':
        print("Sorry this only works on Linux and Windows currently")
        sys.exit(1)
    if sys.version_info[0] < 3 and sys.version_info[1] < 8:
        print("Sorry this has to be run on Python 3.8+")
        sys.exit(1)
    if platform.system() == 'Windows':
        powershell_check = psutil.Process(os.getppid()).name()
        if powershell_check == 'cmd.exe':
            print('Please run this with powershell. It makes it easier for both you and I. Thanks.')
            sys.exit(1)
        print('Your only seeing this because you choose the wrong OS ;-)')
    print('Running...')
    root.title('ARK-4 Upgrader')
    root.geometry('800x150+50+50')
    check = psp()
    frame = ttk.Frame(root)
    frame.grid(column=0, row=0)
    refresh_btn = ttk.Frame(root)
    refresh_btn.grid(column=1, row=0)
    close_but = ttk.Frame(root)
    close_but.grid(column=4, row=4)
    def_text = tk.Label(root, text="Select options (if wanted)\nthen select a Drive to Start", bg="#ff1", padx=10)
    def_text.grid(column=0, row=1)

    refresh = tk.Button(refresh_btn, text='Refresh Devices', command=lambda: _refresh(root))
    refresh.pack()

    close = tk.Button(close_but, text='Close', command=lambda: _close(root))
    close.pack()

    if check == 0:
        psp_detected = tk.Label(frame, text="PSP DETECTED!")
        psp_detected.pack(pady=2, padx=5)
    
        options(root, def_text)


    else:
        psp_not_detected = tk.Label(frame, text='No Device detected,\nmake sure your device is in USB Mode')
        psp_not_detected.pack(pady=2, padx=5)



    # Runs everything
    root.mainloop()
   


if __name__ == '__main__':
    main()
