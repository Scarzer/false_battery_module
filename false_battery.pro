TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

ARCH=x86
SRC_PROJECT_PATH = /home/scarzer/Projects/false_battery
LINUX_HEADERS_PATH = /usr/lib/modules/`uname -r`/build

SOURCES += \
    false_battery.c \
    test_program/test_netlink.c

INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/include -type d)
INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/arch/$$ARCH/include -type d)

DISTFILES += \
    Makefile \
    test_program/test_netlink \
    false_battery.ko \
    Module.symvers \
    modules.order

