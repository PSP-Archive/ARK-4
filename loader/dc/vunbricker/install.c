#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <pspreg.h>
#include <psprtc.h>
#include <psputils.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <pspdecrypt.h>
#include <pspipl_update.h>
#include <vlf.h>

#include <ipl_block_large.h>
#include <ipl_block_01g.h>

#include <ark.h>
#include "dcman.h"
#include "main.h"
#include "install.h"

extern u8 *big_buffer;
extern u8 *sm_buffer1, *sm_buffer2;
extern int status, progress_text, progress_bar;

const char *f0_common[] =
{
    "codepage/cptbl.dat",
    "data/cert/CA_LIST.cer",
    "dic/apotp.dic",
    "dic/atokp.dic",
    "dic/aux0.dic",
    "dic/aux1.dic",
    "dic/aux2.dic",
    "dic/aux3.dic",
    "font/gb3s1518.bwfon",
    "font/imagefont.bin",
    "font/jpn0.pgf",
    "font/kr0.pgf",
    "font/ltn0.pgf",
    "font/ltn1.pgf",
    "font/ltn2.pgf",
    "font/ltn3.pgf",
    "font/ltn4.pgf",
    "font/ltn5.pgf",
    "font/ltn6.pgf",
    "font/ltn7.pgf",
    "font/ltn8.pgf",
    "font/ltn9.pgf",
    "font/ltn10.pgf",
    "font/ltn11.pgf",
    "font/ltn12.pgf",
    "font/ltn13.pgf",
    "font/ltn14.pgf",
    "font/ltn15.pgf",
    "kd/amctrl.prx",
    "kd/audio.prx",
    "kd/audiocodec_260.prx",
    "kd/avcodec.prx",
    "kd/cert_loader.prx",
    "kd/chkreg.prx",
    "kd/chnnlsv.prx",
    "kd/clockgen.prx",
    "kd/codepage.prx",
    "kd/ctrl.prx",
    "kd/dmacman.prx",
    "kd/exceptionman.prx",
    "kd/g729.prx",
    "kd/ge.prx",
    "kd/http_storage.prx",
    "kd/idstorage.prx",
    "kd/ifhandle.prx",
    "kd/init.prx",
    "kd/interruptman.prx",
    "kd/iofilemgr.prx",
    "kd/iofilemgr_dnas.prx",
    "kd/irda.prx",
    "kd/isofs.prx",
    "kd/led.prx",
    "kd/lfatfs.prx",
    "kd/lflash_fatfmt.prx",
    "kd/libaac.prx",
    "kd/libasfparser.prx",
    "kd/libatrac3plus.prx",
    "kd/libaudiocodec2.prx",
    "kd/libdnas.prx",
    "kd/libdnas_core.prx",
    "kd/libgameupdate.prx",
    "kd/libhttp.prx",
    "kd/libmp3.prx",
    "kd/libmp4.prx",
    "kd/libparse_http.prx",
    "kd/libparse_uri.prx",
    "kd/libssl.prx",
    "kd/libupdown.prx",
    "kd/loadcore.prx",
    "kd/lowio.prx",
    "kd/mcctrl.prx",
    "kd/me_wrapper.prx",
    "kd/mediaman.prx",
    "kd/mediasync.prx",
    "kd/memab.prx",
    "kd/mgr.prx",
    "kd/mgvideo.prx",
    "kd/mlnbridge.prx",
    "kd/mlnbridge_msapp.prx",
    "kd/modulemgr.prx",
    "kd/mp4msv.prx",
    "kd/mpeg.prx",
    "kd/mpeg_vsh.prx",
    "kd/mpegbase_260.prx",
    "kd/msaudio.prx",
    "kd/msstor.prx",
    "kd/np.prx",
    "kd/np_auth.prx",
    "kd/np_campaign.prx",
    "kd/np_commerce2.prx",
    "kd/np_commerce2_regcam.prx",
    "kd/np_commerce2_store.prx",
    "kd/np_core.prx",
    "kd/np_inst.prx",
    "kd/np_matching2.prx",
    "kd/np_service.prx",
    "kd/np9660.prx",
    "kd/npdrm.prx",
    "kd/openpsid.prx",
    "kd/popsman.prx",
    "kd/psheet.prx",
    "kd/pspnet.prx",
    "kd/pspnet_adhoc.prx",
    "kd/pspnet_adhoc_auth.prx",
    "kd/pspnet_adhoc_discover.prx",
    "kd/pspnet_adhoc_download.prx",
    "kd/pspnet_adhoc_matching.prx",
    "kd/pspnet_adhoc_transfer_int.prx",
    "kd/pspnet_adhocctl.prx",
    "kd/pspnet_apctl.prx",
    "kd/pspnet_inet.prx",
    "kd/pspnet_resolver.prx",
    "kd/pspnet_upnp.prx",
    "kd/pspnet_wispr.prx",
    "kd/registry.prx",
    "kd/rtc.prx",
    "kd/sc_sascore.prx",
    "kd/semawm.prx",
    "kd/sircs.prx",
    "kd/syscon.prx",
    "kd/sysmem.prx",
    "kd/systimer.prx",
    "kd/threadman.prx",
    "kd/usb.prx",
    "kd/usbacc.prx",
    "kd/usbcam.prx",
    "kd/usbgps.prx",
    "kd/usbmic.prx",
    "kd/usbpspcm.prx",
    "kd/usbstor.prx",
    "kd/usbstorboot.prx",
    "kd/usbstormgr.prx",
    "kd/usbstorms.prx",
    "kd/usersystemlib.prx",
    "kd/utility.prx",
    "kd/vaudio.prx",
    "kd/videocodec_260.prx",
    "kd/vshbridge.prx",
    "kd/vshbridge_msapp.prx",
    "kd/wlan.prx",
    "vsh/etc/version.txt",
    "vsh/module/adhoc_transfer.prx",
    "vsh/module/auth_plugin.prx",
    "vsh/module/auto_connect.prx",
    "vsh/module/camera_plugin.prx",
    "vsh/module/common_gui.prx",
    "vsh/module/common_util.prx",
    "vsh/module/content_browser.prx",
    "vsh/module/dd_helper.prx",
    "vsh/module/dd_helper_utility.prx",
    "vsh/module/dialogmain.prx",
    "vsh/module/dnas_plugin.prx",
    "vsh/module/file_parser_base.prx",
    "vsh/module/game_install_plugin.prx",
    "vsh/module/game_plugin.prx",
    "vsh/module/htmlviewer_plugin.prx",
    "vsh/module/htmlviewer_ui.prx",
    "vsh/module/htmlviewer_utility.prx",
    "vsh/module/hvauth_r.prx",
    "vsh/module/impose_plugin.prx",
    "vsh/module/launcher_plugin.prx",
    "vsh/module/lftv_main_plugin.prx",
    "vsh/module/lftv_middleware.prx",
    "vsh/module/lftv_plugin.prx",
    "vsh/module/libfont_arib.prx",
    "vsh/module/libfont_hv.prx",
    "vsh/module/libpspvmc.prx",
    "vsh/module/libslim.prx",
    "vsh/module/libwww.prx",
    "vsh/module/marlindownloader.prx",
    "vsh/module/mcore.prx",
    "vsh/module/mlnapp_proxy.prx",
    "vsh/module/mlnbb.prx",
    "vsh/module/mlncmn.prx",
    "vsh/module/mlnusb.prx",
    "vsh/module/mm_flash.prx",
    "vsh/module/msgdialog_plugin.prx",
    "vsh/module/msvideo_main_plugin.prx",
    "vsh/module/msvideo_plugin.prx",
    "vsh/module/music_browser.prx",
    "vsh/module/music_main_plugin.prx",
    "vsh/module/music_parser.prx",
    "vsh/module/music_player.prx",
    "vsh/module/netconf_plugin_auto_bfl.prx",
    "vsh/module/netconf_plugin_auto_nec.prx",
    "vsh/module/netfront.prx",
    "vsh/module/netplay_client_plugin.prx",
    "vsh/module/netplay_server_plus_utility.prx",
    "vsh/module/netplay_server_utility.prx",
    "vsh/module/netplay_server2_utility.prx",
    "vsh/module/npadmin_plugin.prx",
    "vsh/module/npinstaller_plugin.prx",
    "vsh/module/npsignin_plugin.prx",
    "vsh/module/npsignup_plugin.prx",
    "vsh/module/opening_plugin.prx",
    "vsh/module/osk_plugin.prx",
    "vsh/module/paf.prx",
    "vsh/module/pafmini.prx",
    "vsh/module/photo_browser.prx",
    "vsh/module/photo_main_plugin.prx",
    "vsh/module/photo_player.prx",
    "vsh/module/premo_plugin.prx",
    "vsh/module/ps3scan_plugin.prx",
    "vsh/module/psn_plugin.prx",
    "vsh/module/psn_utility.prx",
    "vsh/module/radioshack_plugin.prx",
    "vsh/module/recommend_browser.prx",
    "vsh/module/recommend_launcher_plugin.prx",
    "vsh/module/recommend_main.prx",
    "vsh/module/rss_browser.prx",
    "vsh/module/rss_common.prx",
    "vsh/module/rss_downloader.prx",
    "vsh/module/rss_main_plugin.prx",
    "vsh/module/rss_reader.prx",
    "vsh/module/rss_subscriber.prx",
    "vsh/module/savedata_auto_dialog.prx",
    "vsh/module/savedata_plugin.prx",
    "vsh/module/savedata_utility.prx",
    "vsh/module/screenshot_plugin.prx",
    "vsh/module/store_browser_plugin.prx",
    "vsh/module/store_checkout_plugin.prx",
    "vsh/module/store_checkout_utility.prx",
    "vsh/module/subs_plugin.prx",
    "vsh/module/sysconf_plugin.prx",
    "vsh/module/update_plugin.prx",
    "vsh/module/video_main_plugin.prx",
    "vsh/module/video_plugin.prx",
    "vsh/module/visualizer_plugin.prx",
    "vsh/module/vshmain.prx",
    "vsh/resource/adhoc_transfer.rco",
    "vsh/resource/auth_plugin.rco",
    "vsh/resource/camera_plugin.rco",
    "vsh/resource/common_page.rco",
    "vsh/resource/content_browser_plugin.rco",
    "vsh/resource/dd_helper.rco",
    "vsh/resource/dnas_plugin.rco",
    "vsh/resource/game_install_plugin.rco",
    "vsh/resource/game_plugin.rco",
    "vsh/resource/gameboot.pmf",
    "vsh/resource/htmlviewer.res",
    "vsh/resource/htmlviewer_plugin.rco",
    "vsh/resource/impose_plugin.rco",
    "vsh/resource/lftv_main_plugin.rco",
    "vsh/resource/lftv_rmc_univer3in1.rco",
    "vsh/resource/lftv_rmc_univer3in1_jp.rco",
    "vsh/resource/lftv_rmc_univerpanel.rco",
    "vsh/resource/lftv_rmc_univerpanel_jp.rco",
    "vsh/resource/lftv_rmc_univertuner.rco",
    "vsh/resource/lftv_rmc_univertuner_jp.rco",
    "vsh/resource/lftv_tuner_jp_jp.rco",
    "vsh/resource/lftv_tuner_us_en.rco",
    "vsh/resource/msgdialog_plugin.rco",
    "vsh/resource/msvideo_main_plugin.rco",
    "vsh/resource/music_browser_plugin.rco",
    "vsh/resource/music_player_plugin.rco",
    "vsh/resource/netconf_dialog.rco",
    "vsh/resource/netplay_plugin.rco",
    "vsh/resource/npadmin_plugin.rco",
    "vsh/resource/npinstaller_plugin.rco",
    "vsh/resource/npsignin_plugin.rco",
    "vsh/resource/npsignup_plugin.rco",
    "vsh/resource/opening_plugin.rco",
    "vsh/resource/osk_plugin.rco",
    "vsh/resource/photo_browser_plugin.rco",
    "vsh/resource/photo_player_plugin.rco",
    "vsh/resource/premo_plugin.rco",
    "vsh/resource/ps3scan_plugin.rco",
    "vsh/resource/psn_plugin.rco",
    "vsh/resource/radioshack_plugin.rco",
    "vsh/resource/recommend_browser.rco",
    "vsh/resource/rss_browser_plugin.rco",
    "vsh/resource/rss_downloader_plugin.rco",
    "vsh/resource/rss_subscriber.rco",
    "vsh/resource/savedata_plugin.rco",
    "vsh/resource/savedata_utility.rco",
    "vsh/resource/screenshot_plugin.rco",
    "vsh/resource/store_browser_plugin.rco",
    "vsh/resource/store_checkout_plugin.rco",
    "vsh/resource/subs_plugin.rco",
    "vsh/resource/sysconf_plugin.rco",
    "vsh/resource/sysconf_plugin_about.rco",
    "vsh/resource/system_plugin.rco",
    "vsh/resource/system_plugin_bg.rco",
    "vsh/resource/system_plugin_fg.rco",
    "vsh/resource/topmenu_icon.rco",
    "vsh/resource/topmenu_plugin.rco",
    "vsh/resource/update_plugin.rco",
    "vsh/resource/video_main_plugin.rco",
    "vsh/resource/video_plugin_videotoolbar.rco",
    "vsh/resource/visualizer_plugin.rco"
};

const char *f0_01g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "kd/codec_01g.prx",
    "kd/display_01g.prx",
    "kd/hpremote_01g.prx",
    "kd/impose_01g.prx",
    "kd/loadexec_01g.prx",
    "kd/memlmd_01g.prx",
    "kd/mesg_led_01g.prx",
    "kd/pops_01g.prx",
    "kd/power_01g.prx",
    "kd/pspbtcnf.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_blimg.img",
    "kd/resource/me_sdimg.img",
    "kd/resource/meimg.img",
    "kd/wlanfirm_01g.prx",
    "vsh/etc/index_01g.dat",
    "vsh/module/netconf_plugin.prx",
    "vsh/resource/01-12.bmp",
    "vsh/resource/13-27.bmp",
};

const char *f0_02g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "font/arib.pgf",
    "kd/codec_02g.prx",
    "kd/display_02g.prx",
    "kd/hpremote_02g.prx",
    "kd/impose_02g.prx",
    "kd/loadexec_02g.prx",
    "kd/memlmd_02g.prx",
    "kd/mesg_led_02g.prx",
    "kd/pops_02g.prx",
    "kd/power_02g.prx",
    "kd/pspbtcnf_02g.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_t2img.img",
    "kd/umdcache.prx",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "kd/wlanfirm_02g.prx",
    "vsh/etc/index_02g.dat",
    "vsh/module/netconf_plugin.prx",
    "vsh/module/oneseg_core.prx",
    "vsh/module/oneseg_hal_toolbox.prx",
    "vsh/module/oneseg_launcher_plugin.prx",
    "vsh/module/oneseg_plugin.prx",
    "vsh/module/oneseg_sal.prx",
    "vsh/module/oneseg_sdk.prx",
    "vsh/module/oneseg_sdkcore.prx",
    "vsh/module/skype_main_plugin.prx",
    "vsh/module/skype_plugin.prx",
    "vsh/module/skype_skyhost.prx",
    "vsh/module/skype_ve.prx",
    "vsh/resource/01-12.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/custom_theme.dat",
    "vsh/resource/oneseg_plugin.rco",
    "vsh/resource/skype_main_plugin.rco"
};

const char *f0_03g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "font/arib.pgf",
    "kd/codec_03g.prx",
    "kd/display_03g.prx",
    "kd/hpremote_03g.prx",
    "kd/impose_03g.prx",
    "kd/loadexec_03g.prx",
    "kd/memlmd_03g.prx",
    "kd/mesg_led_03g.prx",
    "kd/pops_03g.prx",
    "kd/power_03g.prx",
    "kd/pspbtcnf_03g.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_t2img.img",
    "kd/umdcache.prx",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "kd/wlanfirm_03g.prx",
    "vsh/etc/index_03g.dat",
    "vsh/module/netconf_plugin.prx",
    "vsh/module/oneseg_core.prx",
    "vsh/module/oneseg_hal_toolbox.prx",
    "vsh/module/oneseg_launcher_plugin.prx",
    "vsh/module/oneseg_plugin.prx",
    "vsh/module/oneseg_sal.prx",
    "vsh/module/oneseg_sdk.prx",
    "vsh/module/oneseg_sdkcore.prx",
    "vsh/module/skype_main_plugin.prx",
    "vsh/module/skype_plugin.prx",
    "vsh/module/skype_skyhost.prx",
    "vsh/module/skype_ve.prx",
    "vsh/resource/01-12_03g.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/custom_theme.dat",
    "vsh/resource/oneseg_plugin.rco",
    "vsh/resource/skype_main_plugin.rco"
};

const char *f0_04g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "font/arib.pgf",
    "kd/codec_04g.prx",
    "kd/display_04g.prx",
    "kd/hpremote_04g.prx",
    "kd/impose_04g.prx",
    "kd/loadexec_04g.prx",
    "kd/memlmd_04g.prx",
    "kd/mesg_led_04g.prx",
    "kd/pops_04g.prx",
    "kd/power_04g.prx",
    "kd/pspbtcnf_04g.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_t2img.img",
    "kd/umdcache.prx",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "kd/wlanfirm_04g.prx",
    "vsh/etc/index_04g.dat",
    "vsh/module/netconf_plugin.prx",
    "vsh/module/oneseg_core.prx",
    "vsh/module/oneseg_hal_toolbox.prx",
    "vsh/module/oneseg_launcher_plugin.prx",
    "vsh/module/oneseg_plugin.prx",
    "vsh/module/oneseg_sal.prx",
    "vsh/module/oneseg_sdk.prx",
    "vsh/module/oneseg_sdkcore.prx",
    "vsh/module/skype_main_plugin.prx",
    "vsh/module/skype_plugin.prx",
    "vsh/module/skype_skyhost.prx",
    "vsh/module/skype_ve.prx",
    "vsh/resource/01-12_03g.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/custom_theme.dat",
    "vsh/resource/oneseg_plugin.rco",
    "vsh/resource/skype_main_plugin.rco"
};

const char *f0_05g[] =
{
    "kd/bsman.prx",
    "kd/btdun.prx",
    "kd/eflash_05g.prx",
    "kd/fatmsef.prx",
    "kd/msemu.prx",
    "kd/hidsvc.prx",
    "kd/input_05g.prx",
    "kd/padsvc.prx",
    "kd/usbbsmcdc.prx",
    "font/arib.pgf",
    "kd/codec_05g.prx",
    "kd/display_05g.prx",
    "kd/hpremote_05g.prx",
    "kd/impose_05g.prx",
    "kd/loadexec_05g.prx",
    "kd/memlmd_05g.prx",
    "kd/mesg_led_05g.prx",
    "kd/pops_05g.prx",
    "kd/power_05g.prx",
    "kd/pspbtcnf_05g.bin",
    "kd/resource/impose_05g.rsc",
    "kd/resource/me_t2img.img",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "kd/usb_host.prx",
    "kd/usbps3controller.prx",
    "kd/usbstoreflash.prx",
    "kd/wlanfirm_05g.prx",
    "vsh/etc/index_05g.dat",
    "vsh/module/bluetooth_plugin.prx",
    "vsh/module/netconf_bt_plugin.prx",
    "vsh/module/oneseg_core.prx",
    "vsh/module/oneseg_hal_toolbox.prx",
    "vsh/module/oneseg_launcher_plugin.prx",
    "vsh/module/oneseg_plugin.prx",
    "vsh/module/oneseg_sal.prx",
    "vsh/module/oneseg_sdk.prx",
    "vsh/module/oneseg_sdkcore.prx",
    "vsh/module/skype_main_plugin.prx",
    "vsh/module/skype_plugin.prx",
    "vsh/module/skype_skyhost.prx",
    "vsh/module/skype_ve_05g.prx",
    "vsh/module/slide_plugin.prx",
    "vsh/resource/01-12_03g.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/bluetooth_plugin.rco",
    "vsh/resource/custom_theme.dat",
    "vsh/resource/oneseg_plugin.rco",
    "vsh/resource/skype_main_plugin.rco",
    "vsh/resource/slide_plugin.rco"
};

const char *f0_07g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "font/arib.pgf",
    "kd/codec_07g.prx",
    "kd/display_07g.prx",
    "kd/hpremote_07g.prx",
    "kd/impose_07g.prx",
    "kd/loadexec_07g.prx",
    "kd/memlmd_07g.prx",
    "kd/mesg_led_07g.prx",
    "kd/pops_07g.prx",
    "kd/power_07g.prx",
    "kd/pspbtcnf_07g.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_t2img.img",
    "kd/umdcache.prx",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "kd/wlanfirm_07g.prx",
    "vsh/etc/index_07g.dat",
    "vsh/module/oneseg_core.prx",
    "vsh/module/oneseg_hal_toolbox.prx",
    "vsh/module/oneseg_launcher_plugin.prx",
    "vsh/module/oneseg_plugin.prx",
    "vsh/module/oneseg_sal.prx",
    "vsh/module/oneseg_sdk.prx",
    "vsh/module/oneseg_sdkcore.prx",
    "vsh/module/skype_main_plugin.prx",
    "vsh/module/skype_plugin.prx",
    "vsh/module/skype_skyhost.prx",
    "vsh/module/skype_ve.prx",
    "vsh/resource/01-12_03g.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/custom_theme.dat",
    "vsh/resource/oneseg_plugin.rco",
    "vsh/resource/skype_main_plugin.rco"
};

const char *f0_09g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "font/arib.pgf",
    "kd/codec_09g.prx",
    "kd/display_09g.prx",
    "kd/hpremote_09g.prx",
    "kd/impose_09g.prx",
    "kd/loadexec_09g.prx",
    "kd/memlmd_09g.prx",
    "kd/mesg_led_09g.prx",
    "kd/pops_09g.prx",
    "kd/power_09g.prx",
    "kd/pspbtcnf_09g.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_t2img.img",
    "kd/umdcache.prx",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "kd/wlanfirm_09g.prx",
    "vsh/etc/index_09g.dat",
    "vsh/module/oneseg_core.prx",
    "vsh/module/oneseg_hal_toolbox.prx",
    "vsh/module/oneseg_launcher_plugin.prx",
    "vsh/module/oneseg_plugin.prx",
    "vsh/module/oneseg_sal.prx",
    "vsh/module/oneseg_sdk.prx",
    "vsh/module/oneseg_sdkcore.prx",
    "vsh/module/skype_main_plugin.prx",
    "vsh/module/skype_plugin.prx",
    "vsh/module/skype_skyhost.prx",
    "vsh/module/skype_ve.prx",
    "vsh/resource/01-12_03g.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/custom_theme.dat",
    "vsh/resource/oneseg_plugin.rco",
    "vsh/resource/skype_main_plugin.rco"
};

const char *f0_11g[] =
{
    "kd/ata.prx",
    "kd/fatms.prx",
    "font/arib.pgf",
    "kd/codec_11g.prx",
    "kd/display_11g.prx",
    "kd/hpremote_11g.prx",
    "kd/impose_11g.prx",
    "kd/loadexec_11g.prx",
    "kd/memlmd_11g.prx",
    "kd/mesg_led_11g.prx",
    "kd/pops_11g.prx",
    "kd/power_11g.prx",
    "kd/pspbtcnf_11g.bin",
    "kd/umd9660.prx",
    "kd/umdman.prx",
    "kd/resource/impose.rsc",
    "kd/resource/me_t2img.img",
    "kd/umdcache.prx",
    "kd/usb1seg.prx",
    "kd/usbdmb.prx",
    "vsh/etc/index_11g.dat",
    "vsh/resource/01-12_03g.bmp",
    "vsh/resource/13-27.bmp",
    "vsh/resource/custom_theme.dat",
};

const char *f0_ark[] =
{
    "kd/ark_inferno.prx",
    "kd/ark_popcorn.prx",
    "kd/ark_pspcompat.prx",
    "kd/ark_stargate.prx",
    "kd/ark_systemctrl.prx",
    "kd/ark_vshctrl.prx",
};

struct {
    char* orig;
    char* dest;
} f0_ark_extras[] = {
    {IDSREG_PRX, IDSREG_PRX_FLASH},
    {XMBCTRL_PRX, XMBCTRL_PRX_FLASH},
    {USBDEV_PRX, USBDEV_PRX_FLASH},
    {VSH_MENU, VSH_MENU_FLASH},
    {RECOVERY_PRX, RECOVERY_PRX_FLASH},
    {UPDATER_FILE, UPDATER_FILE_FLASH},
};

int LoadUpdaterModules()
{
    SceUID mod = sceKernelLoadModule("flash0:/kd/emc_sm_updater.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return 1;

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return 2;
    }

    mod = sceKernelLoadModule("flash0:/kd/lfatfs_updater.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return 3;

    dcPatchModuleString("sceLFatFs_Updater_Driver", "flash", "flach");

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return 4;
    }

    mod = sceKernelLoadModule("flash0:/kd/lflash_fatfmt_updater.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return 5;

    sceKernelDelayThread(10000);

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return 6;
    }

    mod = sceKernelLoadModule("flash0:/kd/ipl_update.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return 7;

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return 8;
    }

    mod = sceKernelLoadModule("flash0:/kd/pspdecrypt.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return 9;

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return 10;
    }

    return 0;
}

void SetInstallProgress(int value, int max, int force, int fw)
{
    u32 prog;
    
    prog = ((((fw == FW_OFW) ? 95 : 93) * value) / max) + 4;

    SetProgress(prog, force);
}

char upd_error_msg[256];
char upd_error_msg2[256];

void logtext(char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(upd_error_msg2, fmt, list);
    va_end(list);

    int fd = sceIoOpen("ms0:/dcark.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    sceIoWrite(fd, upd_error_msg2, strlen(upd_error_msg2));
    sceIoClose(fd);
}

int OnInstallError(void *param)
{
    int fw = *(int *)param;
    
    vlfGuiMessageDialog(upd_error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

    vlfGuiRemoveText(status);
    vlfGuiRemoveText(progress_text);
    vlfGuiRemoveProgressBar(progress_bar);

    progress_bar = -1;
    progress_text = -1;
    status = -1;

    int sel;
    switch(fw)
    {
        case FW_OFW: sel = 1; break;
        case FW_ARK: sel = 0; break;
    }

    dcSetCancelMode(0);
    MainMenu(sel);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

void InstallError(int fw, char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(upd_error_msg, fmt, list);
    va_end(list);

    logtext("%s\n", upd_error_msg);

    vlfGuiAddEventHandler(0, -1, OnInstallError, &fw);
    sceKernelExitDeleteThread(0);
}

int OnInstallComplete(void *param)
{
    int fw = *(int *)param;

    vlfGuiRemoveProgressBar(progress_bar);
    vlfGuiRemoveText(progress_text);

    dcSetCancelMode(0);

    if (fw == FW_OFW && kuKernelGetModel() == 1)
    {
        SetStatus("Install is complete.\n"
        	      "A shutdown is required. A normal battery is\n"
        		  "required to boot this firmware on this PSP.");
         AddShutdownRebootBD(1);
    }
    else
    {
        SetStatus("Install is complete.\nA shutdown or a reboot is required.");
        AddShutdownRebootBD(0);
    }

    progress_bar = -1;
    progress_text = -1;    

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int CreateDirs()
{
    int res = sceIoMkdir("flach0:/codepage", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/data", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/data/cert", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/dic", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/font", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/kd", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/kd/resource", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/vsh", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/vsh/etc", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/vsh/module", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach0:/vsh/resource", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    return 0;
}

int CreateFlash1Dirs()
{
    int res = sceIoMkdir("flach1:/dic", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach1:/gps", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach1:/net", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach1:/net/http", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach1:/registry", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach1:/vsh", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    res = sceIoMkdir("flach1:/vsh/theme", 0777);
    if (res < 0 && res != 0x80010011)
        return res;

    return 0;
}

void CopyFileList(int fw, const char **list, int file_count, int start_file_count, int max_file_count)
{
    for (int i = 0; i < file_count; i++)
    {
        char src[256];
        char dest[256];
        strcpy(src,  "flash0:/");
        strcpy(dest, "flach0:/");
        strcat(src, list[i]);
        strcat(dest, list[i]);
        
        SceUID fdi = sceIoOpen(src, PSP_O_RDONLY, 0);
        if (fdi < 0)
        {
        	InstallError(fw, "Error opening %s: 0x%08X", src, fdi);
        }
        
        SceUID fdo = sceIoOpen(dest, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (fdo < 0)
        {
        	InstallError(fw, "Error opening %s: 0x%08X", dest, fdo);
        }
        
        char signechecked = 0;
        while (1)
        {
        	int read = sceIoRead(fdi, big_buffer, BIG_BUFFER_SIZE);
        	if (read < 0)
        	{
        		InstallError(fw, "Error reading %s", src);
        	}
        	if (!read)
        		break;
        
        	if (!signechecked &&
        		(memcmp(&list[i][strlen(list[i]) - 4], ".prx", 4) == 0 ||
        		memcmp(list[i], "kd/pspbtcnf", 11) == 0))
        	{
        		pspSignCheck(big_buffer);
        		signechecked = 1;
        	}

        	if (sceIoWrite(fdo, big_buffer, read) < 0)
        	{
        		InstallError(fw, "Error writing %s", dest);
        	}
        }
        
        sceIoClose(fdi);
        sceIoClose(fdo);

        SetInstallProgress(start_file_count + i, max_file_count, 0, fw);
        scePowerTick(0);
    }
}

void copy_file(char* orig, char* dest){
    #define BUF_SIZE 16*1024
    static u8 buf[BUF_SIZE];
    int fdr = sceIoOpen(orig, PSP_O_RDONLY, 0777);
    int fdw = sceIoOpen(dest, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    while (1){
        int read = sceIoRead(fdr, buf, BUF_SIZE);
        if (read <= 0) break;
        sceIoWrite(fdw, buf, read);
    }
    sceIoClose(fdr);
    sceIoClose(fdw);
}

int install_thread(SceSize args, void *argp)
{
    int fw = *(int *)argp;    
    char *argv[2];
    int res;
    int size;
    int model = kuKernelGetModel();
    int mb;

    dcGetHardwareInfo(NULL, NULL, NULL, &mb, NULL, NULL, NULL);

    dcSetCancelMode(1);

    int file_count = sizeof(f0_common) / sizeof(f0_common[0]);

    switch (model)
    {
        case 0:
        	file_count += sizeof(f0_01g) / sizeof(f0_01g[0]);
        	break;
        case 1:
        	file_count += sizeof(f0_02g) / sizeof(f0_02g[0]);
        	break;
        case 2:
        	file_count += sizeof(f0_03g) / sizeof(f0_03g[0]);
        	break;
        case 3:
        	file_count += sizeof(f0_04g) / sizeof(f0_04g[0]);
        	break;
        case 4:
        	file_count += sizeof(f0_05g) / sizeof(f0_05g[0]);
        	break;
        case 6:
        	file_count += sizeof(f0_07g) / sizeof(f0_07g[0]);
        	break;
        case 8:
        	file_count += sizeof(f0_09g) / sizeof(f0_09g[0]);
        	break;
        case 10:
        	file_count += sizeof(f0_11g) / sizeof(f0_11g[0]);
        	break;
        default:
        	InstallError(fw, "Unsupported model.");
    }

    switch(LoadUpdaterModules(fw))
    {
        case 0: break;
        case 1: InstallError(fw, "Failed to load emc_sm_updater.prx"); break;
        case 2: InstallError(fw, "Failed to start emc_sm_updater.prx"); break;
        case 3: InstallError(fw, "Failed to load lfatfs_updater.prx"); break;
        case 4: InstallError(fw, "Failed to start lfatfs_updater.prx"); break;
        case 5: InstallError(fw, "Failed to load lflash_fatfmt_updater.prx"); break;
        case 6: InstallError(fw, "Failed to start lflash_fatfmt_updater.prx"); break;
        case 7: InstallError(fw, "Failed to load ipl_update.prx"); break;
        case 8: InstallError(fw, "Failed to start ipl_update.prx"); break;
        case 9: InstallError(fw, "Failed to load pspdecrypt.prx"); break;
        case 10: InstallError(fw, "Failed to start pspdecrypt.prx"); break;
        default: InstallError(fw, "Error loading updater modules.");
    }

    // Unassign with error ignore (they might have been assigned in a failed attempt of install ARK/OFW)
    sceIoUnassign("flach0:");
    sceIoUnassign("flach1:");
    sceIoUnassign("flach2:");
    sceIoUnassign("flach3:");

    sceKernelDelayThread(1200000);

    SetStatus("Formatting flash0... ");

    argv[0] = "fatfmt";
    argv[1] = "lflach0:0,0";

    res = dcLflashStartFatfmt(2, argv);
    if (res < 0)
    {
        InstallError(fw, "Flash0 format failed: 0x%08X", res);
    }

    sceKernelDelayThread(1200000);
    SetProgress(1, 1);

    SetStatus("Formatting flash1... ");

    argv[1] = "lflach0:0,1";

    res = dcLflashStartFatfmt(2, argv);
    if (res < 0)
    {
        InstallError(fw, "Flash1 format failed: 0x%08X", res);
    }

    sceKernelDelayThread(1200000);

    SetStatus("Formatting flash2... ");

    argv[1] = "lflach0:0,2";
    dcLflashStartFatfmt(2, argv);

    sceKernelDelayThread(1200000);
    SetProgress(2, 1);

    SetStatus("Assigning flashes...");

    res = sceIoAssign("flach0:", "lflach0:0,0", "flachfat0:", IOASSIGN_RDWR, NULL, 0);
    if (res < 0)
    {
        InstallError(fw, "Flash0 assign failed: 0x%08X", res);
    }

    res = sceIoAssign("flach1:", "lflach0:0,1", "flachfat1:", IOASSIGN_RDWR, NULL, 0);
    if (res < 0)
    {
        InstallError(fw, "Flash1 assign failed: 0x%08X", res);
    }

    sceIoAssign("flach2:", "lflach0:0,2", "flachfat2:", IOASSIGN_RDWR, NULL, 0);
    
    sceKernelDelayThread(1200000);
    SetProgress(3, 1);

    SetStatus("Creating directories...");

    if (CreateDirs() < 0)
    {
        InstallError(fw, "Directories creation failed.");
    }

    sceKernelDelayThread(1200000);
    SetProgress(4, 1);

    SetStatus("Flashing files...");
    
    int ctr = 0;
    CopyFileList(fw, f0_common, sizeof(f0_common) / sizeof(f0_common[0]), ctr, file_count);
    ctr += sizeof(f0_common) / sizeof(f0_common[0]);
    
    switch (model)
    {
        case 0:
        	CopyFileList(fw, f0_01g, sizeof(f0_01g) / sizeof(f0_01g[0]), ctr, file_count);
        	ctr += sizeof(f0_01g) / sizeof(f0_01g[0]);
        	break;
        case 1:
        	CopyFileList(fw, f0_02g, sizeof(f0_02g) / sizeof(f0_02g[0]), ctr, file_count);
        	ctr += sizeof(f0_02g) / sizeof(f0_02g[0]);
        	break;
        case 2:
        	CopyFileList(fw, f0_03g, sizeof(f0_03g) / sizeof(f0_03g[0]), ctr, file_count);
        	ctr += sizeof(f0_03g) / sizeof(f0_03g[0]);
        	break;
        case 3:
        	CopyFileList(fw, f0_04g, sizeof(f0_04g) / sizeof(f0_04g[0]), ctr, file_count);
        	ctr += sizeof(f0_04g) / sizeof(f0_04g[0]);
        	break;
        case 4:
        	CopyFileList(fw, f0_05g, sizeof(f0_05g) / sizeof(f0_05g[0]), ctr, file_count);
        	ctr += sizeof(f0_05g) / sizeof(f0_05g[0]);
        	break;
        case 6:
        	CopyFileList(fw, f0_07g, sizeof(f0_07g) / sizeof(f0_07g[0]), ctr, file_count);
        	ctr += sizeof(f0_07g) / sizeof(f0_07g[0]);
        	break;
        case 8:
        	CopyFileList(fw, f0_09g, sizeof(f0_09g) / sizeof(f0_09g[0]), ctr, file_count);
        	ctr += sizeof(f0_09g) / sizeof(f0_09g[0]);
        	break;
        case 10:
        	CopyFileList(fw, f0_11g, sizeof(f0_11g) / sizeof(f0_11g[0]), ctr, file_count);
        	ctr += sizeof(f0_11g) / sizeof(f0_11g[0]);
        	break;
        default:
        	InstallError(fw, "Unsupported model.");
    }
    
    sceKernelDelayThread(200000);
    SetInstallProgress(1, 1, 1, fw);

    SetStatus("Restoring registry...");
    sceKernelDelayThread(100000);

    if (CreateFlash1Dirs() < 0)
        InstallError(fw, "Error creating flash1 directories.");

    if (WriteFile("flach1:/registry/init.dat", sm_buffer1, 0) < 0)
    {
        InstallError(fw, "Cannot write init.dat\n");
    }

    res = ReadFile("flash2:/registry/act.dat", 0, sm_buffer1, SMALL_BUFFER_SIZE);
    if (res > 0)
    {
        WriteFile("flach2:/act.dat", sm_buffer1, res);		
    }

    SetStatus("Flashing IPL...");

    const char *ipl_name = 0;
    u16 ipl_key = 0;
    int offset = 0;

    memset(big_buffer, 0, BIG_BUFFER_SIZE);
    
    if (fw == FW_OFW)
    {
        switch (model)
        {
        	case 0: ipl_name = "flash0:/ipl_01g.bin"; break;
        	case 1: ipl_name = "flash0:/ipl_02g.bin"; break;
        	case 2: ipl_name = "flash0:/ipl_03g.bin"; ipl_key = 1; break;
        	case 3: ipl_name = "flash0:/ipl_04g.bin"; ipl_key = 1; break;
        	case 4: ipl_name = "flash0:/ipl_05g.bin"; ipl_key = 2; break;
        	case 6: ipl_name = "flash0:/ipl_07g.bin"; ipl_key = 1; break;
        	case 8: ipl_name = "flash0:/ipl_09g.bin"; ipl_key = 1; break;
        	case 10: ipl_name = "flash0:/ipl_11g.bin"; ipl_key = 1; break;
        	default: InstallError(fw, "Unsupported model.");
        }
    }
    else
    {
        switch (model)
        {
        	case 0:  ipl_name = "flash0:/cipl_01g.bin"; break;
        	case 1:  ipl_name = "flash0:/cipl_02g.bin"; break;
        	case 2:  ipl_name = "flash0:/cipl_03g.bin"; break;
        	case 3:  ipl_name = "flash0:/cipl_04g.bin"; break;
        	case 4:  ipl_name = "flash0:/cipl_05g.bin"; break;
        	case 6:  ipl_name = "flash0:/cipl_07g.bin"; break;
        	case 8:  ipl_name = "flash0:/cipl_09g.bin"; break;
        	case 10: ipl_name = "flash0:/cipl_11g.bin"; break;
        	default: InstallError(fw, "Unsupported model.");
        }
    }

    size = offset+ReadFile(ipl_name, 0, big_buffer+offset, BIG_BUFFER_SIZE-offset);
    if (size-offset <= 0)
    {
        char msg[128];
        sprintf(msg, "Cannot read %s\n", ipl_name);
        InstallError(fw, msg);
    }
    
    dcPatchModuleString("IoPrivileged", "IoPrivileged", "IoPrivileged");

    if (pspIplUpdateClearIpl() < 0)
        InstallError(fw, "Error in pspIplUpdateClearIpl");
    
    if (pspIplUpdateSetIpl(big_buffer, size, ipl_key) < 0)
        InstallError(fw, "Error in pspIplUpdateSetIpl");

    sceKernelDelayThread(900000);

    if (fw != FW_OFW)
    {
        int file_count = file_count += sizeof(f0_ark) / sizeof(f0_ark[0]);
        CopyFileList(fw, f0_ark, sizeof(f0_ark) / sizeof(f0_ark[0]), 0, file_count);
        for (int i=0; i<(sizeof(f0_ark_extras)/sizeof(f0_ark_extras[0])); i++)
        {
        	char path[ARK_PATH_SIZE];
        	strcpy(path, "flash0:/ARK_01234/");
        	strcat(path, f0_ark_extras[i].orig);
        	f0_ark_extras[i].dest[3] = 'c';
        	copy_file(path, f0_ark_extras[i].dest);
        }
    }

    SetProgress(100, 1);
    vlfGuiAddEventHandler(0, 600000, OnInstallComplete, &fw);

    return sceKernelExitDeleteThread(0);
}

int Install(int fw)
{
    ClearProgress();
    status = vlfGuiAddText(80, 100, "Loading updater modules...");

    progress_bar = vlfGuiAddProgressBar(136);    
    progress_text = vlfGuiAddText(240, 148, "0%");
    vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

    SceUID install_thid = sceKernelCreateThread("install_thread", install_thread, 0x18, 0x10000, 0, NULL);
    if (install_thid >= 0)
    {
        sceKernelStartThread(install_thid, 4, &fw);
    }

    return VLF_EV_RET_REMOVE_OBJECTS | VLF_EV_RET_REMOVE_HANDLERS;
}

