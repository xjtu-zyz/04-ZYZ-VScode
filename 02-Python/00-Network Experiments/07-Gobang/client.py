# -*- coding: utf-8 -*-
import socket
import threading
import json
import time
import tkinter as tk
from tkinter import messagebox

from protocol import (
    CMD_CONNECT,
    CMD_MOVE,
    CMD_UNDO,
    CMD_SURRENDER,
    CMD_BROADCAST,
    CMD_HEARTBEAT,
    CMD_RECONNECT,
    CMD_LEAVE,
    CMD_ERROR,
    pack_message,
    recv_message,
)

BOARD_SIZE = 15
CELL_SIZE = 28
MARGIN = 24
PIECE_RADIUS = 11
CANVAS_SIZE = MARGIN * 2 + CELL_SIZE * (BOARD_SIZE - 1)


class GobangClient:
    def __init__(self, root):
        self.root = root
        self.root.title("联机五子棋客户端")

        self.sock = None
        self.connected = False
        self.running = True
        self.manual_disconnect = False
        self.server_ip = ""
        self.server_port = 0
        self.nickname = ""
        self.my_color = 0
        self.board = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
        self.current_turn = 1
        self.game_over = False
        self.players = {}
        self.last_move = None

        self.click_locked = False
        self.pending_click = None

        self.connect_frame = None
        self.game_frame = None
        self.canvas = None
        self.info_label = None
        self.color_label = None
        self.players_label = None
        self.reconnect_button = None

        self.build_connect_ui()

    def build_connect_ui(self):
        if self.game_frame:
            self.game_frame.destroy()

        self.connect_frame = tk.Frame(self.root, padx=20, pady=20)
        self.connect_frame.pack()

        tk.Label(self.connect_frame, text="服务器 IP：").grid(row=0, column=0, sticky="e")
        self.ip_entry = tk.Entry(self.connect_frame, width=26)
        self.ip_entry.insert(0, "47.94.243.152")
        self.ip_entry.grid(row=0, column=1, pady=5)

        tk.Label(self.connect_frame, text="端口：").grid(row=1, column=0, sticky="e")
        self.port_entry = tk.Entry(self.connect_frame, width=26)
        self.port_entry.insert(0, "5000")
        self.port_entry.grid(row=1, column=1, pady=5)

        tk.Label(self.connect_frame, text="昵称：").grid(row=2, column=0, sticky="e")
        self.name_entry = tk.Entry(self.connect_frame, width=26)
        self.name_entry.insert(0, "玩家")
        self.name_entry.grid(row=2, column=1, pady=5)

        self.connect_button = tk.Button(
            self.connect_frame,
            text="连接服务器",
            command=self.connect_server,
            width=20,
        )
        self.connect_button.grid(row=3, column=0, columnspan=2, pady=12)

    def build_game_ui(self):
        if self.connect_frame:
            self.connect_frame.destroy()

        self.game_frame = tk.Frame(self.root, padx=10, pady=10)
        self.game_frame.pack()

        self.color_label = tk.Label(self.game_frame, text="我的棋子：未知", font=("Microsoft YaHei", 11))
        self.color_label.pack()

        self.players_label = tk.Label(self.game_frame, text="玩家信息：等待同步", font=("Microsoft YaHei", 11))
        self.players_label.pack()

        self.info_label = tk.Label(self.game_frame, text="等待服务器消息", font=("Microsoft YaHei", 12))
        self.info_label.pack(pady=2)

        self.canvas = tk.Canvas(
            self.game_frame,
            width=CANVAS_SIZE,
            height=CANVAS_SIZE,
            bg="#DDB879",
            highlightthickness=1,
            highlightbackground="#6B3F1D",
        )
        self.canvas.pack()

        self.canvas.bind("<Button-1>", self.on_canvas_click)

        button_frame = tk.Frame(self.game_frame)
        button_frame.pack(pady=4)

        tk.Button(button_frame, text="悔棋", width=10, command=self.request_undo).grid(row=0, column=0, padx=5)
        tk.Button(button_frame, text="认输", width=10, command=self.request_surrender).grid(row=0, column=1, padx=5)

        self.reconnect_button = tk.Button(
            button_frame,
            text="断开重连",
            width=10,
            command=self.reconnect_game,
            state="normal",
        )
        tk.Button(
            button_frame,
            text="返回重开",
            width=10,
            command=self.back_to_connect,
        ).grid(row=0, column=3, padx=5)
        self.reconnect_button.grid(row=0, column=2, padx=5)

        self.draw_board()

    def reconnect_game(self):
        if self.connected:
            messagebox.showinfo("提示", "当前连接正常，无需重连")
            return

        if not self.server_ip or not self.server_port or not self.nickname or self.my_color == 0:
            messagebox.showwarning("提示", "旧棋局信息不存在，请重新连接开局")
            self.back_to_connect()
            return

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5)
            self.sock.connect((self.server_ip, self.server_port))
            self.sock.settimeout(None)

            data = json.dumps({
                "nickname": self.nickname,
                "color": self.my_color,
            }, ensure_ascii=False)

            self.sock.sendall(pack_message(CMD_RECONNECT, data))

            cmd, resp = recv_message(self.sock)

            if cmd == CMD_ERROR:
                try:
                    obj = json.loads(resp)
                    msg = obj.get("message", "重连失败")
                except Exception:
                    msg = "重连失败"

                messagebox.showwarning("重连失败", msg + "\n请重新连接开局")
                self.close_socket_only()
                self.back_to_connect()
                return

            if cmd == CMD_BROADCAST:
                try:
                    obj = json.loads(resp)
                except Exception:
                    messagebox.showwarning("重连失败", "服务器响应异常，请重新连接开局")
                    self.close_socket_only()
                    self.back_to_connect()
                    return

                self.connected = True
                self.click_locked = False

                threading.Thread(target=self.recv_loop, daemon=True).start()
                threading.Thread(target=self.heartbeat_loop, daemon=True).start()

                self.handle_broadcast(obj)
                self.info_label.config(text="重连成功，棋局已恢复")
                return

            messagebox.showwarning("重连失败", "服务器响应异常，请重新连接开局")
            self.close_socket_only()
            self.back_to_connect()

        except OSError as e:
            messagebox.showerror("重连失败", f"无法连接服务器：{e}")
            self.close_socket_only()
            self.back_to_connect()

    def connect_server(self):
        ip = self.ip_entry.get().strip()
        port_text = self.port_entry.get().strip()
        nickname = self.name_entry.get().strip()

        if not ip or not port_text or not nickname:
            messagebox.showerror("错误", "请填写服务器 IP、端口和昵称")
            return

        try:
            port = int(port_text)
        except ValueError:
            messagebox.showerror("错误", "端口必须是数字")
            return

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5)
            self.sock.connect((ip, port))
            self.sock.settimeout(None)

            self.sock.sendall(pack_message(CMD_CONNECT, nickname))

        except OSError as e:
            messagebox.showerror("连接失败", f"无法连接服务器：{e}")
            return

        self.connected = True
        self.running = True
        self.manual_disconnect = False
        self.my_color = 0
        self.server_ip = ip
        self.server_port = port
        self.nickname = nickname
        self.board = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
        self.current_turn = 1
        self.game_over = False
        self.players = {}
        self.last_move = None
        self.click_locked = False
        self.pending_click = None

        self.build_game_ui()

        threading.Thread(target=self.recv_loop, daemon=True).start()
        threading.Thread(target=self.heartbeat_loop, daemon=True).start()

    def recv_loop(self):
        while self.connected:
            cmd, data = recv_message(self.sock)

            if cmd is None:
                self.connected = False

                if not self.manual_disconnect:
                    self.root.after(0, self.on_disconnected)

                break

            try:
                obj = json.loads(data) if data else {}
            except json.JSONDecodeError:
                continue

            if cmd == CMD_BROADCAST:
                self.root.after(0, self.handle_broadcast, obj)
            elif cmd == CMD_ERROR:
                self.root.after(0, self.handle_error, obj)
            elif cmd == CMD_HEARTBEAT:
                pass

    def heartbeat_loop(self):
        while self.connected:
            time.sleep(5)
            try:
                self.sock.sendall(pack_message(CMD_HEARTBEAT, "ping"))
            except OSError:
                self.connected = False

                if not self.manual_disconnect:
                    self.root.after(0, self.on_disconnected)
                break

    def handle_broadcast(self, obj):
        msg_type = obj.get("type")

        if msg_type == "welcome":
            self.my_color = obj.get("color", 0)
            self.color_label.config(text=f"我的棋子：{self.color_text(self.my_color)}")
            self.info_label.config(text=obj.get("message", "连接成功"))
            return

        if msg_type == "state":
            old_board = self.board

            self.board = obj.get("board", self.board)
            self.players = obj.get("players", {})
            self.current_turn = obj.get("current_turn", 1)
            self.game_over = obj.get("game_over", False)
            self.last_move = obj.get("last_move")

            message = obj.get("message", "")
            self.info_label.config(text=message)

            self.update_players_label()
            self.draw_board()
            self.animate_last_move(old_board)

            self.click_locked = False
            self.pending_click = None
            if self.game_over and message:
                messagebox.showinfo("游戏结束", message)

    def handle_error(self, obj):
        self.click_locked = False
        code = obj.get("code", "未知")
        message = obj.get("message", "未知错误")
        messagebox.showwarning("提示", f"{message}\n错误码：{code}")

    def update_players_label(self):
        black = self.players.get("1", {}).get("nickname", "等待加入")
        white = self.players.get("2", {}).get("nickname", "等待加入")
        self.players_label.config(text=f"黑棋：{black}    白棋：{white}")

    def request_undo(self):
        if not self.check_can_send():
            return

        try:
            self.sock.sendall(pack_message(CMD_UNDO, ""))
        except OSError:
            self.connected = False
            self.on_disconnected()

    def request_surrender(self):
        if not self.check_can_send():
            return

        if not messagebox.askyesno("确认认输", "确定要认输吗？"):
            return

        try:
            self.sock.sendall(pack_message(CMD_SURRENDER, ""))
        except OSError:
            self.connected = False
            self.on_disconnected()

    def check_can_send(self):
        if not self.connected:
            messagebox.showwarning("提示", "尚未连接服务器")
            return False

        if self.game_over:
            messagebox.showwarning("提示", "游戏已经结束")
            return False

        return True

    def on_canvas_click(self, event):
        if not self.check_can_send():
            return

        if self.click_locked:
            return

        if self.my_color != self.current_turn:
            messagebox.showwarning("提示", "当前不是你的回合")
            return

        row, col = self.pos_to_grid(event.x, event.y)

        if row is None or col is None:
            return

        if self.board[row][col] != 0:
            messagebox.showwarning("提示", "该位置已有棋子")
            self.pending_click = None
            return

        current_click = (row, col)

        if self.pending_click != current_click:
            self.pending_click = current_click
            self.info_label.config(
                text=f"已选择位置：第 {row + 1} 行，第 {col + 1} 列；再次点击同一位置确认落子"
            )
            self.draw_board()
            self.draw_pending_marker(row, col)
            return

        self.pending_click = None
        self.click_locked = True

        try:
            self.sock.sendall(pack_message(CMD_MOVE, f"{row},{col}"))
        except OSError:
            self.click_locked = False
            self.connected = False
            self.on_disconnected()

    def pos_to_grid(self, x, y):
        col = round((x - MARGIN) / CELL_SIZE)
        row = round((y - MARGIN) / CELL_SIZE)

        if not (0 <= row < BOARD_SIZE and 0 <= col < BOARD_SIZE):
            return None, None

        grid_x = MARGIN + col * CELL_SIZE
        grid_y = MARGIN + row * CELL_SIZE

        if abs(x - grid_x) > CELL_SIZE // 2 or abs(y - grid_y) > CELL_SIZE // 2:
            return None, None

        return row, col

    def draw_board(self):
        self.canvas.delete("all")

        for i in range(BOARD_SIZE):
            x1 = MARGIN
            y1 = MARGIN + i * CELL_SIZE
            x2 = MARGIN + (BOARD_SIZE - 1) * CELL_SIZE
            y2 = y1
            self.canvas.create_line(x1, y1, x2, y2, fill="#4A2A12", width=1)

            x1 = MARGIN + i * CELL_SIZE
            y1 = MARGIN
            x2 = x1
            y2 = MARGIN + (BOARD_SIZE - 1) * CELL_SIZE
            self.canvas.create_line(x1, y1, x2, y2, fill="#4A2A12", width=1)

        star_points = [(3, 3), (3, 11), (7, 7), (11, 3), (11, 11)]
        for row, col in star_points:
            x = MARGIN + col * CELL_SIZE
            y = MARGIN + row * CELL_SIZE
            self.canvas.create_oval(x - 4, y - 4, x + 4, y + 4, fill="#4A2A12", outline="#4A2A12")

        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                piece = self.board[row][col]
                if piece != 0:
                    self.draw_piece(row, col, piece)

    def draw_piece(self, row, col, color):
        x = MARGIN + col * CELL_SIZE
        y = MARGIN + row * CELL_SIZE

        fill = "black" if color == 1 else "white"

        self.canvas.create_oval(
            x - PIECE_RADIUS,
            y - PIECE_RADIUS,
            x + PIECE_RADIUS,
            y + PIECE_RADIUS,
            fill=fill,
            outline="#222222",
            width=2,
        )

    def draw_pending_marker(self, row, col):
        x = MARGIN + col * CELL_SIZE
        y = MARGIN + row * CELL_SIZE

        self.canvas.create_rectangle(
            x - PIECE_RADIUS,
            y - PIECE_RADIUS,
            x + PIECE_RADIUS,
            y + PIECE_RADIUS,
            outline="red",
            width=2,
            tags="pending",
        )

    def animate_last_move(self, old_board):
        if not self.last_move:
            return

        row = self.last_move.get("row")
        col = self.last_move.get("col")
        color = self.last_move.get("color")

        if row is None or col is None:
            return

        if old_board[row][col] == self.board[row][col]:
            return

        x = MARGIN + col * CELL_SIZE
        y = MARGIN + row * CELL_SIZE

        def blink(count):
            if count <= 0:
                self.draw_board()
                return

            self.canvas.create_oval(
                x - PIECE_RADIUS - 3,
                y - PIECE_RADIUS - 3,
                x + PIECE_RADIUS + 3,
                y + PIECE_RADIUS + 3,
                outline="red",
                width=3,
                tags="blink",
            )
            self.root.after(120, lambda: clear_blink(count))

        def clear_blink(count):
            self.canvas.delete("blink")
            self.root.after(120, lambda: blink(count - 1))

        blink(2)

    def color_text(self, color):
        if color == 1:
            return "黑棋"
        if color == 2:
            return "白棋"
        return "未知"

    def on_disconnected(self):
        if self.manual_disconnect:
            return

        self.connected = False
        self.click_locked = False

        try:
            if self.info_label and self.info_label.winfo_exists():
                self.info_label.config(
                    text="网络连接已断开：若对方仍在线可断线重连；若双方均离线请返回重开"
                )
        except tk.TclError:
            return

        messagebox.showwarning(
            "网络断开",
            "网络连接已断开。\n\n"
            "如果只是你一方掉线，请点击“断线重连”恢复棋局。\n"
            "如果双方都已离线，请点击“返回重开”重新连接。"
        )

    def back_to_connect(self):
        # 主动返回重开，不应触发“网络断开”提示
        self.manual_disconnect = True

        if self.connected and self.sock:
            try:
                self.sock.sendall(pack_message(CMD_LEAVE, ""))
            except OSError:
                pass

        self.connected = False
        self.running = False
        self.close_socket_only()

        self.my_color = 0
        self.board = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
        self.current_turn = 1
        self.game_over = False
        self.players = {}
        self.last_move = None
        self.click_locked = False
        self.pending_click = None

        self.server_ip = ""
        self.server_port = 0
        self.nickname = ""

        if self.game_frame:
            try:
                self.game_frame.destroy()
            except tk.TclError:
                pass
            self.game_frame = None

        if self.connect_frame:
            try:
                self.connect_frame.destroy()
            except tk.TclError:
                pass
            self.connect_frame = None

        self.info_label = None
        self.color_label = None
        self.players_label = None
        self.canvas = None
        self.reconnect_button = None

        self.build_connect_ui()

    def close_socket_only(self):
        try:
            if self.sock:
                self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

        try:
            if self.sock:
                self.sock.close()
        except OSError:
            pass

        self.sock = None

    def close(self):
        self.connected = False
        self.running = False
        self.close_socket_only()
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = GobangClient(root)
    root.protocol("WM_DELETE_WINDOW", app.close)
    root.mainloop()