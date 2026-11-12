#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""serve-fundholdings.py -- 基金持仓 wasm 版的独立静态服务 + /__soui_http 转发。

本文件不依赖仓库里的 tools/wasm/serve-isolated.py，和 5 个 wasm 产物放在同一
目录就能独立部署（构建时由 CMake 自动复制到 out_pt / release_pt，Windows 下
可直接运行同目录的 start-server.bat）。

用法（directory 缺省是当前工作目录）：

    # 本机开发，浏览器直接开 http://localhost:8080/fundholdings.html
    python serve-fundholdings.py . --http-proxy

    # 放在 Apache 后面：只监听 loopback，由 Apache 反代 /__soui_http
    python serve-fundholdings.py . --bind 127.0.0.1 --port 18080 ^
        --http-proxy --allow-host eastmoney.com

为什么不能直接用 `python -m http.server`：

1. `-pthread` 产物把线性内存放在 SharedArrayBuffer 里，浏览器只在页面
   crossOriginIsolated 时才允许创建；隔离状态由两个响应头决定：
       Cross-Origin-Opener-Policy: same-origin
       Cross-Origin-Embedder-Policy: require-corp
   缺一个时 worker 线程建不起来，TaskLoop 静默退回泵后端（不报错，只是不并行）。
   起页面后先确认控制台里 crossOriginIsolated 为 true。
2. 东财接口要求 Cookie/Referer，浏览器 XHR 不允许脚本设置这两个头，也不允许读
   Set-Cookie；客户端把请求改写成同源 /__soui_http?url=<编码后的绝对URL>，
   本服务还原成真正的上游请求。X-Soui-Cookie / X-Soui-Referer / X-Soui-User-Agent
   / X-Soui-Accept-Language 是那些禁止头的替身，Set-Cookie 汇总后放
   X-Soui-Cookie 响应头回给页面。同源路径同时绕开 CORS。

安全提示：这个端点本质是代理，默认不限制上游域名，只适合本机开发；部署到
服务器时用 --allow-host 限定上游域名（可重复匹配子域），并用 --bind 127.0.0.1
只允许 Apache 等本机反代访问。完整 Apache 配置见同目录 apache/fundholdings.conf。
"""

import argparse
import functools
import http.cookiejar
import http.server
import os
import socketserver
import urllib.error
import urllib.parse
import urllib.request

# 取数依赖 /__soui_http 的落地页。按文件名识别，仅供"漏了 --http-proxy"的
# 启动期提醒，不是转发逻辑的一部分；新增同类页面时登记到这里即可。
HTTP_PROXY_PAGES = frozenset(('fundholdings.html',))


def needs_http_proxy(directory):
    try:
        return any(name.lower() in HTTP_PROXY_PAGES for name in os.listdir(directory))
    except OSError:
        return False


def guess_page(directory):
    """启动提示里的落地页：优先 fundholdings.html，否则取目录里最靠前的 .html。"""
    if os.path.isfile(os.path.join(directory, 'fundholdings.html')):
        return 'fundholdings.html'
    try:
        pages = sorted(f for f in os.listdir(directory)
                       if f.lower().endswith(('.html', '.htm')))
    except OSError:
        pages = []
    return pages[0] if pages else 'fundholdings.html'


class FundHoldingsHandler(http.server.SimpleHTTPRequestHandler):
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

    def __init__(self, *args, isolated=True, options=None, **kwargs):
        self._isolated = isolated
        self._options = options
        super().__init__(*args, **kwargs)

    # ------------------------------------------------------------- 路由
    def do_GET(self):
        if self.handle_custom_request():
            return
        super().do_GET()

    def do_POST(self):
        if self.handle_custom_request():
            return
        # 先把请求体读干净再回 501：带未读数据就关连接，Windows 会发 RST，
        # 客户端连状态码都收不到（表现为 ConnectionAbortedError）。
        length = int(self.headers.get('Content-Length') or 0)
        if length > 0:
            self.rfile.read(length)
        self.send_error(501, 'POST is not supported by this server')

    def handle_custom_request(self):
        """命中 /__soui_http 就自行应答并返回 True，否则交回静态文件处理。"""
        # 必须在静态文件之前判断：这个路径不对应磁盘文件。
        if not self.path.startswith('/__soui_http'):
            return False
        self.forward_http()
        return True

    # ----------------------------------------------------- 同源 HTTP 转发
    @staticmethod
    def _default_user_agent():
        return ('Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 '
                '(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36')

    def forward_http(self):
        """把 /__soui_http?url=... 转发到上游，返回上游状态码、正文与 Cookie。

        X-Soui-* 是浏览器禁止头（Cookie/Referer/UA/Accept-Language）的替身；
        响应侧用 X-Soui-Cookie 把 Set-Cookie 汇总后带回，因为脚本读不到
        Set-Cookie（XHR 的 getResponseHeader 对它返回 null）。
        """
        if not self._options.http_proxy:
            self.send_error(501, 'http proxy needs --http-proxy')
            return

        parsed = urllib.parse.urlparse(self.path)
        params = urllib.parse.parse_qs(parsed.query)
        url = (params.get('url') or [''])[0]
        if not url.lower().startswith(('http://', 'https://')):
            self.send_error(400, 'url must be an absolute http(s) URL')
            return

        allowed = [h.strip().lower().lstrip('.')
                   for h in (self._options.allow_host or []) if h.strip()]
        if allowed:
            host = (urllib.parse.urlparse(url).hostname or '').lower()
            if not any(host == h or host.endswith('.' + h) for h in allowed):
                self.send_error(403, 'upstream host %s is not in --allow-host' % host)
                return

        headers = {'User-Agent': self.headers.get('X-Soui-User-Agent') or self._default_user_agent()}
        for src, dst in (('X-Soui-Referer', 'Referer'),
                         ('X-Soui-Accept-Language', 'Accept-Language'),
                         ('X-Soui-Cookie', 'Cookie'),
                         ('Accept', 'Accept'),
                         ('Content-Type', 'Content-Type'),
                         ('X-Requested-With', 'X-Requested-With')):
            val = self.headers.get(src)
            if val:
                headers[dst] = val

        data = None
        if self.command == 'POST':
            length = int(self.headers.get('Content-Length') or 0)
            data = self.rfile.read(length) if length > 0 else b''
            if 'Content-Type' not in headers:
                headers['Content-Type'] = 'application/x-www-form-urlencoded'

        # 每个请求一份 CookieJar：HTTPCookieProcessor 会在重定向链的每一跳上收集
        # Set-Cookie，汇总后正好是 GetCookie 需要的 "a=b; c=d" 形式。
        jar = http.cookiejar.CookieJar()
        opener = urllib.request.build_opener(
            urllib.request.HTTPCookieProcessor(jar),
            urllib.request.HTTPSHandler())
        req = urllib.request.Request(url, data=data, headers=headers)
        try:
            with opener.open(req, timeout=15) as resp:
                status = resp.getcode()
                content_type = resp.headers.get('Content-Type') or 'application/octet-stream'
                body = resp.read()
        except urllib.error.HTTPError as exc:
            # 上游 4xx/5xx 也是有效响应：把状态码和正文交给页面，HTTP 客户端才能
            # 分辨 404（基金代码不存在）与网络失败。
            status = exc.code
            content_type = exc.headers.get('Content-Type') if exc.headers else None
            body = exc.read() if exc.fp else b''
            content_type = content_type or 'text/plain; charset=utf-8'
        except Exception as exc:  # noqa: BLE001 - 转成 502 交给前端看
            self.send_error(502, 'upstream %s failed: %s' % (url, exc))
            return

        cookies = '; '.join('%s=%s' % (c.name, c.value) for c in jar)
        self.send_response(status)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(body)))
        if cookies:
            self.send_header('X-Soui-Cookie', cookies)
        self.end_headers()
        self.wfile.write(body)

    # ------------------------------------------------------------- 响应头
    def end_headers(self):
        if self._isolated:
            self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
            self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        # 产物文件名固定，缓存下来会拿到旧胶水/旧数据（表现是 LinkError）。
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def log_message(self, fmt, *args):
        # 只打异常与非 2xx：加载一次模块就有几十条 200，刷屏看不清问题。
        code = args[1] if len(args) > 1 else ''
        if isinstance(code, int) and code >= 400:
            super().log_message(fmt, *args)


def _add_arguments(ap):
    ap.add_argument('--http-proxy', action='store_true',
                    help='打开同源 HTTP 转发端点 /__soui_http?url=...；漏了它页面能打开但取数全部 501')
    ap.add_argument('--allow-host', action='append', default=[], metavar='DOMAIN',
                    help='只转发到该域名或其子域，可重复；缺省不限制，仅适合本机开发')


def build_parser():
    ap = argparse.ArgumentParser(
        description='基金持仓 wasm 版：带 COOP/COEP 与 /__soui_http 转发的静态服务')
    ap.add_argument('directory', nargs='?', default=os.getcwd(),
                    help='要服务的目录，默认当前工作目录')
    ap.add_argument('--port', type=int, default=8080)
    ap.add_argument('--bind', default='',
                    help='监听地址，默认所有网卡；放在 Apache/Nginx 后面时用 127.0.0.1 只允许本机反代访问')
    ap.add_argument('--no-isolation', action='store_true',
                    help='不发 COOP/COEP，用来验证退回泵后端的降级路径')
    _add_arguments(ap)
    return ap


def _startup(root, args):
    print('  HTTP 转发：/__soui_http?url=<百分号编码的绝对 URL>')
    if args.allow_host:
        print('  允许转发的上游：%s' % '、'.join(args.allow_host))
    elif needs_http_proxy(root):
        print('  警告：没带 --allow-host，/__soui_http 会转发到任意 http(s) 地址，'
              '只适合本机开发；对公网部署请加上 --allow-host eastmoney.com。')


def serve(args):
    root = os.path.abspath(args.directory)
    handler = functools.partial(FundHoldingsHandler,
                                directory=root,
                                isolated=not args.no_isolation,
                                options=args)
    socketserver.ThreadingTCPServer.allow_reuse_address = True
    # 每连接一条线程，一个慢请求不会堵住 wasm 模块的并发加载。
    with socketserver.ThreadingTCPServer((args.bind or '', args.port), handler) as httpd:
        httpd.daemon_threads = True
        state = '关闭（--no-isolation）' if args.no_isolation else '开启'
        print('服务 %s' % root)
        print('http://%s:%d/%s   crossOriginIsolated 头：%s'
              % (args.bind or 'localhost', args.port, guess_page(root), state))
        if args.no_isolation:
            print('  注意：这个模式下浏览器不会给出 SharedArrayBuffer，wasm 构建会退回泵后端。')
        if args.http_proxy:
            _startup(root, args)
        elif needs_http_proxy(root):
            print('')
            print('  警告：该目录的页面取数要走 /__soui_http，本次没带 --http-proxy。')
            print('        页面能打开，但数据请求会全部失败（端点固定返回 501）。')
            print('        请 Ctrl+C 后重启： python "%s" "%s" --port %d --http-proxy'
                  % (__file__, root, args.port))
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == '__main__':
    serve(build_parser().parse_args())
