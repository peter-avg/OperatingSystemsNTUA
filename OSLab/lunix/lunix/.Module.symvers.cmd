cmd_/home/user/shared/lunix/Module.symvers := sed 's/ko$$/o/' /home/user/shared/lunix/modules.order | scripts/mod/modpost -m    -o /home/user/shared/lunix/Module.symvers -e -i Module.symvers   -T -
