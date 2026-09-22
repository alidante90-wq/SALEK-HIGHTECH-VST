#!/usr/bin/env python3
"""Generate GITI Early-Access codes matching Source/License/SalekLicense.cpp"""
import sys

PEPPER = bytes([
    0x53,0x41,0x4c,0x45,0x4b,0x2d,0x47,0x49,0x54,0x49,0x2d,0x45,0x41,
    0x32,0x30,0x32,0x36,0x2d,0x48,0x54,0x2d,0x9a,0x3c,0x71,0xe2,0x44
])

def normalize(s: str) -> str:
    return "".join(c for c in s.upper() if c.isalnum())

def hash_hex(s: str) -> str:
    data = s.encode("utf-8")
    h = 1469598103934665603
    for b in data:
        h ^= b
        h = (h * 1099511628211) & ((1 << 64) - 1)
        for p in PEPPER:
            h ^= p
            h = ((h << 7) | (h >> 57)) & ((1 << 64) - 1)
    out = []
    for i in range(3):
        part = (h >> (i * 16)) & 0xFFFF
        out.append(f"{part:04X}")
    return "".join(out)

def gen(machine_id: str, days: int = 0) -> str:
    mid = normalize(machine_id)
    payload = f"{mid}|D{days}|GITI-EA1"
    h = hash_hex(payload)
    return f"SALEK-{h[0:4]}-{h[4:8]}-{h[8:12]}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: generate_ea_code.py <MACHINE_ID> [days=0 unlimited]")
        sys.exit(1)
    days = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    print(gen(sys.argv[1], days))
