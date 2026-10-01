# -*- coding: utf-8 -*-
"""Resistor values are written "100KOHMS" / "120OHMS" in the list, not "100k"."""
import sys, io, csv, re
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
SRC = r"C:\Users\root\.claude\projects\E--Claude-MotoRecoPico\4b4096a9-4169-4622-a77d-e17c28653a27\tool-results\webfetch-1785465766377-zzz3va.bin"
rows = list(csv.DictReader(open(SRC, encoding='utf-8', errors='replace')))

WANT = [('R1-R3', '100KOHMS'), ('R4', '22KOHMS'), ('R5-R7,R10-R13', '10KOHMS'),
        ('R8', '120OHMS'), ('R14', '1KOHMS')]
for refs, val in WANT:
    hits = [r for r in rows
            if '0402' in (r['Package'] or '')
            and re.search(r'\b' + re.escape(val) + r'\b', r['Description'] or '', re.I)]
    if hits:
        r = hits[0]
        print(f"  {refs:14s} {val:9s} 0402 -> {r['LCSC Part #']:9s} ${r['Price (USD)']}")
    else:
        any_pkg = [r for r in rows
                   if re.search(r'\b' + re.escape(val) + r'\b', r['Description'] or '', re.I)]
        pkgs = sorted({r['Package'].strip() for r in any_pkg})
        print(f"  {refs:14s} {val:9s} 0402 -> NOT in the list; other sizes: {pkgs or 'none'}")
