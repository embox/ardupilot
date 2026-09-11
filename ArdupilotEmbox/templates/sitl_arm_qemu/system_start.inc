"export PWD=/mnt",
"export HOME=/",

"netmanager",

"mkdir -s /mnt",
"mount -t vfat /dev/mmc0 /mnt",
"cd /mnt",

// "arducopter -S --model JSON --speedup 1 --defaults /copter.parm --slave 0 --sim-address=10.0.2.10 -I0",
"arducopter -S --model copter --speedup 1 --defaults /copter.parm --slave 0 --sim-address=10.0.2.10 -I0",

"tish",
