import asyncio
import websockets
import socket
import struct
import threading
import time
from http.server import SimpleHTTPRequestHandler, HTTPServer

# 手机端看到的网页界面 (手柄 UI)
HTML_CONTENT = """
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
<title>Snake Controller</title>
<style>
  body { background-color: #222; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; font-family: sans-serif; touch-action: none; overflow: hidden; }
  .grid { display: grid; grid-template-columns: 100px 100px 100px; gap: 15px; }
  .btn { width: 100px; height: 100px; background-color: #444; border: 2px solid #666; border-radius: 20px; font-size: 40px; color: white; display: flex; align-items: center; justify-content: center; user-select: none; transition: background 0.1s; box-shadow: 0 4px 8px rgba(0,0,0,0.5); }
  .btn:active { background-color: #888; transform: translateY(2px); }
  /* 为空格键单独写的长条样式 */
  .btn-space { width: 330px; height: 80px; margin-top: 15px; font-size: 30px; }
  #status { margin-bottom: 30px; font-size: 22px; color: yellow; font-weight: bold; }
</style>
</head>
<body>
<div id="status">Connecting to PC...</div>
<div class="grid">
  <div></div><div class="btn" id="btn-w">W</div><div></div>
  <div class="btn" id="btn-a">A</div><div class="btn" id="btn-s">S</div><div class="btn" id="btn-d">D</div>
</div>
<div class="btn btn-space" id="btn-space">SPACE</div>

<script>
  const status = document.getElementById('status');
  const ws = new WebSocket('ws://' + window.location.hostname + ':8765');
  
  ws.onopen = () => { status.innerText = "Ready! (Connected)"; status.style.color = "lime"; };
  ws.onclose = () => { status.innerText = "Disconnected"; status.style.color = "red"; };

  function sendKey(k) { if (ws.readyState === WebSocket.OPEN) ws.send(k); }

  // 绑定 WASD
  ['w', 'a', 's', 'd'].forEach(k => {
    const btn = document.getElementById('btn-' + k);
    btn.addEventListener('touchstart', (e) => { e.preventDefault(); sendKey(k); btn.style.backgroundColor = '#888'; });
    btn.addEventListener('touchend', (e) => { e.preventDefault(); btn.style.backgroundColor = '#444'; });
    btn.addEventListener('mousedown', (e) => { e.preventDefault(); sendKey(k); }); 
  });

  // 绑定空格键 (发送单字符 ' '，方便 Python 的 struct.pack 直接打包给 C++)
  const btnSpace = document.getElementById('btn-space');
  btnSpace.addEventListener('touchstart', (e) => { e.preventDefault(); sendKey(' '); btnSpace.style.backgroundColor = '#888'; });
  btnSpace.addEventListener('touchend', (e) => { e.preventDefault(); btnSpace.style.backgroundColor = '#444'; });
  btnSpace.addEventListener('mousedown', (e) => { e.preventDefault(); sendKey(' '); });
</script>
</body>
</html>
"""

# 全局变量
tcp_socket = None
player_id = 0

# --- 1. 启动 HTTP 服务，把网页发给手机 ---
def run_http_server():
    with open("index.html", "w", encoding="utf-8") as f:
        f.write(HTML_CONTENT)
    server = HTTPServer(('0.0.0.0', 8000), SimpleHTTPRequestHandler)
    print("[HTTP] Web Controller running at: http://<你的电脑局域网IP>:8000")
    server.serve_forever()

# --- 2. 维持与 C++ 服务端的原生 TCP 连接 ---
def connect_to_cpp_server():
    global tcp_socket, player_id
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # 不断重试，直到 C++ 服务端建房成功
    while True:
        try:
            tcp_socket.connect(('127.0.0.1', 8080))
            break
        except ConnectionRefusedError:
            print("[TCP] Waiting for C++ Game Server to open room on port 8080...")
            time.sleep(2)
            
    print("[TCP] Connected to C++ Game Server!")
    
    # 接收 C++ 服务端发来的 PlayerIDMsg (Type=3, Len=1)
    # Header 4 bytes + Body 1 byte = 5 bytes
    data = tcp_socket.recv(5)
    if len(data) == 5:
        msg_type, body_len = struct.unpack('<HH', data[:4])
        if msg_type == 3:
            player_id, = struct.unpack('<B', data[4:5])
            print(f"[TCP] Joined successfully! Assigned Player ID: {player_id}")
            
    # 开启一个死循环疯狂接收 C++ 服务端的广播，防止操作系统的 TCP 接收缓冲区塞满
    def sink():
        while True:
            try:
                tcp_socket.recv(4096)
            except:
                break
    threading.Thread(target=sink, daemon=True).start()

# --- 3. WebSocket 收到手机指令后，打包成 C++ 的 InputMsg 转发 ---
def send_to_cpp(char_input):
    if not tcp_socket: return
    # MsgHeader: type=5 (InputMsg), body_len=2 
    header = struct.pack('<HH', 5, 2)
    # InputMsg body: player_id (uint8), input_char (char)
    # 因为 JS 发送的要么是 'w','a','s','d' 要么是 ' '，长度必定为1，所以可以直接 encode
    body = struct.pack('<Bc', player_id, char_input.encode('ascii'))
    try:
        tcp_socket.sendall(header + body)
    except Exception as e:
        print("[TCP] Error sending to C++:", e)

# 【修改点】：去掉了 path 参数，解决 TypeError 报错
async def ws_handler(websocket):
    print("[WS] Phone connected to controller!")
    async for message in websocket:
        send_to_cpp(message) # 收到手机按键，立刻转给 C++

async def main():
    # 1. 开线程跑 HTTP 服务器
    threading.Thread(target=run_http_server, daemon=True).start()
    # 2. 开线程连本地的 C++ 游戏服务器
    threading.Thread(target=connect_to_cpp_server, daemon=True).start()
    
    # 3. 跑 WebSocket 服务器
    async with websockets.serve(ws_handler, "0.0.0.0", 8765):
        await asyncio.Future()  # 永远运行

if __name__ == "__main__":
    asyncio.run(main())
