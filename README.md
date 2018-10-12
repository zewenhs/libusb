# libusb

## 编译libusb1.0.so 及libtelink-usb.so步骤：

1. 进入 `cd android/jni` 目录
2. `export NDK=/home/zewen/Android/Sdk/ndk-bundle` 
3. Run "$NDK/ndk-build"
	after that, The libusb library, examples and tests can then be found in:
    "android/libs/$ARCH"

4. 其中 `libtelink-usb.so` 的源文件在 `libtelink-usb/` 路径下， 涉及到的编译文件为 `android/jni/Android.mk`， `android/jni/libtelink-usb.mk`


[![Build Status](https://travis-ci.org/libusb/libusb.svg?branch=master)](https://travis-ci.org/libusb/libusb)
[![Build status](https://ci.appveyor.com/api/projects/status/xvrfam94jii4a6lw?svg=true)](https://ci.appveyor.com/project/LudovicRousseau/libusb)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2180/badge.svg)](https://scan.coverity.com/projects/libusb-libusb)

libusb is a library for USB device access from Linux, macOS,
Windows, OpenBSD/NetBSD and Haiku userspace.
It is written in C (Haiku backend in C++) and licensed under the GNU
Lesser General Public License version 2.1 or, at your option, any later
version (see [COPYING](COPYING)).

libusb is abstracted internally in such a way that it can hopefully
be ported to other operating systems. Please see the [PORTING](PORTING)
file for more information.

libusb homepage:
http://libusb.info/

Developers will wish to consult the API documentation:
http://api.libusb.info

Use the mailing list for questions, comments, etc:
http://mailing-list.libusb.info

- Hans de Goede <hdegoede@redhat.com>
- Xiaofan Chen <xiaofanc@gmail.com>
- Ludovic Rousseau <ludovic.rousseau@gmail.com>
- Nathan Hjelm <hjelmn@cs.unm.edu>
- Chris Dickens <christopher.a.dickens@gmail.com>

(Please use the mailing list rather than mailing developers directly)
