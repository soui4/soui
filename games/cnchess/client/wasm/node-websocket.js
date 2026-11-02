/*
 * node-websocket.js -- 给无头烟测用的最小 WebSocket 客户端
 *
 * wasm 版 ws 组件（components/ws/src/wasm/）不再用 libwebsockets，改成调
 * emscripten 的 emscripten_websocket_*，底下就是浏览器的 WebSocket 对象。
 * node 18 没有全局 WebSocket，所以烟测必须自带一个，否则整条网络路径在无头
 * 环境里根本跑不起来，"到底是走了浏览器原生实现"这件事也就无从验证。
 *
 * 它只实现 RFC6455 客户端侧：握手、掩码、文本/二进制帧、分片重组、ping/pong、
 * close。没有 permessage-deflate（不发送扩展头，服务端就不会压缩），也不支持
 * wss://（烟测连本机 ws://）。
 *
 * 注意两点与浏览器一致、但常被忽略的语义：
 *   - send() 收到 ArrayBufferView 时按二进制帧发（emscripten 的 send_binary
 *     传进来的就是 HEAPU8.subarray(...)，绝不能当字符串处理）；
 *   - 服务端帧不带掩码，客户端帧必须带掩码，且 >125 字节要用 16/64 位长度。
 */
'use strict';

const net = require('net');
const crypto = require('crypto');

const GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11';

class NodeWebSocket {
    constructor(url, protocols) {
        this.url = url;
        this.binaryType = 'blob';
        this.readyState = NodeWebSocket.CONNECTING;
        this.protocol = '';
        this.extensions = '';
        this.onopen = null;
        this.onerror = null;
        this.onclose = null;
        this.onmessage = null;

        this._buf = Buffer.alloc(0);
        this._handshook = false;
        this._frag = [];
        this._fragOp = 0;
        this._socket = null;

        const u = new URL(url);
        if (u.protocol !== 'ws:') {
            setImmediate(() => this._fail('only ws:// is supported by this stub'));
            return;
        }
        const port = Number(u.port || 80);
        const key = crypto.randomBytes(16).toString('base64');
        this._socket = net.connect(port, u.hostname, () => {
            let req = 'GET ' + (u.pathname || '/') + (u.search || '') + ' HTTP/1.1\r\n' +
                'Host: ' + u.hostname + ':' + port + '\r\n' +
                'Upgrade: websocket\r\nConnection: Upgrade\r\n' +
                'Sec-WebSocket-Key: ' + key + '\r\nSec-WebSocket-Version: 13\r\n';
            if (protocols && protocols.length) {
                req += 'Sec-WebSocket-Protocol: ' + [].concat(protocols).join(', ') + '\r\n';
            }
            this._socket.write(req + '\r\n');
        });
        this._socket.on('data', (d) => this._onData(d, key));
        this._socket.on('error', (e) => this._fail(e.message));
        this._socket.on('close', () => {
            if (this.readyState !== NodeWebSocket.CLOSED) this._doClose(1006, '', false);
        });
    }

    send(data) {
        if (this.readyState !== NodeWebSocket.OPEN) throw new Error('INVALID_STATE_ERR');
        if (typeof data === 'string') {
            this._sendFrame(0x1, Buffer.from(data, 'utf8'));
        } else if (ArrayBuffer.isView(data)) {
            // 立刻拷一份：emscripten 传进来的是 HEAPU8 上的视图，写出前堆可能被搬动
            this._sendFrame(0x2, Buffer.from(data.buffer, data.byteOffset, data.byteLength));
        } else {
            this._sendFrame(0x2, Buffer.from(data));
        }
    }

    close(code, reason) {
        if (this.readyState === NodeWebSocket.CLOSED || this.readyState === NodeWebSocket.CLOSING) return;
        this.readyState = NodeWebSocket.CLOSING;
        const r = Buffer.from(reason || '', 'utf8');
        const payload = Buffer.alloc(code ? 2 + r.length : 0);
        if (code) {
            payload.writeUInt16BE(code, 0);
            r.copy(payload, 2);
        }
        this._sendFrame(0x8, payload);
        this._socket.end();
    }

    _fail(msg) {
        if (this.onerror) this.onerror({ message: msg });
        this._doClose(1006, msg, false);
    }

    _doClose(code, reason, wasClean) {
        this.readyState = NodeWebSocket.CLOSED;
        if (this._socket) this._socket.destroy();
        if (this.onclose) this.onclose({ code: code, reason: reason || '', wasClean: wasClean });
    }

    _sendFrame(op, payload) {
        const len = payload.length;
        let head;
        if (len < 126) {
            head = Buffer.alloc(2);
            head[1] = 0x80 | len;
        } else if (len < 65536) {
            head = Buffer.alloc(4);
            head[1] = 0x80 | 126;
            head.writeUInt16BE(len, 2);
        } else {
            head = Buffer.alloc(10);
            head[1] = 0x80 | 127;
            head.writeUInt32BE(0, 2);
            head.writeUInt32BE(len, 6);
        }
        head[0] = 0x80 | op;
        const mask = crypto.randomBytes(4);
        const masked = Buffer.alloc(len);
        for (let i = 0; i < len; ++i) masked[i] = payload[i] ^ mask[i & 3];
        this._socket.write(Buffer.concat([head, mask, masked]));
    }

    _onData(chunk, key) {
        this._buf = Buffer.concat([this._buf, chunk]);
        if (!this._handshook) {
            const end = this._buf.indexOf('\r\n\r\n');
            if (end < 0) return;
            const head = this._buf.slice(0, end).toString('latin1');
            this._buf = this._buf.slice(end + 4);
            const lines = head.split('\r\n');
            if (!/^HTTP\/1\.1 101/.test(lines[0])) {
                this._fail('handshake failed: ' + lines[0]);
                return;
            }
            const hdrs = {};
            for (let i = 1; i < lines.length; ++i) {
                const c = lines[i].indexOf(':');
                if (c > 0) hdrs[lines[i].slice(0, c).trim().toLowerCase()] = lines[i].slice(c + 1).trim();
            }
            const expect = crypto.createHash('sha1').update(key + GUID).digest('base64');
            if (hdrs['sec-websocket-accept'] !== expect) {
                this._fail('bad Sec-WebSocket-Accept');
                return;
            }
            this._handshook = true;
            this.readyState = NodeWebSocket.OPEN;
            if (this.onopen) this.onopen({});
        }
        for (;;) {
            const f = this._readFrame();
            if (!f) break;
            this._dispatch(f);
            if (this.readyState === NodeWebSocket.CLOSED) return;
        }
    }

    _readFrame() {
        const b = this._buf;
        if (b.length < 2) return null;
        const fin = (b[0] & 0x80) !== 0;
        const op = b[0] & 0x0f;
        const hasMask = (b[1] & 0x80) !== 0; // 服务端帧不该带掩码，带了也照解
        let len = b[1] & 0x7f;
        let off = 2;
        if (len === 126) {
            if (b.length < 4) return null;
            len = b.readUInt16BE(2);
            off = 4;
        } else if (len === 127) {
            if (b.length < 10) return null;
            len = Number(b.readBigUInt64BE(2));
            off = 10;
        }
        const maskLen = hasMask ? 4 : 0;
        if (b.length < off + maskLen + len) return null;
        let payload = b.slice(off + maskLen, off + maskLen + len);
        if (hasMask) {
            const mask = b.slice(off, off + 4);
            payload = Buffer.from(payload);
            for (let i = 0; i < payload.length; ++i) payload[i] ^= mask[i & 3];
        }
        this._buf = b.slice(off + maskLen + len);
        return { fin: fin, op: op, payload: payload };
    }

    _dispatch(f) {
        if (f.op === 0x8) {
            const code = f.payload.length >= 2 ? f.payload.readUInt16BE(0) : 1005;
            const reason = f.payload.length > 2 ? f.payload.slice(2).toString('utf8') : '';
            this._sendFrame(0x8, f.payload.slice(0, 2));
            this._doClose(code, reason, true);
            return;
        }
        if (f.op === 0x9) { this._sendFrame(0xA, f.payload); return; }
        if (f.op === 0xA) return;
        if (f.op !== 0x0) {
            this._fragOp = f.op;
            this._frag = [];
        }
        this._frag.push(f.payload);
        if (!f.fin) return;
        const data = Buffer.concat(this._frag);
        this._frag = [];
        if (!this.onmessage) return;
        if (this._fragOp === 0x1) {
            this.onmessage({ data: data.toString('utf8') });
        } else if (this.binaryType === 'arraybuffer') {
            // emscripten 把 binaryType 设成 arraybuffer，然后按 e.data.byteLength 拷贝
            const ab = new ArrayBuffer(data.length);
            new Uint8Array(ab).set(data);
            this.onmessage({ data: ab });
        } else {
            this.onmessage({ data: data });
        }
    }
}
NodeWebSocket.CONNECTING = 0;
NodeWebSocket.OPEN = 1;
NodeWebSocket.CLOSING = 2;
NodeWebSocket.CLOSED = 3;

module.exports = NodeWebSocket;
