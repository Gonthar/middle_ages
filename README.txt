Uruchamianie:
- na studentsie (PLD) przez ./sredniowiecze_gui_with_libs.sh <parametry>
- na w miarę standardowym Linuxie z zainstalowanymi bibliotekami Qt 5 najlepiej ./sredniowiecze_gui
- gdy nie działa (bo np. nie ma bibliotek) spróbować ./sredniowiecze_gui_with_libs.sh
- gdy nadal nie działa, to albo debuggować biblioteki i bawić się z LD_LIBRARY_PATH/RPATH albo ssh -X na studentsa

Skompilowane z biblioteką Qt 5.2.1. Może wymagać paczek libqt5widgets5, libqt5gui5, libqt5core5 albo podobnych. To dość stara wersja Qt 5 (z Linux Mint 17), a Qt 5 jest raczej kompatybilne wstecz, więc powinna działać na większości dystrybucji, gdzie w standardowych lokacjach są biblioteki Qt 5. Konkretnie Qt 5 do działania potrzebuje paru bibliotek (http://doc.qt.io/qt-5/linux-requirements.html) oraz biblioteki-plugina niewidocznego przez ldd do obsługi X-ów.
