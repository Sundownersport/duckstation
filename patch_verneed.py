#!/usr/bin/env python3
"""
patch_verneed.py — patch ELF VERNEED version requirements for glibc 2.31 compat.

Replaces GLIBC_2.32/2.33/2.34 version entries with GLIBC_2.17 in the binary,
allowing it to load on glibc 2.31 devices.

In glibc 2.34, pthread/dl/rt symbols were merged into libc with new version
tags, but the actual ABI is identical. This patch just downgrades the version
requirement so the dynamic linker accepts them.

Usage: patch_verneed.py <binary>
"""

import sys
import struct


def elf_hash(name):
    h = 0
    for c in name:
        h = (h << 4) + ord(c)
        g = h & 0xF0000000
        if g:
            h ^= g >> 24
        h &= ~g
    return h


# Map version strings to replacement. Lengths must match (null-pad shorter ones).
PATCH_MAP = {
    b"GLIBC_2.32\x00": b"GLIBC_2.17\x00",
    b"GLIBC_2.33\x00": b"GLIBC_2.17\x00",
    b"GLIBC_2.34\x00": b"GLIBC_2.17\x00",
    b"GLIBC_2.24\x00": b"GLIBC_2.17\x00",
    b"GLIBC_2.27\x00": b"GLIBC_2.17\x00",
    b"GLIBC_2.30\x00": b"GLIBC_2.17\x00",
}

NEW_HASH = elf_hash("GLIBC_2.17")


def main():
    if len(sys.argv) != 2:
        print("Usage: patch_verneed.py <binary>", file=sys.stderr)
        sys.exit(1)

    path = sys.argv[1]
    with open(path, "rb") as f:
        data = bytearray(f.read())

    if data[:4] != b"\x7fELF":
        print("Not an ELF file", file=sys.stderr)
        sys.exit(1)

    ei_class = data[4]  # 1=32-bit, 2=64-bit
    ei_data = data[5]   # 1=LE, 2=BE
    endian = "<" if ei_data == 1 else ">"

    if ei_class == 2:
        # ELF64
        e_shoff = struct.unpack_from(endian + "Q", data, 40)[0]
        e_shentsize = struct.unpack_from(endian + "H", data, 58)[0]
        e_shnum = struct.unpack_from(endian + "H", data, 60)[0]
        sh_fmt = endian + "IIQQQQIIQQ"  # Elf64_Shdr
    else:
        # ELF32
        e_shoff = struct.unpack_from(endian + "I", data, 32)[0]
        e_shentsize = struct.unpack_from(endian + "H", data, 46)[0]
        e_shnum = struct.unpack_from(endian + "H", data, 48)[0]
        sh_fmt = endian + "IIIIIIIIII"  # Elf32_Shdr

    SHT_GNU_verneed = 0x6FFFFFFE

    sections = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        vals = struct.unpack_from(sh_fmt, data, off)
        sections.append(dict(
            name_idx=vals[0], type=vals[1],
            offset=vals[4], size=vals[5],
            link=vals[6], info=vals[7],
        ))

    verneed_sec = next((s for s in sections if s["type"] == SHT_GNU_verneed), None)
    if not verneed_sec:
        print("No .gnu.version_r section found")
        sys.exit(0)

    dynstr_sec = sections[verneed_sec["link"]]
    dynstr_start = dynstr_sec["offset"]
    dynstr_end = dynstr_start + dynstr_sec["size"]

    # Step 1: patch version strings in .dynstr
    patched = False
    dynstr_view = data[dynstr_start:dynstr_end]
    for old, new in PATCH_MAP.items():
        pos = 0
        while True:
            idx = dynstr_view.find(old, pos)
            if idx == -1:
                break
            abs_pos = dynstr_start + idx
            data[abs_pos: abs_pos + len(old)] = new
            dynstr_view = data[dynstr_start:dynstr_end]
            old_str = old.rstrip(b"\x00").decode()
            new_str = new.rstrip(b"\x00").decode()
            print(f"  [dynstr +0x{idx:04x}]  {old_str!r} -> {new_str!r}")
            patched = True
            pos = idx + len(new)

    if not patched:
        print("No GLIBC_2.32/2.33/2.34 strings found — already compatible")
        sys.exit(0)

    # Step 2: patch vna_hash in VERNEED entries
    vn_off = verneed_sec["offset"]
    num_entries = verneed_sec["info"]

    for _ in range(num_entries):
        vn_version, vn_cnt, vn_file, vn_aux, vn_next = \
            struct.unpack_from(endian + "HHIII", data, vn_off)

        vna_off = vn_off + vn_aux
        for _ in range(vn_cnt):
            vna_hash, vna_flags, vna_other, vna_name, vna_next = \
                struct.unpack_from(endian + "IHHII", data, vna_off)

            name_start = dynstr_start + vna_name
            name_end = data.index(0, name_start)
            name = bytes(data[name_start:name_end]).decode()

            if name == "GLIBC_2.17" and vna_hash != NEW_HASH:
                print(f"  [vernaux +0x{vna_off - verneed_sec['offset']:04x}]  "
                      f"hash 0x{vna_hash:08x} -> 0x{NEW_HASH:08x}  ({name})")
                struct.pack_into(endian + "I", data, vna_off, NEW_HASH)

            if vna_next == 0:
                break
            vna_off += vna_next

        if vn_next == 0:
            break
        vn_off += vn_next

    with open(path, "wb") as f:
        f.write(data)
    print(f"Patched {path}")


if __name__ == "__main__":
    main()
