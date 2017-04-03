# iOS {#SettingUpOgre_iOS}

@tableofcontents

# Requirements {#Requirements_iOS}
    * [CMake 3.x](https://cmake.org/download/)
    * Mercurial. We recommend [TortoiseHg](https://tortoisehg.bitbucket.io/download/index.html)
    * What you do **NOT** need: Boost. Don't waste your time.
    * XCode 8 or newer
    * iOS 9 or newer.
    * [Metal-capable device](https://developer.apple.com/library/content/documentation/DeviceInformation/Reference/iOSDeviceCompatibility/HardwareGPUInformation/HardwareGPUInformation.html#//apple_ref/doc/uid/TP40013599-CH106-SW1)
      \(Apple A7 GPU or newer. That means iPhone 5s or newer, iPad Air or newer, iPad mini 2 or newer, iPad Pro\)
    * For HW & SW requirements, please visit http://www.ogre3d.org/developers/requirements

# Downloading Ogre {#DownloadingOgre_iOS}

Clone Ogre as in [Linux](#DownloadingOgreLinux).
Download iOS prebuilt dependencies from:
https://sourceforge.net/projects/ogre/files/ogre-dependencies-mac/1.9/Ogre_iOS_7.0_Dependencies_20140315.dmg/download
and unzip them into Ogre/iOSDependencies

# Building Ogre {#BuildingOgre_iOS}

```sh
cd Ogre
mkdir build
cd build
cmake -D OGRE_BUILD_PLATFORM_APPLE_IOS=1 -D OGRE_BUILD_RENDERSYSTEM_METAL=1 -D OGRE_USE_BOOST=0 -D OGRE_CONFIG_THREAD_PROVIDER=0 -D OGRE_CONFIG_THREADS=0 -D OGRE_BUILD_SAMPLES2=1 -D OGRE_UNITY_BUILD=1 -D OGRE_SIMD_NEON=1 -D OGRE_BUILD_TESTS=0 -G Xcode ..
```

Now open Ogre/build/OGRE.xcodeproj in XCode and build it.
You may have to set up the provisioning profiles of each sample before running in it on the device.
