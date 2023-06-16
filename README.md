# OBS Scale To Sound
A plugin for OBS Studio that adds a filter which makes a source scale based on the audio levels of any audio source you choose 

There are a few properties so you can fine tune how it behaves  
![Properties](https://dimtpap.xyz/obs-scale-to-sound/plugin-properties.png)  
[OBS forums post](https://obsproject.com/forum/resources/scale-to-sound.1336/)  
This plugin is not extensively tested, you may come across bugs

## Installation
Get the archive/installer for your OS from the [latest release](https://github.com/dimtpap/obs-scale-to-sound/releases/latest).  
### Linux
The archive is for installing in the home directory  
Extract it in `~/.config/obs-studio/plugins`  
#### Flatpak
Available in Flathub. You can find it on OBS Studio's page under Add-ons.  
#### AUR
If you're on Arch Linux or an Arch-based distro, you can get the [AUR package](https://aur.archlinux.org/packages/?O=0&K=obs-scale-to-sound).  
Use the [AUR helper](https://wiki.archlinux.org/title/AUR_helpers) of your choice, or install it manually like this
```bash
# Pre-built binaries can be installed using the obs-scale-to-sound-bin package
git clone https://aur.archlinux.org/obs-scale-to-sound.git  
cd obs-scale-to-sound  
makepkg -i
```

# Thanks
[brodi1](https://github.com/brodi1) and [tytan652](https://github.com/tytan652) for the [AUR Packages](https://aur.archlinux.org/packages/?O=0&K=obs-scale-to-sound)  
[Exeldro](https://github.com/exeldro) for the GitHub Actions workflow  
OBS Studio team and contributors for making such a great tool  
