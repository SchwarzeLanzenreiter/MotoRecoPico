# -*- coding: utf-8 -*-
"""Check the passive values this board needs against the basic-parts list."""
import sys, io, csv, re
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
SRC = r"C:\Users\root\.claude\projects\E--Claude-MotoRecoPico\4b4096a9-4169-4622-a77d-e17c28653a27\tool-results\webfetch-1785465766377-zzz3va.bin"
rows = list(csv.DictReader(open(SRC, encoding='utf-8', errors='replace')))

WANT = [
    ('C2/C3/C4/C5/C10/C12 100nF 0402', r'100NF|0\.1UF', '0402'),
    ('C7/C8 22pF 0402',                r'\b22PF',        '0402'),
    ('C6/C11 10uF 0603',               r'\b10UF',        '0603'),
    ('R1-R3 100k 0402',                r'\b100K\b',      '0402'),
    ('R4 22k 0402',                    r'\b22K\b',       '0402'),
    ('R5-R7,R10-R13 10k 0402',         r'\b10K\b',       '0402'),
    ('R8 120R 0402',                   r'\b120R?\b|\b120 ', '0402'),
    ('R14 1k 0402',                    r'\b1K\b',        '0402'),
]
for label, pat, pkg in WANT:
    hits = [r for r in rows
            if pkg in (r['Package'] or '') and re.search(pat, r['Description'] or '', re.I)]
    print(f"### {label}: {len(hits)} hit(s)")
    for r in hits[:4]:
        print(f"  {r['LCSC Part #']:9s} ${r['Price (USD)']:>8s}  {(r['Description'] or '')[:88]}")
    if not hits:
        print("  -- nothing in the basic list at this size")
    print()
