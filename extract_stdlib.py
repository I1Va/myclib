#!/usr/bin/env python3
import sys
import struct
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

def main(elf_path, combined_out):
    with open(elf_path, 'rb') as f:
        elf = ELFFile(f)

        text_section = None 
        for section in elf.iter_sections():
            if section.name == '.text':
                text_section = section
                break
        if not text_section:
            print("No .text section found", file=sys.stderr)
            sys.exit(1)

        symtab = None
        for section in elf.iter_sections():
            if isinstance(section, SymbolTableSection):
                symtab = section
                break
        if not symtab:
            print("No symbol table found", file=sys.stderr)
            sys.exit(1)

        text_start = text_section['sh_addr']
        text_end = text_start + text_section['sh_size']
        offsets = []

        for symbol in symtab.iter_symbols():
            if symbol['st_info']['type'] == 'STT_FUNC' and symbol.name:
                addr = symbol['st_value']
                if text_start <= addr < text_end:
                    offset = addr - text_start
                    offsets.append((symbol.name, offset))

        if not offsets:
            print("No functions found", file=sys.stderr)
            sys.exit(1)

        print("\n=== EXTRACTED OFFSET TABLE DUMP ===")
        print(f"{'Function Name':<40} | {'Offset (Hex)':<12} | {'Offset (Dec)':<12}")
        print("-" * 70)
        for name, offset in sorted(offsets, key=lambda x: x[1]):
            print(f"{name:<40} | 0x{offset:08X}   | {offset:<12}")
        print("=" * 70 + "\n")

        with open(combined_out, 'wb') as out:
            out.write(b'STDL')                     
            out.write(struct.pack('<I', 1))        
            out.write(struct.pack('<I', len(offsets)))

            for name, offset in offsets:
                out.write(name.encode('utf-8') + b'\0')
                out.write(struct.pack('<I', offset))

            out.write(text_section.data())

    print(f"Created {combined_out} with {len(offsets)} functions")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: extract_stdlib.py <elf> <combined_out>", file=sys.stderr)
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])