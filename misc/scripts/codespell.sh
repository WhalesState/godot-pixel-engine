#!/bin/sh
SKIP_LIST="./.*,./**/.*,./bin,./thirdparty,*.desktop,*.gen.*,*.po,*.pot,*.rc,./AUTHORS.md,./COPYRIGHT.txt,./DONORS.md,"
SKIP_LIST+="./core/input/gamecontrollerdb.txt,./core/string/locales.h,./misc/scripts/codespell.sh,"

IGNORE_LIST="breaked,curvelinear,doubleclick,expct,findn,gird,hel,inout,lod,mis,nd,numer,ot,requestor,te,vai"

codespell -w -q 3 -S "${SKIP_LIST}" -L "${IGNORE_LIST}" --builtin "clear,rare,en-GB_to_en-US"
