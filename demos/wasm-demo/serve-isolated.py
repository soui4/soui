#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""serve-isolated.py -- 给 wasm 的 worker 后端（-DSOUI_WASM_PTHREADS=ON）用的本地静态服务。

为什么不能直接用 `python3 -m http.server`：`-pthread` 的产物把线性内存放在
SharedArrayBuffer 里，而浏览器只在页面是 crossOriginIsolated 时才允许创建
SharedArrayBuffer。隔离状态由两个响应头决定：

    Cross-Origin-Opener-Policy: same-origin
    Cross-Origin-Embedder-Policy: require-corp

两者齐了 `window.crossOriginIsolated` 才是 true，worker 线程才建得起来；缺一个的话
Emscripten 报 `SharedArrayBuffer is not defined`，本模块会退回泵后端（任务照跑，只是
仍在 UI 线程上）—— 也就是说表现是"能用但不并行了"，不会报错，很容易看漏。所以在这
个服务里起页面后，先确认控制台里 crossOriginIsolated 为 true。

用法（在仓库根或任意目录都行，参数是被服务的目录）：

    python3 demos/wasm-demo/serve-isolated.py demos/wasm-demo
    # 浏览器打开 http://localhost:8080/soui.html

同源的 .js/.wasm/.data 在 require-corp 下不需要额外属性；如果页面要从别处拉资源，
那条资源得自带 CORP 头，否则会被拦（这跟本服务无关，只是排查时容易混淆）。

对照用：--no-isolation 故意不发这两个头，用来验证"没隔离时退回泵后端"这条降级路径。

--ws-proxy HOST:PORT：把带 `Upgrade: websocket` 的请求原样隧道到上游。cnchess 的 wasm
页面默认按同源算服务器地址（`ws://location.host`，线上由 Apache/nginx 在同 origin 反代
`/cnchess/` 到同机的 cnchess_server），本机用这个脚本起页面时没有那层反代，默认地址会打到
静态服务自己身上 —— 表现是点了登录什么也不下载。加这条参数后本地链接与线上形态一致：

    python3 demos/wasm-demo/serve-isolated.py games/cnchess/client/wasm/out_pt --ws-proxy 127.0.0.1:3311
"""

import argparse
import functools
import http.server
import os
import select
import socket
import socketserver


class IsolatedHandler(http.server.SimpleHTTPRequestHandler):
    # 常见扩展名的 MIME；缺了 application/wasm 时浏览器会按 text/plain 编流，
    # 大模块直接放弃编译（Chrome 报 "Refused to apply mime type"）。
    extensions_map = {
        **http.server.SimpleHTTPRequestHandler.extensions_map,
        '.wasm': 'application/wasm',
        '.data': 'application/octet-stream',
        '.js': 'text/javascript',
        '.mjs': 'text/javascript',
        '.html': 'text/html',
        '.idx': 'application/octet-stream',
        '.xml': 'text/xml',
        '.ttf': 'font/ttf',
    }

    def __init__(self, *args, isolated=True, ws_proxy=None, **kwargs):
        self._isolated = isolated
        self._ws_proxy = ws_proxy
        super().__init__(*args, **kwargs)

    def do_GET(self):
        # 浏览器只在协商成功（101）时才把这条连接当 WebSocket，所以隧道里必须把
        # 握手原样透传，不做任何改写：路径、Origin、Sec-WebSocket-* 全部照发。
        if (self.headers.get('Upgrade') or '').lower() == 'websocket':
            self.tunnel_websocket()
            return
        super().do_GET()

    def tunnel_websocket(self):
        if not self._ws_proxy:
            # 明确报错比让浏览器拿到 400 再猜好：这条路径的失败表现是"点了登录没反应"。
            self.send_error(501, 'websocket upgrade needs --ws-proxy HOST:PORT')
            return
        head = ('%s %s %s\r\n' % (self.command, self.path, self.request_version)).encode('latin-1')
        for key, val in self.headers.items():
            head += ('%s: %s\r\n' % (key, val)).encode('latin-1')
        head += b'\r\n'
        # 这里不碰 self.rfile：它的缓冲已经空了，任何 read/peek 都会一直等到有字节为止，
        # 而客户端在收到 101 之前不会发第一个帧（RFC 6455 §1.3），所以确实没有要转发的
        # 残留。表现是一旦去读它就再也没回过响应，握手看起来"卡住"。
        try:
            up = socket.create_connection(self._ws_proxy, timeout=5)
        except OSError as exc:
            self.send_error(502, 'ws upstream %s:%d unreachable: %s'
                            % (self._ws_proxy[0], self._ws_proxy[1], exc))
            return

        # 从这一行起这条连接就归隧道了：先表态，握手结束后 handler 不会再拿 rfile
        # 去解析下一个请求。
        self.close_connection = True
        try:
            up.sendall(head)
            resp = b''
            while b'\r\n\r\n' not in resp and len(resp) < 65536:
                chunk = up.recv(4096)
                if not chunk:
                    break
                resp += chunk
            first_line = resp.split(b'\r\n', 1)[0].decode('latin-1', 'replace')
            if not first_line.startswith('HTTP/1.1 101') and not first_line.startswith('HTTP/1.0 101'):
                # 上游没同意升级（路径/子协议不对，或它根本不是 ws 服务器）：把它的
                # 响应头原样回给浏览器，然后各自收摊。
                self.send_response(502)
                self.send_header('Content-Type', 'text/plain')
                self.end_headers()
                self.wfile.write(('upstream replied: %s' % first_line).encode('utf-8'))
                return
            self.connection.sendall(resp)
            head_len = resp.find(b'\r\n\r\n')
            self.relay(self.connection, up, resp[head_len + 4:] if head_len >= 0 else b'')
        except OSError:
            pass
        finally:
            try:
                up.close()
            except OSError:
                pass

    @staticmethod
    def relay(client, upstream, prelude=b''):
        """两条 socket 之间双向搬到任一侧关闭；prelude 是已经从上游读走、还没转发的字节。"""
        socks = (client, upstream)
        buffers = {id(upstream): prelude}
        while True:
            try:
                readable, _, _ = select.select(socks, [], [], 300)
            except OSError:
                break
            if not readable:
                break
            for src in readable:
                peer = upstream if src is client else client
                try:
                    data = buffers.pop(id(src), b'') + src.recv(65536)
                except OSError:
                    data = b''
                if not data:
                    return
                try:
                    peer.sendall(data)
                except OSError:
                    return

    def end_headers(self):
        if self._isolated:
            self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
            self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        # 开发期产物换得快，缓存下来会拿到旧胶水（表现是缺少 import 的 LinkError）。
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def log_message(self, fmt, *args):
        # 只打异常与非 2xx：加载一次模块就有几十条 200，刷屏看不清问题。
        code = args[1] if len(args) > 1 else ''
        if isinstance(code, int) and code >= 400:
            super().log_message(fmt, *args)


def parse_host_port(text):
    host, _, port = text.rpartition(':')
    if not host or not port.isdigit():
        raise argparse.ArgumentTypeError('--ws-proxy 要 HOST:PORT 形式，收到 %r' % text)
    return host, int(port)


def main():
    ap = argparse.ArgumentParser(description='带 COOP/COEP 的静态服务（wasm pthreads 需要）')
    ap.add_argument('directory', nargs='?', default=os.path.dirname(os.path.abspath(__file__)),
                    help='要服务的目录，默认本脚本所在目录')
    ap.add_argument('--port', type=int, default=8080)
    ap.add_argument('--no-isolation', action='store_true',
                    help='不发 COOP/COEP，用来验证退回泵后端的降级路径')
    ap.add_argument('--ws-proxy', type=parse_host_port, metavar='HOST:PORT',
                    help='把 WebSocket 升级请求隧道到这个上游（cnchess 页面默认按同源算地址，'
                         '本机没有线上那层反代时用它顶上）')
    args = ap.parse_args()

    handler = functools.partial(IsolatedHandler,
                                directory=os.path.abspath(args.directory),
                                isolated=not args.no_isolation,
                                ws_proxy=args.ws_proxy)
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    # 一条活着的 WebSocket 会占住整个 handler 到断开为止，所以必须每连接一条线程；
    # daemon_threads 让 Ctrl-C 不用等在跑的隧道上。
    with socketserver.ThreadingTCPServer(('', args.port), handler) as httpd:
        httpd.daemon_threads = True
        state = '关闭（--no-isolation）' if args.no_isolation else '开启'
        print('服务 %s' % os.path.abspath(args.directory))
        print('http://localhost:%d/soui.html   crossOriginIsolated 头：%s' % (args.port, state))
        if args.no_isolation:
            print('  注意：这个模式下浏览器不会给出 SharedArrayBuffer，wasm 构建会退回泵后端。')
        if args.ws_proxy:
            print('  WebSocket 反代：%s（页面里同源地址可直接用）' % ' '.join(
                (args.ws_proxy[0], str(args.ws_proxy[1]))))
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == '__main__':
    main()
