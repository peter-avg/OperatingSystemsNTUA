cmd_/home/user/shared/lunix/modules.order := {   echo /home/user/shared/lunix/lunix.ko; :; } | awk '!x[$$0]++' - > /home/user/shared/lunix/modules.order
