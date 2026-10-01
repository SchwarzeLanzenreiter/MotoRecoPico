# -*- coding: utf-8 -*-
"""One machine-readable line from a kicad-cli DRC report: violations unconnected."""
import sys, json
r = json.load(open(sys.argv[1], encoding='utf-8'))
print(len(r.get('violations') or []), len(r.get('unconnected_items') or []))
