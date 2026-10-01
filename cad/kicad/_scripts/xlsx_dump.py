# -*- coding: utf-8 -*-
"""Dump an xlsx without openpyxl - it is a zip of XML."""
import sys, io, zipfile, re
import xml.etree.ElementTree as ET
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

NS = '{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
path = sys.argv[1]
z = zipfile.ZipFile(path)
print("members:", z.namelist())

shared = []
if 'xl/sharedStrings.xml' in z.namelist():
    r = ET.fromstring(z.read('xl/sharedStrings.xml'))
    for si in r.findall(NS + 'si'):
        shared.append(''.join(t.text or '' for t in si.iter(NS + 't')))

for name in z.namelist():
    if not re.match(r'xl/worksheets/sheet\d+\.xml$', name):
        continue
    print(f"\n=== {name} ===")
    r = ET.fromstring(z.read(name))
    for row in r.iter(NS + 'row'):
        cells = []
        for c in row.findall(NS + 'c'):
            ref = c.get('r')
            t = c.get('t')
            v = c.find(NS + 'v')
            isel = c.find(NS + 'is')
            if t == 's' and v is not None:
                val = shared[int(v.text)]
            elif isel is not None:
                val = ''.join(x.text or '' for x in isel.iter(NS + 't'))
            elif v is not None:
                val = v.text
            else:
                val = ''
            cells.append(f"{ref}={val!r}")
        print(f"row {row.get('r'):>3}: " + ' | '.join(cells))
