# -*- coding: utf-8 -*-
"""What do the resistor and capacitor rows actually look like in the basic list?"""
import sys, io, csv, re
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
SRC = r"C:\Users\root\.claude\projects\E--Claude-MotoRecoPico\4b4096a9-4169-4622-a77d-e17c28653a27\tool-results\webfetch-1785465766377-zzz3va.bin"
rows = list(csv.DictReader(open(SRC, encoding='utf-8', errors='replace')))

types = {}
for r in rows:
    types[r['Type']] = types.get(r['Type'], 0) + 1
print("Type column:", types)

print("\n### sample rows containing RESIST")
n = 0
for r in rows:
    if re.search('RESIST', (r['Description'] or ''), re.I):
        print(f"  {r['LCSC Part #']:9s} {r['Package']:14s} {(r['Description'] or '')[:90]}")
        n += 1
        if n >= 10:
            break
print(f"  total resistor rows: {sum(1 for r in rows if re.search('RESIST', r['Description'] or '', re.I))}")

print("\n### every 0402 / 0603 capacitor in the list, by voltage")
for r in rows:
    d = r['Description'] or ''
    if 'MLCC' in d and ('0402' in (r['Package'] or '') or '0603' in (r['Package'] or '')):
        m = re.search(r'([\d.]+[NUP]F)\s+([\d.]+V)', d, re.I)
        if m:
            print(f"  {r['LCSC Part #']:9s} {r['Package']:8s} {m.group(1):8s} {m.group(2)}")
