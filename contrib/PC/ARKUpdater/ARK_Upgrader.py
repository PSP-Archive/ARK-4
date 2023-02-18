#!/usr/bin/env python3
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox as mb
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
    tmp = tempfile.gettempdir()

def psp() -> int:
    try:
        if platform.system() == 'Linux':
            product = usb.core.find(idVendor=0x054c, idProduct=0x01c8).product
            manufacturer = usb.core.find(idVendor=0x054c, idProduct=0x01c8).manufacturer
            if " ".join((manufacturer, product[:3])) == 'Sony PSP':
                return 0 
        else:
            return 0
    except:
        return 1

def new_version(psp_path, advanced_vsh=None) -> None:
    wy = tk.Label(root, text="Copying files! Please Wait!")
    wy.grid(column=1, row=1)

    os.chdir('ARK')

    shutil.copytree('./ARK_01234', f'{psp_path}/PSP/SAVEDATA/ARK_01234', dirs_exist_ok=True)
    shutil.copytree('./ARK_Live', f'{psp_path}/PSP/GAME/ARK_Live', dirs_exist_ok=True)
    if advanced_vsh != 0:
        shutil.copyfile('./AdvancedVSH/VSHMENU.PRX', f'{psp_path}/PSP/SAVEDATA/ARK_01234/VSHMENU.PRX')

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


def dropdown_update(val, advanced_vsh=None) -> None:
    dropdown_val = val
    if dropdown_val is not None:
        os.chdir(dropdown_val)
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

                if md5 != r_md5:
                    wx = tk.Label(root, text="Newer version available", bg="#0c0",fg="#fff")
                    wx.grid(column=0, row=1, padx=10)
                    new_version(dropdown_val, advanced_vsh)
                else:
                    wx = tk.Label(root, text="You have the latest version available.", bg='#0c0', fg='#fff')
                    wx.grid(column=0, row=1)
                    return
            elif install == 'yes':
                download_latest_ARK()
                new_version(dropdown_val, advanced_vsh)

        else:
            err = tk.Label(root, text='ERR: INCORRECT DRIVE!!!!', fg='#f00', bg='#000')
            err.grid(column=0, row=1)

# For Refresh Button
def _refresh(win) -> None:
    win.destroy()
    if platform.system() == 'Linux':
        os.execv(sys.argv[0], sys.argv)
    else:
        subprocess.call(["python", __file__])

# List for Drives
def options(win=None) -> str:
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
        cb = tk.IntVar()
        z = tk.Checkbutton(win, text='Advanced VSH Menu', variable=cb, onvalue=1, offvalue=0)
        z.grid(column=4, row=1)

        w = tk.OptionMenu(win, _default, *final, command=lambda x : dropdown_update(x, cb.get()))
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
        cb = tk.IntVar()
        z = tk.Checkbutton(win, text='Advanced VSH Menu', variable=cb, onvalue=1, offvalue=0)
        z.grid(column=4, row=1)

        w = tk.OptionMenu(win, _default, *final, command=lambda x : dropdown_update(x, cb.get()))
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
    root.geometry('800x100+50+50')
    check = psp()
    frame = ttk.Frame(root)
    frame.grid(column=0, row=0)
    refresh_btn = ttk.Frame(root)
    refresh_btn.grid(column=1, row=0)

    refresh = tk.Button(refresh_btn, text='Refresh Devices', command=lambda: _refresh(root))
    refresh.pack()

    if check == 0:
        psp_detected = tk.Label(frame, text="PSP DETECTED!")
        psp_detected.pack(pady=2, padx=5)
    
        options(root)


    else:
        psp_not_detected = tk.Label(frame, text='No Device detected,\nmake sure your device is in USB Mode')
        psp_not_detected.pack(pady=2, padx=5)



    # Runs everything
    root.mainloop()
   


if __name__ == '__main__':
    main()
