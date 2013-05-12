import atos
#atos.symbolicate("armv7s", "CrashTest3Dwarf.thin", ("0x0000b1e7", "0x0000b1e8"))
atos.symbolicate("armv7s", "CrashTest3Dwarf.thin", tuple(["0x0000b1e7"]))
atos.symbolicate("armv7s", "CrashTest3Dwarf.fat.error", tuple(["0x00006ed7"]))
