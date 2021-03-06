#!/bin/bash

# change this to what version of Xcode you have installed
SDKVER=8.2

scons TARGET_OS=ios TARGET_ARCH=armv7 SYS_VERSION=$SDKVER RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=armv7s SYS_VERSION=$SDKVER RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=arm64 SYS_VERSION=$SDKVER RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=i386 SYS_VERSION=$SDKVER RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=x86_64 SYS_VERSION=$SDKVER RELEASE=false
