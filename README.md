# Localized EmulationStation

This is a fork of [Localized Retropie's EmulationStation fork](https://github.com/flyinghead/EmulationStation.git).

This project aims to keep a localized EmulationStation up-to-date with [Retropie's EmulationStation fork](https://github.com/RetroPie/EmulationStation.git)

An update of the localized EmulationStation will be done only on a tagged release from [Retropie's EmulationStation fork](https://github.com/RetroPie/EmulationStation.git).
There is no need to ask for an update on an untagged master branch.

## Languages currently supported

Initial localized fork supports multiple languages but for now, only French is fully supported.

A partial translation is available for the following languages (based on v2.8.0):
* Catalan
* German
* Italian
* Japanese
* Korean
* Portuguese (Brazilian)
* Spanish (Spain)
* Traditional Chinese

## How to use the localized EmulationStation

### Resolve dependencies of libraries
To build localized EmulationStation, you need to install all dependencies as wanted by the [Retropie's EmulationStation fork](https://github.com/RetroPie/EmulationStation#building)

In addition to these dependencies, you need to install following ones for the localized version using `apt-get`:
```bash
sudo apt-get install libboost-system-dev libboost-filesystem-dev libboost-date-time-dev libboost-locale-dev
```

### Create a clone from GitHub

```
cd /home/pi/
git clone --recursive https://github.com/Yaoh/EmulationStation.git
```

### Compile EmulationStation

```
cd /home/pi/EmulationStation
mkdir build
cd build
cmake ..
make
```

### Run EmulationStation

> Be aware that your system must be set to the same locale that you want EmulationStation to run on.

```
cd /home/pi/EmulationStation
LANG=[your_locale].UTF8 ./emulationstation
```

### Install EmulationStation

```
cd /home/pi/EmulationStation
sudo cp ./emulationstation /opt/retropie/supplementary/emulationstation/
sudo cp -r locale /opt/retropie/supplementary/emulationstation/
```

## How to add a translation

> You should have, at least, [created a clone from GitHub](#create-a-clone-from-github) before continuing with the following steps.

### Create files for the new locale

```
cd /home/pi/EmulationStation/locale
mkdir lang/[your_locale]
cp emulationstation2.pot lang/[your_locale]/emulationstation2.po
```

### Translate the strings

Open the newly created `emulationstation2.po` inside `[your_locale]` folder and start translating all the `msgstr` strings.

Once you are done, you can [compile EmulationStation](#compile-emulationstation) and then [run](#run-emulationstation) it to test your translations.

## Author

[Yaoh](https://github.com/Yaoh)

## Credits

Thanks to:

* [flyinghead and all the contributors](https://github.com/flyinghead/EmulationStation#credits) - For the initial work.
