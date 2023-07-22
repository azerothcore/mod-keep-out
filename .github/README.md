# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

## mod-keepout

Module for azerothcore to keep players who are non gm entering a zone/map

- Latest build status with azerothcore:

[![Build Status](https://github.com/azerothcore/mod-keep-out/workflows/core-build/badge.svg?branch=master&event=push)](https://github.com/azerothcore/mod-keep-out)

## This module currently requires:
- AzerothCore v1.0.1+

## How to install

1. Simply place the module under the `modules` folder of your AzerothCore source folder.
2. Re-run cmake and launch a clean build of AzerothCore
3. Run the `worldserver` and `authserver`, so that the changes are applied to the database.

## Usage

- Enable this module in conf
- Get the map ID and zone ID with ingame command `.gps`, then edit the table `map_lock` table.

## Credits

- Talamortis (original author) - Show your appreciation:

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SBJFTAJKUNEXC)
