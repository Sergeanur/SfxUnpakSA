# SfxUnpackSA

[![Join Discord](https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat)](https://discord.gg/WgAg9ymHbj)

[![Support Author](https://img.shields.io/badge/support-author-blue)](https://bit.ly/3sX2oMk) [![Help Ukraine](https://img.shields.io/badge/help-ukraine-yellow)](https://bit.ly/3afhuGm)

Two tools to unpack SA audio data.

## SfxUnpack

A small utility to unpack audio data from SA PC version (possibly Xbox version too) with an ability to name banks and some files from given list.

### Usage

Put into 'audio' directory. Launch 'SfxUnpak.exe'. Names are read from 'banklist.txt', 'tracklist.txt' and '../data/AudioEvents.txt'. Output will be in 'unpacked' directory.

### Changelog

v0.2.1
* small fixes

v0.2
* fixed size in wav header
* added streams support

v0.1
* initial release

## SfxUnpack_PS2

Same as above but for PS2 version. Can extract original vag format and perform conversion into wav.

### Usage

Put into 'audio' directory. Launch 'SfxUnpak_PS2.exe'. Names are read from 'banklist_ps2.txt', 'tracklist_ps2.txt' and '../data/AudioEvents.txt'. Output will be in 'unpacked' directory.
DO NOT USE AudioEvents.txt FROM PC!

### Changelog

v0.2.1
* small fixes

v0.2
* added modes to dump only VAG files or both VAG and WAV
* fixed excessive samples written to SFX wavs
* Changed reading from 'tracklist.txt' to 'tracklist_ps2.txt' with some fixes to tracks order

v0.1
* initial release