
enforce_install_from_magisk_app() {
  if $BOOTMODE; then
    ui_print "- 开始安装，即将进入愉快的游戏之旅"
  else
    ui_print "*********************************************************"
    ui_print "! 该模块无法在 recovery 模式下安装"
    ui_print "! 在 recovery 模块下安装该模块，会出现一些未知的问题"
    ui_print "! 请从 Magisk app 中安装该模块"
    abort "*********************************************************"
  fi
}

get_choose() {
   local choose
   local branch
   while :; do
      choose="$(getevent -qlc 1 | awk '{ print $3 }')"
      case "$choose" in
      KEY_VOLUMEUP)   branch="0" ;;
      KEY_VOLUMEDOWN)   branch="1" ;;
      *)   continue ;;
      esac
      echo "$branch"
      break
   done
}