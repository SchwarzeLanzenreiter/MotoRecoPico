# -*- coding: utf-8 -*-
"""Summarise a kicad-cli DRC json report by rule type."""
import sys, io, json, collections
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

r = json.load(open(sys.argv[1], encoding='utf-8'))
for key in ('violations', 'unconnected_items', 'schematic_parity'):
    items = r.get(key) or []
    print(f"\n===== {key}: {len(items)}")
    by = collections.defaultdict(list)
    for v in items:
        by[(v.get('severity'), v.get('type'))].append(v)
    for (sev, typ), vs in sorted(by.items(), key=lambda kv: -len(kv[1])):
        print(f"  [{sev}] {typ}: {len(vs)}")
        for v in vs[:3]:
            where = "; ".join(
                f"{i.get('description','')}" for i in (v.get('items') or [])[:2])
            print(f"      {v.get('description','')[:110]}")
            if where:
                print(f"        -> {where[:150]}")
