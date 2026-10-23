# -*- coding: utf-8 -*-
"""Print what the DRC actually complained about (first few violations).

The seed loop only counts violations; when a seed fails repeatedly for the
same non-zero count, the types and positions are what identify the culprit.
"""
import sys, io, json
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
d = json.load(io.open(sys.argv[1], encoding='utf-8', errors='replace'))
for v in d.get('violations', [])[:6]:
    where = v.get('items', [{}])[0].get('pos', {})
    print(f"    {v.get('type','?')}: {v.get('description','')[:90]}"
          f" @ ({where.get('x','?')},{where.get('y','?')})")
