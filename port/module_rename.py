"""Renames a translated MODULE's tables so they can sit in one program with the title's (port/patch_modules.py).

usage: module_rename.py <cafe-recomp output folder of the module> <short name, e.g. mvplayer>
cafe_guest_functions / _count, cafe_guest_imports / _count and cafe_program_info become cafe_mod_<name>_functions,
..._function_count, ..._imports, ..._import_count, ..._info. Run after every cafe-recomp run of the module.
"""
import os
import re
import sys

folder, name = sys.argv[1], sys.argv[2]
names = {
    "cafe_guest_function_count": f"cafe_mod_{name}_function_count",
    "cafe_guest_functions": f"cafe_mod_{name}_functions",
    "cafe_guest_import_count": f"cafe_mod_{name}_import_count",
    "cafe_guest_imports": f"cafe_mod_{name}_imports",
    "cafe_program_info": f"cafe_mod_{name}_info",
}
pattern = re.compile(r"\b(" + "|".join(sorted(names, key=len, reverse=True)) + r")\b")
for file in ("function_table.cpp", "imports.cpp"):
    path = os.path.join(folder, file)
    text = open(path, encoding="utf-8").read()
    new = pattern.sub(lambda m: names[m.group(1)], text)
    if new != text:
        open(path, "w", encoding="utf-8", newline="").write(new)
        print("renamed:", path)
    else:
        print("unchanged:", path)
