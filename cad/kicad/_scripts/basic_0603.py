# -*- coding: utf-8 -*-
"""If every chip became a 1608, would JLCPCB still have them all as basic parts?"""
import sys, io, csv, re
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
SRC = r"C:\Users\root\.claude\projects\E--Claude-MotoRecoPico\4b4096a9-4169-4622-a77d-e17c28653a27\tool-results\webfetch-1785465766377-zzz3va.bin"
rows = list(csv.DictReader(open(SRC, encoding='utf-8', errors='replace')))

WANT = [('R1-R3', '100KOHMS'), ('R4', '22KOHMS'), ('R5-R7,R10-R13', '10KOHMS'),
        ('R8', '120OHMS'), ('R14', '1KOHMS')]
for pkg in ('0402', '0603'):
    print(f"--- {pkg} ---")
    for refs, val in WANT:
        hits = [r for r in rows if pkg in (r['Package'] or '')
                and re.search(r'\b' + re.escape(val) + r'\b', r['Description'] or '', re.I)]
        print(f"  {refs:14s} {val:9s} {'-> ' + hits[0]['LCSC Part #'] if hits else 'NOT FOUND'}")
    for val in ('100NF', '22PF'):
        hits = [r for r in rows if pkg in (r['Package'] or '') and 'MLCC' in (r['Description'] or '')
                and re.search(r'\b' + val + r'\b', r['Description'] or '', re.I)]
        for h in hits:
            m = re.search(r'([\d.]+V)', h['Description'])
            print(f"  {'C ' + val:14s} {'':9s} -> {h['LCSC Part #']} ({m.group(1) if m else '?'})")
        if not hits:
            print(f"  {'C ' + val:14s} {'':9s} NOT FOUND")
