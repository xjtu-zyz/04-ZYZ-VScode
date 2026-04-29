# -*- coding: utf-8 -*-
import socket
import threading
import json
import time

from protocol import (
    CMD_CONNECT,
    CMD_MOVE,
    CMD_UNDO,
    CMD_LEAVE,
    CMD_SURRENDER,
    CMD_BROADCAST,
    CMD_HEARTBEAT,
    CMD_RECONNECT,
    CMD_ERROR,
    ERR_SERVER_FULL,
    ERR_WAIT_PLAYER,
    ERR_NOT_YOUR_TURN,
    ERR_INVALID_MOVE,
    ERR_GAME_OVER,
    ERR_UNDO_DENIED,
    ERR_BAD_REQUEST,
    pack_message,
    recv_message,
)

HOST = "0.0.0.0"
PORT = 5000
BOARD_SIZE = 15
HEARTBEAT_TIMEOUT = 15

board = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
players = {}
current_turn = 1
game_over = False
move_history = []

lock = threading.Lock()


def color_name(color):
    return "黑棋" if color == 1 else "白棋"


def reset_board():
    global board, current_turn, game_over, move_history
    board = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
    current_turn = 1
    game_over = False
    move_history = []


def make_state(message=""):
    return {
        "type": "state",
        "board": board,
        "players": {
            str(color): {
                "nickname": info["nickname"],
                "color": info["color"],
                "online": info.get("online", False),
            }
            for color, info in players.items()
        },
        "current_turn": current_turn,
        "game_over": game_over,
        "message": message,
        "last_move": move_history[-1] if move_history else None,
    }


def send_json(conn, cmd, obj):
    try:
        conn.sendall(pack_message(cmd, json.dumps(obj, ensure_ascii=False)))
    except OSError:
        pass


def send_error(conn, code, message):
    send_json(conn, CMD_ERROR, {
        "type": "error",
        "code": code,
        "message": message,
    })

def mark_player_offline(color):
    info = players.get(color)
    if not info:
        return

    # 关键：已经离线就直接返回，避免重复处理
    if not info.get("online", False):
        return

    nickname = info["nickname"]

    try:
        if info.get("conn") is not None:
            info["conn"].close()
    except OSError:
        pass

    info["conn"] = None
    info["online"] = False

    print(f"[离线] {nickname} 暂时断开，棋局保留")

    online_count = sum(1 for p in players.values() if p.get("online", False))

    if online_count == 0:
        print("[重置] 双方均已断开，清空旧棋局，允许重新开局")
        reset_all_game()
        return

    broadcast_state(f"{nickname} 网络状态不佳，正在重连")

def broadcast_state(message=""):
    state = make_state(message)
    data = json.dumps(state, ensure_ascii=False)

    for color, info in list(players.items()):
        if not info.get("online", False):
            continue

        conn = info.get("conn")
        if conn is None:
            continue

        try:
            conn.sendall(pack_message(CMD_BROADCAST, data))
        except OSError:
            mark_player_offline(color)


def check_win(row, col, color):
    directions = [
        (1, 0),
        (0, 1),
        (1, 1),
        (1, -1),
    ]

    for dr, dc in directions:
        count = 1

        r, c = row + dr, col + dc
        while 0 <= r < BOARD_SIZE and 0 <= c < BOARD_SIZE and board[r][c] == color:
            count += 1
            r += dr
            c += dc

        r, c = row - dr, col - dc
        while 0 <= r < BOARD_SIZE and 0 <= c < BOARD_SIZE and board[r][c] == color:
            count += 1
            r -= dr
            c -= dc

        if count >= 5:
            return True

    return False


def board_full():
    return all(cell != 0 for row in board for cell in row)

def handle_reconnect(conn, addr, data):
    try:
        obj = json.loads(data)
        nickname = obj["nickname"]
        color = int(obj["color"])
    except Exception:
        send_error(conn, ERR_BAD_REQUEST, "重连数据格式错误")
        conn.close()
        return None

    if color not in players:
        send_error(conn, ERR_BAD_REQUEST, "无法重连：服务器没有保存你的棋局")
        conn.close()
        return None

    if players[color]["nickname"] != nickname:
        send_error(conn, ERR_BAD_REQUEST, "无法重连：昵称或身份不匹配")
        conn.close()
        return None

    players[color]["conn"] = conn
    players[color]["addr"] = addr
    players[color]["online"] = True
    players[color]["left"] = False
    players[color]["last_heartbeat"] = time.time()


    send_json(conn, CMD_BROADCAST, {
        "type": "welcome",
        "color": color,
        "message": f"重连成功，你仍然是{color_name(color)}",
    })

    broadcast_state(f"{nickname} 已重连，棋局继续")
    return color

def handle_move(color, data):
    global current_turn, game_over

    conn = players[color]["conn"]

    if game_over:
        send_error(conn, ERR_GAME_OVER, "游戏已经结束，不能继续落子")
        return

    if len(players) < 2:
        send_error(conn, ERR_WAIT_PLAYER, "等待另一名玩家加入")
        return

    if not both_players_online():
        send_error(conn, ERR_WAIT_PLAYER, "对方网络状态不佳，正在重连，请等待")
        return


    if color != current_turn:
        send_error(conn, ERR_NOT_YOUR_TURN, f"当前不是你的回合，现在轮到{color_name(current_turn)}")
        return

    try:
        row_str, col_str = data.split(",")
        row = int(row_str)
        col = int(col_str)
    except ValueError:
        send_error(conn, ERR_BAD_REQUEST, "落子数据格式错误")
        return

    if not (0 <= row < BOARD_SIZE and 0 <= col < BOARD_SIZE):
        send_error(conn, ERR_INVALID_MOVE, "落子位置越界")
        return

    if board[row][col] != 0:
        send_error(conn, ERR_INVALID_MOVE, "该位置已有棋子")
        return

    board[row][col] = color
    move_history.append({
        "row": row,
        "col": col,
        "color": color,
        "nickname": players[color]["nickname"],
    })

    if check_win(row, col, color):
        game_over = True
        broadcast_state(f"{color_name(color)}（{players[color]['nickname']}）五子连珠，获得胜利！")
        return

    if board_full():
        game_over = True
        broadcast_state("棋盘已满，本局平局！")
        return

    current_turn = 2 if current_turn == 1 else 1
    broadcast_state(f"{color_name(color)}落子成功，轮到{color_name(current_turn)}")


def handle_undo(color):
    global current_turn

    conn = players[color]["conn"]

    if game_over:
        send_error(conn, ERR_GAME_OVER, "游戏已经结束，不能悔棋")
        return

    if not move_history:
        send_error(conn, ERR_UNDO_DENIED, "当前没有可以悔棋的落子")
        return

    last = move_history[-1]

    if last["color"] != color:
        send_error(conn, ERR_UNDO_DENIED, "只能由上一回合落子的玩家悔棋")
        return

    last = move_history.pop()
    board[last["row"]][last["col"]] = 0
    current_turn = color

    broadcast_state(f"{color_name(color)}（{players[color]['nickname']}）悔棋，重新轮到{color_name(color)}")


def handle_surrender(color):
    global game_over

    if game_over:
        send_error(players[color]["conn"], ERR_GAME_OVER, "游戏已经结束")
        return

    game_over = True
    winner = 2 if color == 1 else 1

    if winner in players:
        broadcast_state(f"{color_name(color)}（{players[color]['nickname']}）认输，{color_name(winner)}（{players[winner]['nickname']}）获胜！")
    else:
        broadcast_state(f"{color_name(color)}认输，游戏结束")




def heartbeat_checker():
    while True:
        time.sleep(3)

        with lock:
            now = time.time()
            dead = []

            for color, info in list(players.items()):
                # 关键：已经离线的玩家不要再检测心跳
                if not info.get("online", False):
                    continue

                if now - info.get("last_heartbeat", now) > HEARTBEAT_TIMEOUT:
                    dead.append(color)

            for color in dead:
                if color in players and players[color].get("online", False):
                    print(f"[心跳超时] {players[color]['nickname']}")
                    mark_player_offline(color)


def handle_client(conn, addr):
    global current_turn, game_over

    color = None

    try:
        cmd, data = recv_message(conn)

        if cmd == CMD_RECONNECT:
            with lock:
                color = handle_reconnect(conn, addr, data)
            if color is None:
                return

        elif cmd == CMD_CONNECT:
            if not data.strip():
                send_error(conn, ERR_BAD_REQUEST, "连接失败：请先发送昵称")
                conn.close()
                return

            nickname = data.strip()

            with lock:
                if game_over:
                    reset_all_game()

                if players and all(not p.get("online", False) for p in players.values()):
                    reset_all_game()
                if len(players) >= 2:
                    send_error(conn, ERR_SERVER_FULL, "服务器已有棋局，请等待结束或使用重连")
                    conn.close()
                    return

                color = 1 if 1 not in players else 2

                players[color] = {
                    "conn": conn,
                    "addr": addr,
                    "nickname": nickname,
                    "color": color,
                    "online": True,
                    "left": False,
                    "last_heartbeat": time.time(),
                }

                send_json(conn, CMD_BROADCAST, {
                    "type": "welcome",
                    "color": color,
                    "message": f"连接成功，你是{color_name(color)}",
                })

                if len(players) == 1:
                    broadcast_state(f"{nickname} 加入游戏，等待另一名玩家")
                else:
                    reset_board()
                    broadcast_state("两名玩家已就绪，新局开始，黑棋先手")

        else:
            send_error(conn, ERR_BAD_REQUEST, "请先连接或重连")
            conn.close()
            return
        while True:
            cmd, data = recv_message(conn)

            if cmd is None:
                break

            with lock:
                if color not in players:
                    break

                players[color]["last_heartbeat"] = time.time()

                if cmd == CMD_MOVE:
                    handle_move(color, data)

                elif cmd == CMD_UNDO:
                    handle_undo(color)

                elif cmd == CMD_SURRENDER:
                    handle_surrender(color)

                elif cmd == CMD_HEARTBEAT:
                    send_json(conn, CMD_HEARTBEAT, {
                        "type": "heartbeat_ack",
                        "time": time.time(),
                    })
                elif cmd == CMD_LEAVE:
                    handle_leave(color)
                    break

                else:
                    send_error(conn, ERR_BAD_REQUEST, "未知指令")

    except Exception as e:
        print(f"[异常] 客户端 {addr} 出错：{e}")

    finally:
        with lock:
            if color in players:
                info = players[color]

                if info.get("left", False):
                    return

                if info.get("online", False):
                    print(f"[断开] {info['nickname']} {addr}")
                    mark_player_offline(color)

def reset_all_game():
    global board, players, current_turn, game_over, move_history

    board = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
    players = {}
    current_turn = 1
    game_over = False
    move_history = []

def both_players_online():
    return (
        1 in players and
        2 in players and
        players[1].get("online", False) and
        players[2].get("online", False)
    )

def handle_leave(color):
    info = players.get(color)
    if not info:
        return

    nickname = info["nickname"]

    try:
        if info.get("conn") is not None:
            info["conn"].close()
    except OSError:
        pass

    info["conn"] = None
    info["online"] = False
    info["left"] = True

    print(f"[离开] {nickname} 主动返回重开")

    if players and all(p.get("left", False) for p in players.values()):
        print("[重置] 双方主动离开，清空棋局，允许重新开局")
        reset_all_game()
    else:
        broadcast_state(f"{nickname} 已离开本局，等待对方返回重开")

def start_server():
    reset_all_game()

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(5)

    print(f"[启动] 五子棋服务器已启动：{HOST}:{PORT}")

    threading.Thread(target=heartbeat_checker, daemon=True).start()

    try:
        while True:
            conn, addr = server.accept()
            print(f"[连接] {addr}")

            t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
            t.start()

    except KeyboardInterrupt:
        print("\n[关闭] 服务器关闭")

    finally:
        server.close()


if __name__ == "__main__":
    start_server()