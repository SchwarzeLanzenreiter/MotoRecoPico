# -*- coding: utf-8 -*-
"""Search a downloaded copy of JLCPCB's basic-parts list for what this board needs."""
import sys, io, csv, re
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
SRC = r"C:\Users\root\.claude\projects\E--Claude-MotoRecoPico\4b4096a9-4169-4622-a77d-e17c28653a27\tool-results\webfetch-1785465766377-zzz3va.bin"

with open(SRC, encoding='utf-8', errors='replace') as f:
    rows = list(csv.DictReader(f))
print(f"{len(rows)} rows; columns: {list(rows[0].keys())}\n")

QUERIES = {
    'NPN SOT-23 (Q1 MMBT3904)': r'NPN.*SOT-23',
    'P-ch MOSFET (Q2)': r'MOSFET P',
    'Schottky SOD-123 (D1/D3)': r'SCHOTTKY.*SOD-123',
    'Schottky any package': r'SCHOTTKY',
    'Zener (D4 3.3V)': r'ZENER',
    'TVS (D2)': r'TVS',
    'PPTC / fuse (F1)': r'PPTC|FUSE|RESETTABLE',
    'LED (D6)': r'LIGHT EMITTING|LED',
    'microSD / card socket (J2)': r'SD|CARD|SOCKET|CONNECTOR',
    'CAN transceiver/controller (U2)': r'CAN |TRANSCEIVER',
}
for label, pat in QUERIES.items():
    hits = [r for r in rows if re.search(pat, r['Description'] or '', re.I)]
    print(f"### {label}: {len(hits)} hit(s)")
    for r in hits[:12]:
        print(f"  {r['LCSC Part #']:9s} {r['MFR.Part #']:20s} {r['Package']:16s} "
              f"${r['Price (USD)']:>8s}  {(r['Description'] or '')[:78]}")
    print()

# what packages does the basic list actually cover, for the passives
print("### 0402 / 0603 passive coverage")
for pkg in ('0402', '0603', '1206'):
    n = sum(1 for r in rows if pkg in (r['Package'] or ''))
    print(f"  {pkg}: {n} parts")
