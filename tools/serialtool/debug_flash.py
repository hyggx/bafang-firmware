#!/usr/bin/env python3
"""
Raw debug flash script.
Dumps every packet byte-by-byte and constructs one MSG_PROG_FW manually,
so we can see exactly what the bootloader responds with.
"""

import sys
import time
import serial

PORT = "/dev/cu.usbserial-130"
BAUD = 38400

OBFUS = bytes([0x16,0x6c,0x14,0xe6,0x2e,0x91,0x0d,0x40,0x21,0x35,0xd5,0x40,0x13,0x03,0xe9,0x80])

MSG_NOTIFY_DEV_INFO = 0x0518
MSG_NOTIFY_BL_VER   = 0x0530
MSG_PROG_FW         = 0x0519
MSG_PROG_FW_RESP    = 0x051A

def obfus(buf, off, size):
    for i in range(size):
        buf[off+i] ^= OBFUS[i % len(OBFUS)]

def crc16(buf, off, size):
    crc = 0
    for i in range(size):
        crc ^= buf[off+i] << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xffff
            else:
                crc = (crc << 1) & 0xffff
    return crc

def make_packet(msg_bytes):
    msg = bytearray(msg_bytes)
    msg_len = len(msg)
    if msg_len % 2:
        msg_len += 1
    buf = bytearray(8 + msg_len)
    buf[0] = 0xAB; buf[1] = 0xCD
    buf[2] = msg_len & 0xFF; buf[3] = (msg_len >> 8) & 0xFF
    buf[4:4+len(msg)] = msg
    crc = crc16(buf, 4, msg_len)
    buf[4+msg_len] = crc & 0xFF; buf[5+msg_len] = (crc >> 8) & 0xFF
    buf[6+msg_len] = 0xDC; buf[7+msg_len] = 0xBA
    obfus(buf, 4, 2 + msg_len)
    return bytes(buf)

def make_msg(msg_type, data_len):
    buf = bytearray(4 + data_len)
    buf[0] = msg_type & 0xFF; buf[1] = (msg_type >> 8) & 0xFF
    buf[2] = data_len & 0xFF; buf[3] = (data_len >> 8) & 0xFF
    return buf

def set_hw_le(buf, off, val):
    buf[off] = val & 0xFF; buf[off+1] = (val >> 8) & 0xFF

def set_word_le(buf, off, val):
    buf[off]   =  val        & 0xFF
    buf[off+1] = (val >>  8) & 0xFF
    buf[off+2] = (val >> 16) & 0xFF
    buf[off+3] = (val >> 24) & 0xFF

def get_hw_le(buf, off):
    return buf[off] | (buf[off+1] << 8)

def fetch(buf):
    """Try to parse one packet from buf (bytearray). Returns (msg_bytes, consumed) or None."""
    while len(buf) >= 8:
        # Find AB CD
        if buf[0] != 0xAB or buf[1] != 0xCD:
            del buf[0]
            continue
        msg_len = get_hw_le(buf, 2)
        pack_end = 4 + msg_len + 2
        if len(buf) < pack_end + 2:
            return None  # incomplete
        if buf[pack_end] != 0xDC or buf[pack_end+1] != 0xBA:
            del buf[0:2]; continue  # bad end marker
        # de-obfuscate
        obfus(buf, 4, 2 + msg_len)
        msg = bytes(buf[4:4+msg_len])
        del buf[0:pack_end+2]
        return msg
    return None

def read_msg_of_type(ser, rxbuf, want_type, timeout=5.0):
    """Read until a packet with want_type arrives (beacons are printed but ignored)."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        chunk = ser.read(256)
        if chunk:
            rxbuf.extend(chunk)
        msg = fetch(rxbuf)
        if msg is None:
            continue
        mt = get_hw_le(msg, 0)
        if mt == want_type:
            return msg
        if mt == MSG_NOTIFY_DEV_INFO:
            print(f"  (beacon ignored, waiting for 0x{want_type:04x})")
        else:
            print(f"  (unexpected 0x{mt:04x} ignored)")
    return None

def dump_msg(label, msg):
    if msg is None:
        print(f"  {label}: <timeout>")
        return
    mt = get_hw_le(msg, 0)
    dl = get_hw_le(msg, 2)
    print(f"  {label}: type=0x{mt:04x} dlen={dl} raw=[{msg.hex()}]")
    if len(msg) >= 12:
        x4        = msg[4]|(msg[5]<<8)|(msg[6]<<16)|(msg[7]<<24)
        pg        = get_hw_le(msg, 8)
        err_or_pg = get_hw_le(msg, 10)
        print(f"           x4=0x{x4:08x} field8=0x{pg:04x} field10=0x{err_or_pg:04x}")

def main():
    if len(sys.argv) < 2:
        print("Usage: debug_flash.py <firmware.bin>")
        sys.exit(1)

    with open(sys.argv[1], "rb") as f:
        fw = f.read()
    print(f"Firmware: {len(fw)} bytes, {(len(fw)+255)//256} pages")

    ser = serial.Serial(PORT, BAUD, timeout=0.05)
    rxbuf = bytearray()

    # ── Phase 1: wait for 3 device-info beacons ──────────────────────────────
    print("\n[1] Waiting for device beacons (power-cycle the radio into boot mode)...")
    beacon_count = 0
    last_bl_ver = "7.0"
    while beacon_count < 3:
        msg = read_msg_of_type(ser, rxbuf, MSG_NOTIFY_DEV_INFO, timeout=30)
        if msg is None:
            print("  Timeout waiting for beacon")
            sys.exit(1)
        beacon_count += 1
        end = msg.find(0, 20, 36) if 0 in msg[20:36] else 36
        last_bl_ver = msg[20:end].decode("ascii", errors="replace")
        print(f"  Beacon #{beacon_count}: BL={last_bl_ver} raw=[{msg[:12].hex()}...]")

    # ── Phase 2: handshake ──────────────────────────────────────────────────
    print("\n[2] Handshaking (sending BL_VER 3 times)...")
    bl_ver_str = last_bl_ver[:4]
    for i in range(3):
        hs = make_msg(MSG_NOTIFY_BL_VER, 4)
        hs[4:4+len(bl_ver_str)] = bl_ver_str.encode("ascii")[:4]
        ser.write(make_packet(hs))
        ser.flush()
        print(f"  Sent BL_VER #{i+1}: [{bytes(hs).hex()}]")
        # wait for next beacon before sending the next BL_VER
        msg = read_msg_of_type(ser, rxbuf, MSG_NOTIFY_DEV_INFO, timeout=2)
        if msg:
            print(f"  Got beacon after BL_VER #{i+1}")
        else:
            print(f"  No beacon after BL_VER #{i+1} (timeout)")

    # 500 ms pause (matches UVTools2 JS)
    print("  Sleeping 500 ms after handshake...")
    time.sleep(0.5)
    # Drain any buffered data
    ser.reset_input_buffer()
    rxbuf.clear()

    # ── Phase 3: send page 0 and dump raw response ──────────────────────────
    print("\n[3] Sending MSG_PROG_FW page 0...")
    page_cnt = (len(fw) + 255) // 256
    x4 = int(time.time() * 1000) & 0xFFFFFFFF  # ms like JS

    prog_msg = make_msg(MSG_PROG_FW, 268)
    set_word_le(prog_msg, 4, x4)
    set_hw_le(prog_msg, 8, 0)         # page_index = 0
    set_hw_le(prog_msg, 10, page_cnt) # page_cnt
    # page data at [16..271]
    chunk = fw[0:256]
    prog_msg[16:16+len(chunk)] = chunk

    pkt = make_packet(prog_msg)
    print(f"  Request packet ({len(pkt)} bytes): [{pkt.hex()}]")
    ser.write(pkt)
    ser.flush()

    print("  Waiting for response (ignoring beacons)...")
    resp = read_msg_of_type(ser, rxbuf, MSG_PROG_FW_RESP, timeout=5)
    if resp is None:
        print("  <no response received>")
        sys.exit(1)

    mt = get_hw_le(resp, 0)
    dl = get_hw_le(resp, 2)
    print(f"  Response raw: [{resp.hex()}]")
    print(f"  msgType=0x{mt:04x} dataLen={dl}")
    if mt == MSG_PROG_FW_RESP:
        print("  -> MSG_PROG_FW_RESP")
        if len(resp) >= 12:
            x4r  = resp[4]|(resp[5]<<8)|(resp[6]<<16)|(resp[7]<<24)
            pg   = get_hw_le(resp, 8)
            err  = get_hw_le(resp, 10)
            print(f"     x4=0x{x4r:08x}  page_index={pg}  err={err}")
    else:
        print(f"  -> unexpected msg type 0x{mt:04x}")

    ser.close()

if __name__ == "__main__":
    main()
