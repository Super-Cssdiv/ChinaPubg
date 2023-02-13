# shellcheck disable=SC2034
SKIPUNZIP=1

VERSION=$(grep_prop version "${TMPDIR}/module.prop")
MODULE_LIB_NAME="%%%MODULE_LIB_NAME%%%"

unzip -oq "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print "*********************************************************"
  ui_print "! 这个Zip文件可能是错误的，请重新下载"
  abort "*********************************************************"
fi
. "$TMPDIR/verify.sh"

# Base check
extract "$ZIPFILE" 'util_functions.sh' "$TMPDIR"
. "$TMPDIR/util_functions.sh"

ui_print " "
ui_print "********************************************************"
ui_print "*  - [&]请先阅读 避免一些不必要的问题"
ui_print "********************************************************"
ui_print "*"
ui_print "*   - 安装模块前，请确保游戏已经安装"
ui_print "*   - 因模块采用了随机模块信息的原因，每次更新需卸载原模块"
ui_print "*   - 开挂本就是风险行为，请考虑好后再进行安装使用"
ui_print "*"
ui_print "*   - 如果你不想使用，或觉得不稳定，那么请按音量键－。"
ui_print "*"
ui_print "*   - 群绘助手 - Peace $VERSION - 芒果不忙 YYDS"
ui_print "*"
ui_print "********************************************************"
echo " "
ui_print "- 是否安装？(请选择)"
ui_print "- 按音量键＋: 安装 √"
ui_print "- 按音量键－: 退出 ×"
if [[ $(get_choose) != 0 ]]; then
   abort "- 已选择退出"
fi
echo " "
echo "- 开始生成随机信息"
sleep 2
ui_print "- 模块ID：$MODID"
ui_print "- 模块名称：$MODNAME"

extract "$ZIPFILE" 'customize.sh' "$TMPDIR"
extract "$ZIPFILE" 'verify.sh' "$TMPDIR"
echo " "

ui_print "- 开始获取环境信息"
sleep 3
var_CpuAbi=`dumpsys package com.tencent.tmgp.pubgmhd | grep -w 'primaryCpuAbi'`
var_Abi=${var_CpuAbi#*=}
var_model="$(getprop ro.product.model)"
var_version="$(getprop ro.build.version.release)"

ui_print "- 设备型号: $var_model"
ui_print "- 设备架构: $ARCH"
ui_print "- 系统版本: Android $var_version"
ui_print "- Magisk 版本: $MAGISK_VER_CODE"
ui_print "- 游戏架构: $var_Abi"

if [ "$API" -lt 26 ]; then
  ui_print "*********************************************************"
  ui_print "! 系统版本版本最低支持 Android 8.0"
  abort    "*********************************************************"
fi

if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ]; then
  ui_print "*********************************************************"
  ui_print "! 无法识别设备架构: $ARCH"
  abort    "*********************************************************"
fi

if [ "$var_Abi" != "arm64-v8a" ]; then
  ui_print "*********************************************************"
  ui_print "! 游戏架构: $var_Abi 不支持，模块仅支持 arm64-v8a 架构"
  abort    "*********************************************************"
fi
echo " "
enforce_install_from_magisk_app
echo " "

ui_print "- 开始解压资源"

# Extract libs
echo "id=$MODID" > $MODPATH/module.prop
echo "name=$MODNAME" >> $MODPATH/module.prop
echo "version=$MODVERSION" >> $MODPATH/module.prop
echo "versionCode=$MODVERSIONCODE" >> $MODPATH/module.prop
echo "author=$MODAUTHOR" >> $MODPATH/module.prop
echo "description=$MODDES" >> $MODPATH/module.prop
extract "$ZIPFILE" 'service.sh' "$MODPATH"

mkdir "$MODPATH/libs"

if [ "$ARCH" = "arm64" ]; then
  if [ "$IS64BIT" = true ]; then
    extract "$ZIPFILE" "lib/arm64-v8a/lib$MODULE_LIB_NAME.so" "$MODPATH/libs" true
    extract "$ZIPFILE" "lib/arm64-v8a/PeaceTools" "$MODPATH/libs" true
    mv "$MODPATH/libs/lib$MODULE_LIB_NAME.so" "$MODPATH/libs/arm64-v8a.so"
  fi
fi


set_perm_recursive "$MODPATH" 0 0 0755 0644