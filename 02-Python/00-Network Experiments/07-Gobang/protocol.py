# -*- coding: utf-8 -*-
import struct

MAGIC = b"\x58\x51"
HEADER_LEN = 7

CMD_CONNECT = 0x01
CMD_MOVE = 0x02
CMD_UNDO = 0x03
CMD_SURRENDER = 0x04
CMD_BROADCAST = 0x05
CMD_HEARTBEAT = 0x06
CMD_RECONNECT = 0x07
CMD_LEAVE = 0x08
CMD_ERROR = 0x09


ERR_SERVER_FULL = 1001
ERR_WAIT_PLAYER = 1002
ERR_NOT_YOUR_TURN = 1003
ERR_INVALID_MOVE = 1004
ERR_GAME_OVER = 1005
ERR_UNDO_DENIED = 1006
ERR_BAD_REQUEST = 1007


def pack_message(cmd: int, data: str = "") -> bytes:
    body = data.encode("utf-8")
    header = MAGIC + bytes([cmd]) + struct.pack(">I", len(body))
    return header + body


def recv_all(sock, n: int):
    data = b""
    while len(data) < n:
        try:
            chunk = sock.recv(n - len(data))
        except OSError:
            return None
        if not chunk:
            return None
        data += chunk
    return data


def recv_message(sock):
    header = recv_all(sock, HEADER_LEN)
    if header is None:
        return None, None

    if header[:2] != MAGIC:
        return None, None

    cmd = header[2]
    length = struct.unpack(">I", header[3:7])[0]

    body = recv_all(sock, length) if length > 0 else b""
    if body is None:
        return None, None

    return cmd, body.decode("utf-8")