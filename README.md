# OBS Scale To Sound
A plugin for OBS Studio that adds a filter which makes a source scale based on the audio levels of any audio source you choose 

There are a few properties so you can fine tune how it behaves  
![Properties](https://qufy.cf/obs-scale-to-sound/plugin-properties-0-3-0.png)  
[OBS forums post](https://obsproject.com/forum/resources/scale-to-sound.1336/)  
This plugin is not extensively tested, you may come across bugs
# Installation
Get the archive for your OS from the [releases](https://github.com/Qufyy/obs-scale-to-sound/releases).  
### Windows
Make sure the folders get merged. Be careful to not overwrite the existing folders with the ones in the zip file  
Extract in the OBS installation folder, default is `C:\Program Files\obs-studio`

### macOS
Extract in `/Library/Application Support/obs-studio/plugins`

### Linux
Extract in `~/.config/obs-studio/plugins`  

If you're on Arch Linux or an Arch-based distro, you can get the [AUR package](https://aur.archlinux.org/packages/?O=0&K=obs-scale-to-sound).  
Use the [AUR helper](https://wiki.archlinux.org/title/AUR_helpers) of your choice, or install it manually like this
```bash
git clone https://aur.archlinux.org/obs-scale-to-sound-bin.git  
cd obs-scale-to-sound-bin  
makepkg -i
```

# Thanks
[brodi1](https://github.com/brodi1) for the [AUR Package](https://aur.archlinux.org/packages/?O=0&K=obs-scale-to-sound)  
[Exeldro](https://github.com/exeldro) for the GitHub Actions workflow  
OBS Studio team and contributors for making such a great tool  
