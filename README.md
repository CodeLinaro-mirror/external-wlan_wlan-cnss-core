# napier_x86
Porting qualcomm related components(mhi, ipc_router, qmi, ...) to support napier on Ubuntu

Firmware & config files on Ubuntu:
1. put firmware files to /lib/firmware/ directory;
2. mkdir wlan in /lib/firmware, and put qcom_cfg.ini in it.

Build msm driver:
$ cd plat_drv
$ make

Build cld driver:
$ cd qcacld-3.0
$ make CONFIG_CNSS_QCA6290=y CONFIG_CNSS2=y CONFIG_INET_LRO=n

Ubuntu crash dump:
https://projects.qualcomm.com/sites/WLAN-SW/_layouts/OneNote.aspx?id=%2Fsites%2FWLAN-SW%2FOneNote%2FSDC_LA%2FSDC%20LA%20CNSS&wd=target%28Proejct%2FNapier_Ubuntu.one%7CA137E73A-9962-46BD-A57B-8B37149BEA5B%2FUbuntu%20crash%20dump%7C455BBF4F-1A88-44A3-A058-D519E3A87958%2F%29
onenote:https://projects.qualcomm.com/sites/WLAN-SW/OneNote/SDC_LA/SDC%20LA%20CNSS/Proejct/Napier_Ubuntu.one#Ubuntu%20crash%20dump%20&section-id={A137E73A-9962-46BD-A57B-8B37149BEA5B}&page-id={455BBF4F-1A88-44A3-A058-D519E3A87958}&end
