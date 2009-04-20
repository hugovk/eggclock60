pushd ..\sis\
move eggclock.sis eggclock.sis.bak
move eggclock.sisx eggclock.sisx.bak
call makesis eggclock.pkg
call signsis eggclock.sis eggclock.sisx eggclock.cer eggclock.key password
call eggclock.sisx
popd
