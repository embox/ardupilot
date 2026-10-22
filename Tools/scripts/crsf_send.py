#!/usr/bin/env python3
"""Send CRSF RC_CHANNELS_PACKED frames to a Linux UART or QEMU PTY.

Example: python3 Tools/scripts/crsf_send.py /dev/pts/5 --channel 1=1700
Use --count 1 for a single frame, or Ctrl-C to stop the default 50 Hz stream.
The guest must feed its RC protocol decoder at 416666 baud and must not use
the CRSF telemetry-only standalone mode. A PTY baud setting does not configure
the UART inside the guest.
"""

import argparse
import fcntl
import math
import os
import struct
import sys
import termios
import time
import tty


def crc8_dvb_s2(data):
    crc = 0
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ (0xD5 if crc & 0x80 else 0)) & 0xFF
    return crc


def make_frame(channels_us):
    """Pack 16 PWM values using the inverse of ArduPilot's 5/8 + 880 scale."""
    if len(channels_us) != 16 or any(not 1000 <= value <= 2000 for value in channels_us):
        raise ValueError("expected 16 channel values in 1000..2000 us")
    packed = 0
    for index, value in enumerate(channels_us):
        raw = ((value - 880) * 8 + 2) // 5
        packed |= raw << (11 * index)
    body = bytes([0x16]) + packed.to_bytes(22, "little")
    return bytes([0xC8, len(body) + 1]) + body + bytes([crc8_dvb_s2(body)])


def set_baud(fd, baud):
    """Linux termios2/BOTHER supports the nonstandard CRSF baud rate."""
    # Linux asm-generic termios2 ABI (including x86 and ARM).
    layout = "=IIIIB19sII"
    settings = bytearray(struct.calcsize(layout))
    fcntl.ioctl(fd, 0x802C542A, settings)  # TCGETS2
    fields = list(struct.unpack(layout, settings))
    fields[2] = (fields[2] & ~0x100F) | 0x1000  # CBAUD / BOTHER
    fields[-2:] = [baud, baud]
    fcntl.ioctl(fd, 0x402C542B, struct.pack(layout, *fields))  # TCSETS2


def channel_override(text):
    try:
        channel, value = map(int, text.split("="))
        if 1 <= channel <= 16 and 1000 <= value <= 2000:
            return channel - 1, value
    except ValueError:
        pass
    raise argparse.ArgumentTypeError("use CHANNEL=PWM, channel 1..16, PWM 1000..2000 us")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("tty", help="Linux serial device or QEMU PTY, e.g. /dev/pts/5")
    parser.add_argument("--baud", type=int, default=416666, help="host baud rate (default: 416666)")
    parser.add_argument("--rate", type=float, default=50, help="frames per second (default: 50)")
    parser.add_argument("--count", type=int, default=0, help="number of frames; 0 means continuous (default)")
    parser.add_argument("--channel", action="append", type=channel_override, default=[], metavar="N=PWM",
                        help="override channel 1..16 in microseconds; repeat as needed")
    args = parser.parse_args()
    if not 1 <= args.baud <= 0xFFFFFFFF or not math.isfinite(args.rate) or args.rate <= 0 or args.count < 0:
        parser.error("baud and rate must be positive; count must be nonnegative")

    channels = [1500] * 16
    channels[2] = 1000  # Conventional throttle channel.
    channels[4:] = [1000] * 12
    for channel, value in args.channel:
        channels[channel] = value
    frame = make_frame(channels)
    print("Channels (us):", ", ".join(map(str, channels)))
    print("Frame (26 bytes):", frame.hex(" "))

    sent = 0
    try:
        fd = os.open(args.tty, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        try:
            original = termios.tcgetattr(fd)
            try:
                tty.setraw(fd, termios.TCSANOW)
                settings = termios.tcgetattr(fd)
                settings[2] = (settings[2] | termios.CLOCAL | termios.CREAD) & ~termios.CRTSCTS
                termios.tcsetattr(fd, termios.TCSANOW, settings)
                set_baud(fd, args.baud)
                os.set_blocking(fd, True)
                deadline = time.monotonic()
                while args.count == 0 or sent < args.count:
                    remaining = memoryview(frame)
                    while remaining:
                        written = os.write(fd, remaining)
                        if written == 0:
                            raise OSError("zero-byte tty write")
                        remaining = remaining[written:]
                    termios.tcdrain(fd)
                    sent += 1
                    if sent == args.count:
                        break
                    deadline = max(deadline + 1 / args.rate, time.monotonic())
                    time.sleep(max(0, deadline - time.monotonic()))
            finally:
                termios.tcsetattr(fd, termios.TCSANOW, original)
        finally:
            os.close(fd)
    except KeyboardInterrupt:
        pass
    except (OSError, termios.error) as error:
        print("TTY error: %s" % error, file=sys.stderr)
        return 1
    finally:
        print("Sent %u frame(s)" % sent)
    return 0


if __name__ == "__main__":
    sys.exit(main())
