#!/usr/bin/env python3
"""flash_font.py — Write the CJK font binary to Bafang (UV-K1) SPI Flash.

Two write paths are supported, selected automatically based on font size:

  ≤ 12 KB  (menu-only font, ≤ 222 glyphs):
      CMD_051D  EEPROM virtual address 0xD000 → SPI Flash 0x020000
      Readable back via CMD_051B for verification.

  > 12 KB  (full IME font, up to 37.5 KB / 1372 glyphs):
      CMD_0535  Direct SPI Flash write at physical address 0x020000
      Readable back via CMD_0537  Direct SPI Flash read (for verification).
      Requires firmware built with ENABLE_UART_SPI_WRITE.

The radio firmware must be built with ENABLE_CHINESE defined.

Requirements:
    pip install pyserial

Usage:
    python tools/serialtool/flash_font.py --port /dev/cu.usbserial-130 --font tools/cjk_font_menu.bin
    python tools/serialtool/flash_font.py --port /dev/cu.usbserial-130 --font tools/cjk_font.bin
    python tools/serialtool/flash_font.py --port COM3 --font tools/cjk_font.bin --no-verify
    python tools/serialtool/flash_font.py --port /dev/cu.usbserial-130 --font tools/cjk_font.bin --direct-spi

UART protocol summary (UV-K5/K6/K1 proprietary):
    Frame  (host → radio):
        0xAB 0xCD               outer start (2 B)
        OuterSize  uint16_t LE  = InnerSize + 4
        ID         uint16_t LE  command ID
        InnerSize  uint16_t LE  payload size (bytes after the two header fields)
        Payload    InnerSize B  (XOR-obfuscated)
        CRC        uint16_t LE  CRC-16/CCITT over (ID + InnerSize + Payload)
        0xDC 0xBA               outer end   (2 B)

Address mapping (firmware driver/eeprom_compat.c):
    EEPROM virtual 0xD000–0xFFFE  →  SPI Flash physical 0x020000–0x022FFE
    12 KB EEPROM window is sufficient for the menu-only font.
    The full IME font (37.5 KB) uses CMD_0535 direct-SPI write at 0x020000.
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

WRITE_CHUNK       = 128  # bytes per CMD_0535 write (SPI direct path)
EEPROM_WRITE_CHUNK = 8   # bytes per CMD_051D write; must be 8 (EEPROM_WriteBuffer granularity)
                         # 128-byte blocks cause 16 × sector-erase per block (~400 ms each =
                         # 6.4 s worst case), which exceeds a 5 s timeout when re-flashing over
                         # existing data.  Sending 8 bytes per CMD_051D keeps each round-trip
                         # to one sector-erase (~400 ms max), safely within TIMEOUT_S.
READ_CHUNK  = 128  # bytes per 0x051B / 0x0537 read  (max 128)
BAUD_RATE   = 38400
TIMEOUT_S   = 3.0  # seconds; one CMD_051D (8 bytes) triggers at most one 4 KB sector erase
                   # (PY25Q16 spec: typ 50 ms, max 400 ms) — 3 s is a safe margin.
                   # CMD_0535 (direct SPI, 128 bytes) also causes one erase → 3 s is fine.


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


def _write_block_spi(ser: "serial.Serial", spi_addr: int, chunk: bytes) -> None:
    """Send CMD_0535 (direct SPI Flash write) and wait for CMD_0536 ACK.

    Requires firmware built with ENABLE_UART_SPI_WRITE.

    Inner payload layout (CMD_0535_t minus Header_t):
        Addr uint32, DataLen uint16, Padding[2], Data[DataLen]

    Reply inner data layout (REPLY_0535_t minus Header_t):
        Result uint8, Padding[3]   (Result 0=OK, 1=bad length)
    """
    inner = struct.pack("<IH2x", spi_addr, len(chunk)) + chunk
    frame = _build_frame(0x0535, inner)
    rep   = _send_recv(ser, frame, expected_id=0x0536)
    result = rep[0] if rep else 0xFF
    if result != 0:
        raise ValueError(f"CMD_0535 error result {result} at SPI addr {spi_addr:#010x}")


def _read_block_spi(ser: "serial.Serial", spi_addr: int, size: int) -> bytes:
    """Send CMD_0537 (direct SPI Flash read) and return the data bytes.

    Requires firmware built with ENABLE_UART_SPI_WRITE.

    Inner payload layout (CMD_0537_t minus Header_t):
        Addr uint32, ReadLen uint8, Padding[3]

    Reply inner data layout (REPLY_0537_t minus Header_t, variable length):
        Addr uint32, ReadLen uint8, Result uint8, Padding[2], Data[ReadLen]
        (Result 0=OK, 1=bad length)
    """
    inner = struct.pack("<IB3x", spi_addr, size)
    frame = _build_frame(0x0537, inner)
    rep   = _send_recv(ser, frame, expected_id=0x0538)
    if len(rep) < 8:
        raise ValueError(f"CMD_0537 reply too short ({len(rep)} B) at SPI addr {spi_addr:#010x}")
    echo_addr, echo_len, result = struct.unpack_from("<IBBxx", rep, 0)
    if result != 0:
        raise ValueError(f"CMD_0537 error result {result} at SPI addr {spi_addr:#010x}")
    return rep[8 : 8 + echo_len]


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
    ap.add_argument(
        "--direct-spi",
        dest="direct_spi",
        action="store_true",
        help=(
            "Force CMD_0535/0537 direct-SPI path even for small fonts. "
            "Requires firmware with ENABLE_UART_SPI_WRITE."
        ),
    )
    ap.set_defaults(verify=True, direct_spi=False)
    args = ap.parse_args()

    # Load font file; pad to 8-byte boundary (EEPROM write granularity).
    with open(args.font, "rb") as fh:
        font_data = fh.read()
    if not font_data:
        sys.exit("ERROR: font file is empty")
    remainder = len(font_data) % 8
    if remainder:
        font_data += b"\xff" * (8 - remainder)

    # Decide write path:
    #   EEPROM path  — font ≤ 12 KB and not --direct-spi
    #   Direct-SPI   — font > 12 KB  OR  --direct-spi flag
    use_direct_spi = args.direct_spi or (len(font_data) > FONT_EEPROM_SIZE)
    SPI_FONT_BASE = 0x020000  # physical address in PY25Q16 flash

    if use_direct_spi:
        write_desc = f"SPI Flash  : 0x{SPI_FONT_BASE:06X}..0x{SPI_FONT_BASE + len(font_data) - 1:06X}  (CMD_0535 direct)"
        path_name  = "direct-SPI (CMD_0535/0537)"
    else:
        end_eeprom = FONT_EEPROM_BASE + len(font_data) - 1
        write_desc = (
            f"EEPROM virt: 0x{FONT_EEPROM_BASE:04X}..0x{end_eeprom:04X}  "
            f"→ SPI 0x{SPI_FONT_BASE:06X}..0x{SPI_FONT_BASE + len(font_data) - 1:06X}  (CMD_051D)"
        )
        path_name = "EEPROM-virtual (CMD_051D/051B)"

    print(f"Font file  : {args.font}  ({len(font_data):,} bytes)")
    print(write_desc)
    print(f"Write path : {path_name}")
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
        step = WRITE_CHUNK if use_direct_spi else EEPROM_WRITE_CHUNK
        for off in range(0, total, step):
            chunk = font_data[off : off + step]
            try:
                if use_direct_spi:
                    _write_block_spi(ser, SPI_FONT_BASE + off, chunk)
                else:
                    _write_block(ser, FONT_EEPROM_BASE + off, chunk, ts)
            except Exception as exc:
                addr_str = f"SPI 0x{SPI_FONT_BASE + off:06X}" if use_direct_spi else f"EEPROM 0x{FONT_EEPROM_BASE + off:04X}"
                print(f"\nERROR at {addr_str}: {exc}", file=sys.stderr)
                sys.exit(1)
            written += len(chunk)
            dot_interval = step * 128  # print '.' every ~1 KB
            print(
                ".",
                end=("" if written % dot_interval else "\n         "),
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
            size = min(READ_CHUNK, total - off)
            try:
                if use_direct_spi:
                    rb = _read_block_spi(ser, SPI_FONT_BASE + off, size)
                else:
                    rb = _read_block(ser, FONT_EEPROM_BASE + off, size, ts)
            except Exception as exc:
                addr_str = f"SPI 0x{SPI_FONT_BASE + off:06X}" if use_direct_spi else f"EEPROM 0x{FONT_EEPROM_BASE + off:04X}"
                print(f"\nERROR reading back {addr_str}: {exc}", file=sys.stderr)
                sys.exit(1)
            if rb != font_data[off : off + size]:
                addr_str = f"SPI 0x{SPI_FONT_BASE + off:06X}" if use_direct_spi else f"EEPROM 0x{FONT_EEPROM_BASE + off:04X}"
                print(f"\nMISMATCH at {addr_str}", file=sys.stderr)
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
