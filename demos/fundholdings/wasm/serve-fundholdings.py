#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""serve-fundholdings.py -- 基金持仓 wasm 版的本地静态服务入口。

通用部分（.wasm MIME、COOP/COEP、--no-isolation、--ws-proxy）全部继承
tools/wasm/serve-isolated.py，本文件只放基金持仓独有的那块：同源 HTTP 转发端点
/__soui_http。这样通用服务里不会出现任何 app 名、页面名或上游接口约定。

为什么必须有这个端点：浏览器里的 XHR 不允许脚本设置 Cookie / Referer /
User-Agent（禁止头），也不允许读取 Set-Cookie，而东财接口恰好两样都要。客户端
components/httpclient/wasm/HttpClient.cpp 把每个请求改写成本端点，并把禁止头
改名成 X-Soui-Cookie / X-Soui-Referer / X-Soui-User-Agent /
X-Soui-Accept-Language；本端点还原成真正的上游请求头，响应的 Set-Cookie 经
CookieJar 汇总后放进 X-Soui-Cookie 响应头回给页面。同源路径同时绕开了 CORS
（fundf10.eastmoney.com 不返回 Access-Control-Allow-Origin，直连一定被拦）。

用法（worker 产物 out_pt / 泵产物 out 都要带 --http-proxy）：

    python3 demos/fundholdings/wasm/serve-fundholdings.py \
        demos/fundholdings/wasm/out_pt --port 8080 --http-proxy

漏了 --http-proxy 页面照样能打开（静态资源是本地的），但取数请求全部撞 501，
页面上不容易看出网络错误；所以启动时会按落地页名字检查并打印重启命令。
不带这个端点时（例如只用通用 serve-isolated.py 或 python -m http.server），
页面探测 /__soui_http 得到 404，状态行同样会给出警告。

安全提示：这个端点没有上游域名白名单，本质是开放代理。只用于本机开发，
监听范围请限制在 127.0.0.1 / 本机防火墙内，不要直接暴露到公网。
"""

import http.cookiejar
import importlib.util
import os
import pathlib
import sys
import urllib.error
import urllib.parse
import urllib.request

# 通用服务的文件名带连字符，不能直接 import；按仓库根用 importlib 装载。
# __file__ = <repo>/demos/fundholdings/wasm/serve-fundholdings.py，parents[3] = <repo>。
_REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]
_GENERIC = _REPO_ROOT / 'tools' / 'wasm' / 'serve-isolated.py'
_spec = importlib.util.spec_from_file_location('soui_serve_isolated', _GENERIC)
_base = importlib.util.module_from_spec(_spec)
# 通用服务是被按路径加载的模块，默认会在 tools/wasm/ 下留一份 __pycache__；
# 那是共享工具目录，不该被开发时的字节码污染。
sys.dont_write_bytecode = True
_spec.loader.exec_module(_base)

# 取数依赖 /__soui_http 的落地页。按文件名识别，仅供"漏了 --http-proxy"的
# 启动期提醒，不是转发逻辑的一部分；新增同类页面时登记到这里即可。
HTTP_PROXY_PAGES = frozenset(('fundholdings.html',))


def needs_http_proxy(directory):
    try:
        return any(name.lower() in HTTP_PROXY_PAGES for name in os.listdir(directory))
    except OSError:
        return False


class FundHoldingsHandler(_base.IsolatedHandler):
    def handle_custom_request(self):
        # 必须在静态文件之前判断：这个路径不对应磁盘文件。
        if not self.path.startswith('/__soui_http'):
            return False
        self.forward_http()
        return True

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


def _add_arguments(ap):
    ap.add_argument('--http-proxy', action='store_true',
                    help='打开同源 HTTP 转发端点 /__soui_http?url=...；漏了它页面能打开但取数全部 501')


def _startup(root, args):
    if args.http_proxy:
        print('  HTTP 转发：/__soui_http?url=<百分号编码的绝对 URL>')
    elif needs_http_proxy(root):
        print('')
        print('  警告：该目录的页面取数要走 /__soui_http，本次没带 --http-proxy。')
        print('        页面能打开，但数据请求会全部失败（端点固定返回 501）。')
        print('        请 Ctrl+C 后重启： python "%s" "%s" --port %d --http-proxy'
              % (__file__, root, args.port))


if __name__ == '__main__':
    _base.main(handler_class=FundHoldingsHandler,
               add_arguments=_add_arguments,
               description='基金持仓 wasm 版：带 COOP/COEP 与 /__soui_http 转发的静态服务',
               startup_hook=_startup)
