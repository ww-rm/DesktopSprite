# DesktopSprite

## What's it ?

- A small desktop tool used to show your real-time performance data of your PC.

![Interface1.png](https://github.com/ww-rm/DesktopSprite/blob/main/img/interface1.png)

![Interface2.png](https://github.com/ww-rm/DesktopSprite/blob/main/img/interface2.png)

## How to use it ?

- Just go to [release page](https://github.com/ww-rm/DesktopSprite/releases) and download the lastest version of it.
- Choose a appropriate location to store the executable file of this tool.
- Then double click it, a small icon will be added to your notification area and you can see a small floating window appearing on you desktop.

## Config File

The program uses a `config.json` as its config file, the default config contents are shown as below:

```json
{
  "BalloonIconPath" : "res\\image\\timealarm.ico",
  "IsAutoRun" : false,
  "IsDarkTheme" : true,
  "IsFloatWnd" : true,
  "IsInfoSound" : true,
  "IsTimeAlarm" : true,
  "ShowContent" : 3,
  "Transparency" : 75.0
}
```

Most config options can be set within program, some need to edit this config file.

To make `config.json` take effect, you need to put it under the **program install directory**, and all paths in this config file are relative to the program install directory.

## FAQ

- Why sometimes the performance data is different than in the task manager?
  - In this small tool, the performance data is collected by [PDH](https://docs.microsoft.com/en-us/windows/win32/perfctrs/performance-counters-portal), there is some difference between their collecting mode. However, the data should be as the same as the data in your [perfmon.msc](https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/perfmon) program.

- Why the memory usage is a little higher than the data in task manager?
  - In this small tool, the memory usage is refer to committed size not the physical size.

## About

- Source of material
  - [Icon File](https://github.com/ww-rm/DesktopSprite/blob/main/DesktopSprite/res/appicon_dark.png): From: [凪白みと](https://www.pixiv.net/artworks/56065927)
  - [Balloontip Icon File](https://github.com/ww-rm/DesktopSprite/blob/main/DesktopSprite/res/timealarm.png) From: [むーらん@お仕事募集中](https://www.pixiv.net/artworks/67763478)
  - [Font File](https://github.com/ww-rm/DesktopSprite/blob/main/DesktopSprite/res/AGENCYR.TTF): From Internet.

---

*If you think this project is helpful to you, plz star it and let more people see it. :)*
