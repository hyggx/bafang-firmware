#!/usr/bin/env python3
"""flash_font.py — Write the CJK font binary to Bafang (UV-K1) SPI Flash.

The font is written to EEPROM virtual address 0xD000, which the firmware maps
to SPI Flash physical address 0x020000 via the ADDR_MAPPINGS table in
driver/eeprom_compat.c (entry: _MK_MAPPING(0x020000, 0xD000, 0xFFFF)).

Window limitation: the EEPROM virtual address field is uint16_t, so the
mapping window tops out at 0xFFFF → only 12 KB (0xD000..0xFFFE) is writable
via this protocol.  The current 222-glyph font binary is 6232 bytes and fits
comfortably.  For the full 1359-glyph font (≈38 KB) a dedicated CMD_053x
direct-SPI write command will be needed.

The radio firmware must be built with ENABLE_CHINESE defined.

Requirements:
    pip install pyserial

Usage:
    python tools/serialtool/flash_font.py --port /dev/cu.usbserial-130 --font tools/cjk_font.bin
    python tools/serialtool/flash_font.py --port /dev/cu.usbserial-130 --font tools/cjk_font.bin --no-verify
    python tools/serialtool/flash_font.py --port COM3 --font tools/cjk_font.bin

UART protocol summary (UV-K5/K6/K1 proprietary):
    Frame  (host → radio):
        0xAB 0xCD               outer start (2 B)
        OuterSize  uint16_t LE  = InnerSize + 4
        ID         uint16_t LE  command ID
        InnerSize  uint16_t LE  payload size (bytes after the two header fields)
        Payload    InnerSize B  (XOR-obfuscated when encrypted; unencrypted here)
        CRC        uint16_t LE  CRC-16/CCITT over (ID + InnerSize + Payload)
        0xDC 0xBA               outer end   (2 B)

    After the 0x0514 handshake the firmware disables XOR obfuscation for the
    rest of the session, so all frames sent by this tool are plaintext.

Address mapping (firmware driver/eeprom_compat.c):
    EEPROM virtual 0xD000–0xFFFE  →  SPI Flash physical 0x020000–0x022FFE
    Current font (222 glyphs) is 6,232 bytes; 12 KB window is sufficient.
"""

import argparse
import struct
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("ERROR: pyserial not installed.  Run:  pip install pyserial")

# ---------------------------------------------------------------------------
# Protocol constants
# ---------------------------------------------------------------------------

_OBFUSCATION = bytes(
    [
        0x16, 0x6C, 0x14, 0xE6, 0x2E, 0x91, 0x0D, 0x40,
        0x21, 0x35, 0xD5, 0x40, 0x13, 0x03, 0xE9, 0x80,
    ]
)

# EEPROM virtual address for the font (maps to SPI Flash 0x020000)
FONT_EEPROM_BASE = 0xD000
# Size of the EEPROM virtual window reserved for the font.
# 0xFFFF - 0xD000 = 0x2FFF = 12,287 bytes (uint16_t address space limit).
FONT_EEPROM_SIZE = 0x2FFF  # 12 KB − 1

WRITE_CHUNK = 128  # bytes per 0x051D write (must be multiple of 8, max 128)
READ_CHUNK  = 128  # bytes per 0x051B read  (max 128; firmware buffer limit)
BAUD_RATE   = 38400
TIMEOUT_S   = 5.0  # generous: 128-byte write may trigger sector erase (~300 ms)


# ---------------------------------------------------------------------------
# CRC-16/CCITT  (poly 0x1021, init 0x0000, no input/output reflection)
# Matches the PY32F071 hardware CRC peripheral configuration in driver/crc.c
# ---------------------------------------------------------------------------


def _crc16(data: bytes) -> int:
    crc = 0
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if (crc & 0x8000) else (crc << 1)
        crc &= 0xFFFF
    return crc


# ---------------------------------------------------------------------------
# Frame assembly / parsing
# ---------------------------------------------------------------------------


def _build_frame(cmd_id: int, inner_data: bytes, encrypted: bool = True) -> bytes:
    """Assemble a complete host-to-radio UART frame."""
    inner_size = len(inner_data)
    outer_size = inner_size + 4  # ID(2) + InnerSize(2) + Payload(inner_size)
    header  = struct.pack("<HH", cmd_id, inner_size)
    payload = header + inner_data  # outer_size bytes
    crc     = struct.pack("<H", _crc16(payload))
    full    = payload + crc  # outer_size + 2 bytes
    if encrypted:
        full = bytes(b ^ _OBFUSCATION[i % 16] for i, b in enumerate(full))
    return b"\xab\xcd" + struct.pack("<H", outer_size) + full + b"\xdc\xba"


def _parse_reply(raw: bytes, expected_id: int, encrypted: bool = True) -> bytes:
    """Validate a radio-to-host reply frame; return inner payload bytes.

    Reply frame layout (radio → host):
        0xABCD        uint16_t LE  outer start marker
        Size          uint16_t LE  total payload size  (= 4 + inner_size)
        reply_id      uint16_t LE  reply command ID    (e.g. 0x0515)
        inner_size    uint16_t LE  inner payload bytes
        inner_data    inner_size B
        0xDCBA        uint16_t LE  outer end marker

    Note: reply frames have NO CRC field (unlike host→radio request frames).
    """
    idx = raw.find(b"\xab\xcd")
    if idx < 0:
        raise ValueError("No 0xABCD start marker in reply")
    idx += 2
    if idx + 2 > len(raw):
        raise ValueError("Reply too short (no size field)")
    size = struct.unpack_from("<H", raw, idx)[0]
    idx += 2  # now pointing at reply_id
    data = raw[idx : idx + size]
    if len(data) < size:
        raise ValueError(f"Reply truncated ({len(data)} B, need {size} B)")
    if encrypted:
        data = bytes(b ^ _OBFUSCATION[i % 16] for i, b in enumerate(data))
    if size < 4:
        raise ValueError(f"Reply data too short ({size} B) to contain header")
    reply_id, inner_size = struct.unpack_from("<HH", data, 0)
    if reply_id != expected_id:
        raise ValueError(
            f"Unexpected reply ID {reply_id:#06x} (expected {expected_id:#06x})"
        )
    return data[4 : 4 + inner_size]


def _send_recv(
    ser: "serial.Serial", frame: bytes, expected_id: int, encrypted: bool = True
) -> bytes:
    """Send a frame and collect the reply, waiting up to TIMEOUT_S."""
    ser.reset_input_buffer()
    ser.write(frame)
    ser.flush()
    raw = b""
    deadline = time.monotonic() + TIMEOUT_S
    while time.monotonic() < deadline:
        chunk = ser.read(ser.in_waiting or 1)
        if chunk:
            raw += chunk
        if b"\xdc\xba" in raw:
            break
        time.sleep(0.005)
    return _parse_reply(raw, expected_id, encrypted)


# ---------------------------------------------------------------------------
# Protocol operations
# ---------------------------------------------------------------------------


def _drain(ser: "serial.Serial") -> None:
    """Read and discard any pending data until the port is idle."""
    deadline = time.monotonic() + 0.5
    while time.monotonic() < deadline:
        n = ser.in_waiting
        if n:
            ser.read(n)
            deadline = time.monotonic() + 0.5
        else:
            time.sleep(0.05)


def _handshake(ser: "serial.Serial") -> tuple:
    """Drain port, then send CMD_0514 (session init); returns (version_str, timestamp).

    The bafang firmware always uses XOR obfuscation (bIsEncrypted is a
    compile-time constant true; the toggle code is commented out).
    """
    _drain(ser)
    ts    = int(time.time()) & 0xFFFFFFFF
    frame = _build_frame(0x0514, struct.pack("<I", ts))
    data  = _send_recv(ser, frame, expected_id=0x0515)
    # REPLY_0514_t.Data layout: Version[16], bHasCustomAesKey, bIsInLockScreen,
    #                            Padding[2], Challenge[16]
    version = data[:16].rstrip(b"\x00").decode("ascii", errors="replace")
    return version, ts


def _write_block(
    ser: "serial.Serial", eeprom_addr: int, chunk: bytes, timestamp: int
) -> None:
    """Send CMD_051D and wait for CMD_051E ACK.

    Inner payload layout (CMD_051D_t minus Header_t):
        Offset uint16_t, Size uint8_t, bAllowPassword uint8_t,
        Timestamp uint32_t, Data[Size]
    """
    inner     = struct.pack("<HBBI", eeprom_addr, len(chunk), 0, timestamp) + chunk
    frame     = _build_frame(0x051D, inner)
    rep       = _send_recv(ser, frame, expected_id=0x051E)
    # REPLY_051D_t.Data: Offset uint16_t
    rep_offset = struct.unpack_from("<H", rep, 0)[0]
    if rep_offset != eeprom_addr:
        raise ValueError(
            f"ACK offset {rep_offset:#06x} does not match sent {eeprom_addr:#06x}"
        )


def _read_block(
    ser: "serial.Serial", eeprom_addr: int, size: int, timestamp: int
) -> bytes:
    """Send CMD_051B and return the read bytes.

    Inner payload layout (CMD_051B_t minus Header_t):
        Offset uint16_t, Size uint8_t, Padding uint8_t, Timestamp uint32_t

    Reply inner data layout (REPLY_051B_t.Data):
        Offset uint16_t, Size uint8_t, Padding uint8_t, Data[Size]
    """
    inner = struct.pack("<HBBI", eeprom_addr, size, 0, timestamp)
    frame = _build_frame(0x051B, inner)
    rep   = _send_recv(ser, frame, expected_id=0x051C)
    return rep[4 : 4 + size]  # skip Offset(2)+Size(1)+Padding(1)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main() -> None:
    ap = argparse.ArgumentParser(
        description=(
            "Write CJK font binary to Bafang (UV-K1) SPI Flash.\n"
            f"Font is stored at EEPROM virtual 0x{FONT_EEPROM_BASE:04X} "
            f"→ SPI Flash physical 0x1F0000."
        )
    )
    ap.add_argument(
        "--port", required=True, help="Serial port (e.g. /dev/cu.usbserial-130 or COM3)"
    )
    ap.add_argument(
        "--font",
        default="tools/cjk_font.bin",
        help="Font binary produced by gen_cjk_font.py (default: tools/cjk_font.bin)",
    )
    ap.add_argument(
        "--baud", type=int, default=BAUD_RATE, help=f"Baud rate (default {BAUD_RATE})"
    )
    ap.add_argument(
        "--no-verify",
        dest="verify",
        action="store_false",
        help="Skip read-back verification after writing",
    )
    ap.set_defaults(verify=True)
    args = ap.parse_args()

    # Load and pad font file to 8-byte boundary (EEPROM writes 8 B at a time)
    with open(args.font, "rb") as fh:
        font_data = fh.read()
    if not font_data:
        sys.exit("ERROR: font file is empty")
    remainder = len(font_data) % 8
    if remainder:
        font_data += b"\xff" * (8 - remainder)

    end_addr = FONT_EEPROM_BASE + len(font_data) - 1
    if end_addr >= FONT_EEPROM_BASE + FONT_EEPROM_SIZE:
        sys.exit(
            f"ERROR: font too large — would end at EEPROM 0x{end_addr:04X} "
            f"(window limit 0x{FONT_EEPROM_BASE + FONT_EEPROM_SIZE - 1:04X})"
        )

    print(f"Font file  : {args.font}  ({len(font_data):,} bytes)")
    print(f"EEPROM virt: 0x{FONT_EEPROM_BASE:04X}..0x{end_addr:04X}")
    spi_base = 0x020000 + (FONT_EEPROM_BASE - FONT_EEPROM_BASE)
    print(f"SPI Flash  : 0x{spi_base:06X}..0x{spi_base + (end_addr - FONT_EEPROM_BASE):06X}  (physical)")
    print(f"Port       : {args.port} @ {args.baud} baud")

    with serial.Serial(args.port, args.baud, timeout=TIMEOUT_S) as ser:
        # --- Handshake ---------------------------------------------------
        try:
            version, ts = _handshake(ser)
        except Exception as exc:
            sys.exit(
                f"ERROR: handshake failed — {exc}\n"
                "  Check: radio powered on, USB connected, correct port,\n"
                "         firmware compiled with ENABLE_CHINESE defined."
            )
        print(f"Firmware   : {version}")

        # --- Write -------------------------------------------------------
        total   = len(font_data)
        written = 0
        print("Writing: ", end="", flush=True)
        for off in range(0, total, WRITE_CHUNK):
            chunk = font_data[off : off + WRITE_CHUNK]
            addr  = FONT_EEPROM_BASE + off
            try:
                _write_block(ser, addr, chunk, ts)
            except Exception as exc:
                print(f"\nERROR at EEPROM 0x{addr:04X}: {exc}", file=sys.stderr)
                sys.exit(1)
            written += len(chunk)
            print(
                ".",
                end=("" if written % (WRITE_CHUNK * 32) else "\n         "),
                flush=True,
            )
        print(f"  {written:,} bytes written.")

        if not args.verify:
            print("Verification skipped.")
            return

        # --- Verify (read back) ------------------------------------------
        print("Verifying: ", end="", flush=True)
        verified = 0
        ok = True
        for off in range(0, total, READ_CHUNK):
            size  = min(READ_CHUNK, total - off)
            addr  = FONT_EEPROM_BASE + off
            try:
                rb = _read_block(ser, addr, size, ts)
            except Exception as exc:
                print(f"\nERROR reading back EEPROM 0x{addr:04X}: {exc}", file=sys.stderr)
                sys.exit(1)
            if rb != font_data[off : off + size]:
                print(f"\nMISMATCH at EEPROM 0x{addr:04X}", file=sys.stderr)
                ok = False
                break
            verified += size
            print(".", end="", flush=True)

        if ok:
            print(f"  {verified:,} bytes verified OK.")
            print("Done. CJK font successfully written to SPI Flash.")
        else:
            sys.exit("Verification FAILED. Try writing again.")


if __name__ == "__main__":
    main()
