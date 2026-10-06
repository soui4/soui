win = nil;
tid = 0;
gamewnd = nil;
gamecanvas = nil;
players = {};
flag_win = nil;
runTimer = nil;

coins_all = 100;	--现有资金
coins_bet = {0,0,0,0} --下注金额
bet_rate = 4;		--赔率
prog_max	 = 200;	--最大步数
prog_all = {0,0,0,0} --马匹进度

function on_host_msg(hostWnd,msg, wp,lp,pRes)
	--slog("test on host msg:" .. msg);
	if msg == 0x82 then -- 0x82 == WM_NCDESTROY
		HostWnd_SetMsgHandler(hostWnd,"",nil);
	end
	return 0;
end

function onBtnLrc(e)
 slog("btn lrc clicked!");
 local btn = toSWindow(e:Sender());
 local hwnd = btn:GetHostHwnd();
 local ret = SMessageBox(hwnd,L"onBtnLrc(我是中文), cancel origin proc?",L"test",1);
 if ret == 1 then
	 e:SetBubbleUp(0); --block origin behavior.
 end
 return 1;
end

function on_init(args)
	--初始化全局对象
	slog("on_init");
	win = getHostFromInitEvent(args);
	HostWnd_SetMsgHandler(win,"on_host_msg",win);

	if win == nil then
		return 0;
	end
	local root = toSWindow(args:Sender());
	gamewnd = root:FindChildByNameA("game_wnd",-1);
	gamecanvas = gamewnd:FindChildByNameA("game_canvas",-1);
	flag_win = gamewnd:FindChildByNameA("flag_win",-1);
	players = {
					gamecanvas:FindChildByNameA("player_1",-1),
					gamecanvas:FindChildByNameA("player_2",-1),
					gamecanvas:FindChildByNameA("player_3",-1),
					gamecanvas:FindChildByNameA("player_4",-1)
				    };
	--布局
	on_canvas_size(nil);

	--show how to do SubscribeEvent
	local btnLrc = root:FindChildByNameA("btn_lrc",-1);
	local lrcSlot = CreateEventSlot("onBtnLrc");
	btnLrc:SubscribeEvent(10000,lrcSlot); -- 10000 == EVT_CMD
	lrcSlot:Release();
	math.randomseed(os.time());
	local souiFac = CreateSouiFactory();
	local timerSlot = CreateEventSlot("on_timer");
	runTimer = souiFac:CreateTimer(timerSlot);
	timerSlot:Release();
	souiFac:Release();
	--init the lua match-3 game (page_script 的"消消乐"子页)
	xxl_init(root);
end

function on_exit(args)
	slog("execute script function: on_exit");
	win = getHostFromInitEvent(args);
	HostWnd_SetMsgHandler(win,"",nil);--remove msg handler
	runTimer:Release();
	xxl_exit();
end

function on_timer(args)
	if(gamewnd ~= nil) then

		local rcCanvas = gamecanvas:GetWindowRect2();
		local heiCanvas = rcCanvas:Height();
		local widCanvas = rcCanvas:Width();

		local rcPlayer =  players[1]:GetWindowRect2();
		local wid = rcPlayer:Width();
		local hei = rcPlayer:Height();

		local win_id = 0;
		for i = 1,4 do
			local prog = prog_all[i];
			if(prog<prog_max) then
				prog = prog + math.random(0,10);
				prog_all[i] = prog;
				local rc = players[i]:GetWindowRect2();
				rc.left = rcCanvas.left + (widCanvas-wid)*prog/prog_max;
				players[i]:Move2(rc.left,rc.top,-1,-1);
			else
				win_id = i;

				local rc = players[i]:GetWindowRect2();
				rc.left = rcCanvas.left + (widCanvas-wid);
				players[i]:Move2(rc.left,rc.top,-1,-1);
			end
		end

		if win_id ~= 0 then
			gamewnd:FindChildByNameA("btn_run",-1):FireCommand();
			coins_all = coins_all + coins_bet[win_id] * 4;
			gamewnd:FindChildByNameA("txt_coins",-1):SetWindowText(T(coins_all));

			coins_bet = {0,0,0,0};

			local rcPlayer = players[win_id]:GetWindowRect2();
			local widPlayer = rcPlayer:Width();
			local heiPlayer = rcPlayer:Height();
			local szFlag = CSize(0,0); 
			flag_win:GetDesiredSize(szFlag,widPlayer,heiPlayer);
			rcPlayer.right = rcPlayer.left + szFlag.cx;
			rcPlayer.bottom = rcPlayer.top + szFlag.cy;
			rcPlayer:OffsetRect(-szFlag.cx,-szFlag.cy/3);

			flag_win:Move(rcPlayer);
			flag_win:SetVisible(1,1);
			flag_win:SetUserData(win_id);

			for i= 101,104 do
				gamewnd:FindChildByID(i,-1):SetWindowText(T("0"));
			end
		end
	end
end

function on_bet(args)
	if tid ~= 0 then
		return 1;
	end

	local btn = toSWindow(args:Sender());
	if coins_all >= 10 then
	    --id range from 101-104
		id = btn:GetID()-100;
		coins_bet[id] = coins_bet[id] + 10;
		coins_all = coins_all -10;
		btn:SetWindowText(T(coins_bet[id]));

		gamewnd:FindChildByNameA("txt_coins",-1):SetWindowText(T(coins_all));

	end
	return 1;
end

function on_canvas_size(args)
	if win == nil then
		return 0;
	end

	local rcCanvas =  gamecanvas:GetWindowRect2();
	local heiCanvas = rcCanvas:Height();
	local widCanvas = rcCanvas:Width();

	local szPlayer =CSize(0,0);
	players[1]:GetDesiredSize(szPlayer,widCanvas,heiCanvas);

	local wid = szPlayer.cx;
	local hei = szPlayer.cy;

	local rcPlayer = CRect(0,0,wid,hei);
	local interval = (heiCanvas - hei*4)/5;
	rcPlayer:OffsetRect(rcCanvas.left,rcCanvas.top+interval);
	for i = 1, 4 do
		local rc = rcPlayer;
		rc.left = rcCanvas.left + (widCanvas-wid)*prog_all[i]/prog_max;
		rc.right = rc.left+wid;
		players[i]:Move(rc);
		rcPlayer:OffsetRect(0,interval+hei);
	end

	local win_id = flag_win:GetUserData();
	if win_id ~= 0 then
		local rcPlayer = players[win_id]:GetWindowRect2();
		local widPlayer = rcPlayer:Width();
		local heiPlayer = rcPlayer:Height();
		local szFlag = CSize(0,0);
		flag_win:GetDesiredSize(szFlag,widPlayer,heiPlayer);
		flag_win:Move2(rcPlayer.left-szFlag.cx,rcPlayer.top-szFlag.cy/3,-1,-1);
	end

	return 1;

end

function on_run(args)

	if win == nil then
		return 0;
	end
	local btn = toSWindow(args:Sender());
	if tid == 0 then
		prog_all = {0,0,0,0};
		on_canvas_size(nil);
		tid = runTimer:StartTimer(50,1,0);
		btn:SetWindowText(T"stop");
		flag_win:SetVisible(0,1);
	else
		runTimer:KillTimer();
		btn:SetWindowText(T"run");
		tid = 0;
	end
	return 1;
end

function on_btn_select_cbx(args)
	local btn = toSWindow(args:Sender());
	local cbxwnd = btn:GetWindow(2);--get previous sibling
	local cbx = toSComboBase(cbxwnd);
	cbx:SetCurSel(-1);
end

--[[
  onDynBtnCmd - page_misc "create window" 页演示:
  edit_xml 的默认 XML 中按钮带 on_command="onDynBtnCmd",
  点击 CreateChildren 动态创建后,按钮点击事件经脚本模块路由到此函数。
  用 SMessageBox 弹窗演示 lua 响应动态创建窗口的命令事件。
]]
function onDynBtnCmd(args)
	local btn = toSWindow(args:Sender());
	local ret = SMessageBox(btn:GetHostHwnd(), L"dynamic created button clicked!\r\nlua handler: onDynBtnCmd(event: on_command)", L"msgbox", 1);
	slog("onDynBtnCmd ret=" .. tostring(ret));
	--return 1 阻断事件继续冒泡
	return 1;
end


--[[
==================== 消消乐(lua 版) ====================
1:1 移植自 soxxl main.js(测试通过的 js 参考实现),改用 ScriptModule-LUA 导出。
函数与 js 的 Board/MainDialog 方法一一对应,动画衔接顺序与 js 完全一致:
  点击 -> 选中动画(anim:xxl_scale_select)
  点相邻格 -> 两元素交换动画组 -> 组结束:提交交换数据(js 同款,不消除也不回退)
           -> 全盘检查 -> 有 3 连:聚拢动画组 -> 组结束:上方元素下落动画组
           -> 组结束:计分+数据补位(freeSameX/Y) -> 再次全盘检查(级联)
与 js 的三处刻意偏离(均为修复,勿"改回"):
  1. 浮层副本 id 全局唯一:级联时同一格可能同时有多个存活副本,复用格子 id
     会让 FindChildByID 撞回旧副本,旧副本销毁后新动画仍 tick => 段错误;
  2. freeSameY 源行越界保护:js 的 this.board[x][y-1-i] 是转置索引笔误,
     贴顶消除时越界(js 靠 setGridState 的 try/catch 掩盖);
  3. 副本创建失败时回滚格子可见性,绝不让格子凭空消失。
市场常见玩法完善(在 js 版基础上新增):
  - 金币/得分用七段 LED 翻页计数器(t:g.xxl_digit,SelectPage 驱动,push 动画);
  - 初始棋盘受约束随机(xxl_gen_board),绝无初始 3 连且保证有解
    (旧版裸随机开局即自动消除,白送分且棋盘缺格);
  - 连击计分:第 n 波消除得分 = len * n,新交换清零,settle 时清零显示;
    连击 x2 起在连击文本位播同款金星芒(字号已放大到 26);
  - 金币入账链(三段顺序,xxl_coin_fx 入口):① 消除星芒(特效窗口播 anim,
    on_animation_stop 驱动转段) => ② 数值动画把【这个特效窗口本身】直线移到
    金币数字区(SetRangeRect 插值,终点缩 22x22,xxl_coin_fly_begin) =>
    ③ LED 滚动计数到新值(xxl_coin_count_begin);金币数据在 ① 起点(clear_end)
    即 +1,仅显示层延迟;每段独立计数单位,段尾先开下一段再释放本段,
    ani_count 归零(全局动画收尾)统一补跑 xxl_on_settle;
  - 动画浮层 wnd_xxl_aniframe 扩为整页 float 覆盖层(page_script.xml,
    盖住棋盘 + 左侧功能按钮/计分区;原 stack 会裁剪子窗口,不能放里面);
  - 洗牌为主动技能:点"洗牌"按钮花 xxl_shuffle_cost 金币,对现有棋子做位置
    置换(Fisher-Yates,约束重试:无初始 3 连且有解,不重新发牌);
    每枚棋子做两段数值动画(SValueAnimator 驱动,xxl_on_shuffle => xxl_redeal,
    随时可洗):第一段以棋子当前位置为起点、棋盘中心为圆心绕行
    1.0~3.0 圈(随机方向),第二段从轨道终点直线飞往目标棋格;
    棋子一落到目标格立即在目标位显示(新盘数据随置换即时提交,
    xxl_shuffle_arrive);全部棋子归位后(xxl_shuffle_finalize)才检查消除行列;
    洗牌全程禁用棋盘窗口防误操作(结束/中途重开时恢复);
    金币不足不能洗(提示不结算);死局且金币不足(settle 判定解不开) => 游戏结束;
  - 提示按钮(xxl_on_hint):找一个可行交换,两枚棋子复用选中脉冲动画;
  - 金币耗尽/死局无力洗牌 => 游戏结束弹窗(SMessageBox),确定后重开。
棋盘 id 约定: xxl_base_id + y*8 + x (x,y 从 0 开始)。
]]

xxl_base_id = 30000;
xxl_row = 8; xxl_col = 8;
xxl_max_state = 7; xxl_min_same = 3;
xxl_shuffle_cost = 5; -- 死局洗牌费用(金币)

xxl = {
	board = {};      -- board[y][x] = icon state(0..6)
	click_id = -1;
	coin = 20; score = 0;
	coin_shown = 20; -- 金币 LED 当前显示值(滚动计数期间落后于 coin,落地/支付即同步)
	combo = 0;       -- 连击数(第 n 波消除得分 x n,settle 清零)
	gameover = false;-- 金币耗尽弹窗只出一次
	ani_list = {};   -- ctxId -> 动画组(js ani_list;按 ctxId 索引,勿用对象身份比较)
	ani_orphan = {}; -- 上一次 init_board 时的存活动画组(保活防 GC 悬挂,重建时清)
	ani_count = 0;   -- 存活动画组数
	ani_ctx = {};    -- ctxId -> {kind=..., ele=..., ani_widget=...}
	ani_seq = 0;
	copy_seq = 0;    -- 浮层副本 id 序号(全局唯一)
	ani_move = nil;  -- 模板动画(animator:xxl_move)
	ani_sel = nil;   -- 选中标记动画(anim:xxl_scale_select)
	fx_ani = {};     -- 特效动画缓存(key="anim:xxx",pop_fx 懒加载)
	fx_seq = 0;      -- 特效窗口 id 序号(每次动态创建,+xxl_fx_base_id)
	root = nil; wndBoard = nil; aniframe = nil;
};

function xxl_slog(tag)
	slog("XXL " .. tag);
end

function xxl_id2pos(id)
	id = id - xxl_base_id;
	return { x = id % xxl_col, y = math.floor(id / xxl_col) };
end

function xxl_pos2id(pos)
	return xxl_base_id + pos.y * xxl_col + pos.x;
end

function xxl_new_ctx(kind, data)
	xxl.ani_seq = xxl.ani_seq + 1;
	data.kind = kind;
	xxl.ani_ctx[xxl.ani_seq] = data;
	return xxl.ani_seq;
end

-- js showScore/showCoin 用三位数字 stack;LED 翻页计数器版(soxxl setDigit 同款):
-- 每位是 t:g.xxl_digit 十页 stack,SelectPage(digit,true) 走 push 翻页动画。
function xxl_set_digits(prefix, num)
	if xxl.root == nil then return end
	for i = 0, 2 do
		local stack = xxl.root:FindChildByNameA(prefix .. "_" .. i,-1);
		if stack then
			local d = math.floor(num / 10^i) % 10;
			local stackApi = QiIStackView(stack);
			if stackApi then
				stackApi:SelectPage(d, true);
				stackApi:Release();
			end
		end
	end
end

function xxl_show_score()
	xxl_set_digits("digit_score", xxl.score % 1000);
end

function xxl_show_coin()
	xxl.coin_shown = xxl.coin; -- 即时路径(支付/重开)直接对齐显示值
	xxl_set_digits("digit_coin", xxl.coin % 1000);
end

-- 连击提示文本(第 n 波消除 n>=2 才显示;字号已放大,不播特效——
-- 每波消除中心已有星芒,连击位再播会重复且遮挡金币区)
function xxl_show_combo()
	if xxl.root == nil then return end
	local txt = xxl.root:FindChildByNameA("txt_xxl_combo",-1);
	if txt then
		if xxl.combo >= 2 then
			txt:SetWindowText(T("连击 x" .. xxl.combo .. "!"));
		else
			txt:SetWindowText(T(""));
		end
	end
end

-- ============ 金币入账链:① 消除星芒 → ② 数值动画移动特效窗口 → ③ LED 滚动计分 ============
-- 三段顺序执行,每段一个计数单位;段尾先启动下一段(其 +1)再释放本段(-1),
-- ani_count 不中途归零 => settle 链不会被打断;真正归零(全局动画收尾)时补跑 settle。
-- 金币数据在 ① 起点(clear_end)即 +1,③ 才滚动显示(显示层延迟,数据层即时)。

-- 金币数字区中心(宿主坐标系;取中间那颗 LED 的矩形)
function xxl_get_coin_rect()
	local ele = xxl.root:FindChildByNameA("digit_coin_1",-1);
	if ele == nil then return nil end
	return ele:GetWindowRect2();
end

-- 计数单位 -1;归零 => 全部动画结束,补跑稳定态收尾(settle 自带门禁,可安全多发)
function xxl_ani_count_dec()
	xxl.ani_count = xxl.ani_count - 1;
	if xxl.ani_count < 0 then xxl.ani_count = 0 end
	xxl_slog("coin: dec -> ani_count=" .. xxl.ani_count);
	if xxl.ani_count == 0 then
		xxl_slog("coin: chain all done, run settle");
		xxl_on_settle();
	end
end

-- ① 特效段:消除中心播星芒(短停留版 anim:xxl_fx_pop_fly,600ms,
-- 播完经 on_animation_stop 转入飞行段;装饰特效仍用长停留版 xxl_fx_pop)
function xxl_coin_fx(rc, boost)
	local fx = xxl_pop_fx(rc, boost, nil, "anim:xxl_fx_pop_fly", nil, "xxl_fx_fly_start");
	if fx == nil then
		xxl_slog("coin: [1]fx create FAILED, fallback count");
		xxl_coin_count_begin(); -- 特效起不来:跳过动画直接计分(计数段自带单位)
		return
	end
	xxl.ani_count = xxl.ani_count + 1;
	xxl_slog("coin: [1]fx #" .. xxl.fx_seq .. " boost=" .. tostring(boost) .. " ani_count=" .. xxl.ani_count);
end

-- ①→② 特效动画播完(EventSwndAnimationStop,Sender=特效窗口):
-- 不销毁,用数值动画把这个特效窗口本身直线移到金币数字区。
function xxl_fx_fly_start(args)
	local fx = toSWindow(args:Sender());
	if fx == nil then
		xxl_slog("coin: [1->2]stop event sender nil, fallback count");
		xxl_coin_count_begin();
		xxl_ani_count_dec();
		return
	end
	xxl_slog("coin: [1->2]fx anim stop -> fly");
	local started = xxl_coin_fly_begin(fx);
	if not started then
		fx:Destroy();
		xxl_slog("coin: [2]fly begin FAILED, fallback count");
		xxl_coin_count_begin(); -- 飞行没接上:兜底计分(计数段自带单位)
	end
	xxl_ani_count_dec(); -- 释放特效段(下一段已 +1,不会中断 settle 链)
end

-- ② 飞行段:特效窗口从当前位置沿抛物线飞到金币数字区中心(终点缩为 22x22):
-- x 线性插值,y 在线性插值上叠加向上弓起的抛物线偏移 arc*4t(1-t)(中点最高),
-- 宽高同步插值 => 起飞是全尺寸星芒,落地缩成小金币大小。
-- xxl_move 模板无插值器 => GetFraction 即线性进度 t,轨迹形状精确可控。
function xxl_coin_fly_begin(fx)
	local rcTo = xxl_get_coin_rect();
	if rcTo == nil then
		xxl_slog("coin: [2]coin rect nil (digit_coin_1 not found)");
		return false
	end
	local rc1 = fx:GetWindowRect2();
	local cx = rcTo.left + rcTo:Width()/2;
	local cy = rcTo.top + rcTo:Height()/2;
	local rc2 = CRect(math.floor(cx-11), math.floor(cy-11), math.floor(cx-11)+22, math.floor(cy-11)+22);
	-- 弓高:与飞行距离成比例(35%),限制在 60~150px,方向向上
	local cx1 = rc1.left + rc1:Width()/2;
	local cy1 = rc1.top + rc1:Height()/2;
	local dist = math.sqrt((cx-cx1)^2 + (cy-cy1)^2);
	local arc = math.max(60, math.min(150, dist * 0.35));
	local ani = LuaValueAnimator();
	ani:CopyFrom(xxl.ani_move:GetIValueAnimator());
	ani:GetIValueAnimator():setDuration(400);
	ani:SetRangeRect(rc1, rc2); -- 线性基准(实际位置在 update 里叠加抛物线偏移)
	ani:SetOnUpdate("xxl_fly_update");
	ani:SetOnEnd("xxl_fly_end");
	local ctxId = xxl_new_ctx("coin_fly", { ani_widget=fx, ani=ani, r1=rc1, r2=rc2, arc=arc });
	ani:SetCtx(ctxId);
	xxl.ani_count = xxl.ani_count + 1;
	-- 🚨 lua_tinker 的 BOOL 返回是数字,用 ==0 判(勿用 not)
	local ok = ani:Start(xxl.aniframe);
	xxl_slog("coin: [2]fly start=" .. tostring(ok) .. " ani_count=" .. xxl.ani_count);
	if ok == 0 or ok == false then
		xxl.ani_ctx[ctxId] = nil;
		xxl.ani_count = xxl.ani_count - 1;
		if xxl.ani_count < 0 then xxl.ani_count = 0 end
		return false
	end
	return true
end

-- 飞行跟随:抛物线轨迹——x/宽/高线性插值,y 在线性插值上叠加向上弓起
-- 的偏移 arc*4t(1-t)(t=0/1 时为 0,中点最高,起终点精确落在两格中心)
function xxl_fly_update(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil or c.ani_widget == nil then return end
	local t = luaAni:GetFraction();
	local w = c.r1:Width() + (c.r2:Width() - c.r1:Width()) * t;
	local h = c.r1:Height() + (c.r2:Height() - c.r1:Height()) * t;
	local x = c.r1.left + (c.r2.left - c.r1.left) * t;
	local y = c.r1.top + (c.r2.top - c.r1.top) * t - c.arc * 4 * t * (1 - t);
	local l = math.floor(x);
	local tp = math.floor(y);
	c.ani_widget:Move(CRect(l, tp, l + math.floor(w), tp + math.floor(h)));
end

-- ②→③ 飞行落地:销毁特效窗口,先开计数段(+1)再释放飞行段(-1)
function xxl_fly_end(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil then
		xxl_slog("coin: [2]fly end but ctx gone (anomaly)");
		return
	end
	if c.ani_widget then c.ani_widget:Destroy(); end
	xxl.ani_ctx[ctxId] = nil;
	xxl_slog("coin: [2]fly landed");
	xxl_coin_count_begin();
	xxl_ani_count_dec();
end

-- ③ 计数段:LED 从显示值滚到数据值;无需滚动时不占计数单位(调用方释放自己那段)
function xxl_coin_count_begin()
	if xxl.coin_shown == nil then xxl.coin_shown = xxl.coin end
	if xxl.coin_shown == xxl.coin then
		xxl_slog("coin: [3]count skipped (shown=" .. xxl.coin_shown .. " == coin)");
		return
	end
	local ani = LuaValueAnimator();
	ani:CopyFrom(xxl.ani_move:GetIValueAnimator());
	ani:GetIValueAnimator():setDuration(400);
	ani:SetRangeRect(CRect(0,0,1,1), CRect(0,0,1,1)); -- rect 恒等,仅取 fraction 驱动
	ani:SetOnUpdate("xxl_coin_count_update");
	ani:SetOnEnd("xxl_coin_count_end");
	local ctxId = xxl_new_ctx("coin_count", { ani=ani, from=xxl.coin_shown, to=xxl.coin });
	ani:SetCtx(ctxId);
	xxl.ani_count = xxl.ani_count + 1;
	-- 🚨 lua_tinker 的 BOOL 返回是数字,用 ==0 判(勿用 not)
	local ok = ani:Start(xxl.aniframe);
	xxl_slog("coin: [3]count begin " .. xxl.coin_shown .. "->" .. xxl.coin .. " start=" .. tostring(ok) .. " ani_count=" .. xxl.ani_count);
	if ok == 0 or ok == false then
		xxl.ani_ctx[ctxId] = nil;
		xxl.ani_count = xxl.ani_count - 1;
		if xxl.ani_count < 0 then xxl.ani_count = 0 end
	end
end

-- 计数跟随:按插值 fraction 在 [from,to] 间取整滚动 LED
function xxl_coin_count_update(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil then return end
	local v = math.floor(c.from + (c.to - c.from) * luaAni:GetFraction() + 0.5);
	if v ~= xxl.coin_shown then
		xxl.coin_shown = v;
		xxl_set_digits("digit_coin", xxl.coin_shown % 1000);
	end
end

-- 计数结束:显示值对齐真实数据,释放计数段;归零 => 整局动画收尾补跑 settle
function xxl_coin_count_end(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil then return end
	xxl.ani_ctx[ctxId] = nil;
	xxl.coin_shown = xxl.coin; -- 终值对齐(多波叠加/中途重开都以数据为准)
	xxl_set_digits("digit_coin", xxl.coin_shown % 1000);
	xxl_ani_count_dec();
end

-- js onGridChanged
function xxl_on_grid_changed(pos, enableAni)
	local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
	if ele == nil then return end
	local stackApi = QiIStackView(ele);
	if stackApi == nil then return end
	stackApi:SelectPage(xxl.board[pos.y][pos.x], enableAni);
	stackApi:Release();
	ele:Invalidate(); -- SelectPage 内部翻页在不可见态可能不失效,强制补一次重绘
end

-- js setGridState
function xxl_set_grid_state(x,y,state,enableAni)
	xxl.board[y][x] = state;
	xxl_on_grid_changed({x=x,y=y},enableAni);
end

-- js getGridState
function xxl_get_grid_state(x,y)
	return xxl.board[y][x];
end

-- js canSwap
function xxl_can_swap(pos1,pos2)
	if pos1.x == pos2.x then
		return math.abs(pos1.y - pos2.y) == 1;
	elseif pos1.y == pos2.y then
		return math.abs(pos1.x - pos2.x) == 1;
	end
	return false;
end

-- js Board.swap: 交换数据并刷新两格,然后全盘检查(不消除也提交,js 同款)
function xxl_swap(pos1,pos2)
	if pos1.x < 0 or pos1.x >= xxl_col or pos1.y < 0 or pos1.y >= xxl_row then
		return false;
	end
	if pos2.x < 0 or pos2.x >= xxl_col or pos2.y < 0 or pos2.y >= xxl_row then
		return false;
	end
	if pos1.x == pos2.x and pos1.y == pos2.y then
		return false;
	end
	local tmp = xxl.board[pos1.y][pos1.x];
	xxl.board[pos1.y][pos1.x] = xxl.board[pos2.y][pos2.x];
	xxl.board[pos2.y][pos2.x] = tmp;
	xxl_on_grid_changed(pos1,false);
	xxl_on_grid_changed(pos2,false);
	return xxl_check_board();
end

-- js checkBoard: 先查横向再查纵向,一次只处理一组 3 连(级联由补位后再触发)
function xxl_check_board()
	if xxl_check_board_row() then return true end
	if xxl_check_board_col() then return true end
	xxl_on_settle();
	return false;
end

-- js checkBoardCol: 从底往上扫列
function xxl_check_board_col()
	for x = 0, xxl_col-1 do
		local nSame = 1;
		local y = xxl_row - 2;
		while y >= 0 do
			if xxl.board[y][x] == xxl.board[y+1][x] then
				nSame = nSame + 1;
			else
				if nSame >= xxl_min_same then
					y = y + 1;
					break;
				end
				nSame = 1;
			end
			y = y - 1;
		end
		if y < 0 then y = 0 end
		if nSame >= xxl_min_same then
			xxl_on_get_same_y(x,y,nSame);
			return true;
		end
	end
	return false;
end

-- js checkBoardRow: 从右往左扫行
function xxl_check_board_row()
	for y = xxl_row-1, 0, -1 do
		local nSame = 1;
		local x = 1;
		while x < xxl_col do
			if xxl.board[y][x] == xxl.board[y][x-1] then
				nSame = nSame + 1;
			else
				if nSame >= xxl_min_same then break end
				nSame = 1;
			end
			x = x + 1;
		end
		if nSame >= xxl_min_same then
			xxl_on_get_same_x(y, x - nSame, nSame);
			return true;
		end
	end
	return false;
end

-- js freeSameX: 横向清除补位,上方整段下移一格,顶行随机(带翻页动画)
function xxl_free_same_x(y,x,len)
	for i = y, 1, -1 do
		for j = x, x+len-1 do
			xxl_set_grid_state(j,i,xxl.board[i-1][j],false);
		end
	end
	for j = x, x+len-1 do
		xxl_set_grid_state(j,0,math.random(0, xxl_max_state-1),true);
	end
	xxl_check_board();
end

-- js freeSameY: 纵向清除补位,上方下移 len 格,顶部随机;带源行越界保护(见文件头)
function xxl_free_same_y(x,y,len)
	for i = 0, len-1 do
		local src = y-1-i;
		if src < 0 then break end -- 清除段贴顶:其余目标格由下面的随机填充覆盖
		xxl_set_grid_state(x,y+len-1-i,xxl.board[src][x],false);
	end
	for j = 0, len-1 do
		xxl_set_grid_state(x,j,math.random(0, xxl_max_state-1),true);
	end
	xxl_check_board();
end

-- ============ 市场常见玩法:死局检测 / 提示 / 洗牌 ============
-- pos 处交换后是否构成 3 连(纯数据,横竖两个方向)
function xxl_match_at(pos)
	local s = xxl.board[pos.y][pos.x];
	local n = 1;
	for i = pos.x-1, 0, -1 do
		if xxl.board[pos.y][i] ~= s then break end
		n = n + 1;
	end
	for i = pos.x+1, xxl_col-1 do
		if xxl.board[pos.y][i] ~= s then break end
		n = n + 1;
	end
	if n >= xxl_min_same then return true end
	n = 1;
	for j = pos.y-1, 0, -1 do
		if xxl.board[j][pos.x] ~= s then break end
		n = n + 1;
	end
	for j = pos.y+1, xxl_row-1 do
		if xxl.board[j][pos.x] ~= s then break end
		n = n + 1;
	end
	return n >= xxl_min_same;
end

-- 试交换 p1,p2(数据层试完立即还原),返回是否成 3 连
function xxl_try_swap_creates_match(p1,p2)
	local s1 = xxl.board[p1.y][p1.x];
	local s2 = xxl.board[p2.y][p2.x];
	xxl.board[p1.y][p1.x] = s2;
	xxl.board[p2.y][p2.x] = s1;
	local hit = xxl_match_at(p1) or xxl_match_at(p2);
	xxl.board[p1.y][p1.x] = s1;
	xxl.board[p2.y][p2.x] = s2;
	return hit;
end

-- 找一个可行交换(右邻、下邻两个方向),返回 {p1=..,p2=..} 或 nil
function xxl_find_move()
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			if x+1 < xxl_col and xxl_try_swap_creates_match({x=x,y=y},{x=x+1,y=y}) then
				return {p1={x=x,y=y}, p2={x=x+1,y=y}};
			end
			if y+1 < xxl_row and xxl_try_swap_creates_match({x=x,y=y},{x=x,y=y+1}) then
				return {p1={x=x,y=y}, p2={x=x,y=y+1}};
			end
		end
	end
	return nil;
end

function xxl_has_valid_move()
	return xxl_find_move() ~= nil;
end

-- 受约束随机生成棋盘:无初始 3 连,且保证有解(市场消消乐发牌规则)
function xxl_gen_board()
	for attempt = 1, 50 do
		for y = 0, xxl_row-1 do
			xxl.board[y] = {};
			for x = 0, xxl_col-1 do
				local s;
				repeat
					s = math.random(0, xxl_max_state-1);
				until not ((x >= 2 and xxl.board[y][x-1] == s and xxl.board[y][x-2] == s)
					or (y >= 2 and xxl.board[y-1][x] == s and xxl.board[y-2][x] == s));
				xxl.board[y][x] = s;
			end
		end
		if xxl_has_valid_move() then return end
	end
	-- 50 次仍无解(概率极低)接受最后一版,避免死循环
end

-- 当前盘是否存在任意 3 连(纯数据,供洗牌候选盘约束判定)
function xxl_board_has_match()
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			if xxl_match_at({x=x,y=y}) then return true end
		end
	end
	return false;
end

-- 现有棋子位置置换(Fisher-Yates,不重新发牌):约束重试 50 次——
-- 无初始 3 连且保证有解;50 次仍不满足(概率极低)接受最后一版。
-- 返回 nb(新盘数据)与 perm(位置置换表:perm[i+1] = 第 i 个棋子
-- (x=i%col, y=i/col) 的目标格 0 基索引)。判定期间临时换 xxl.board。
function xxl_gen_permutation()
	local old = xxl.board;
	local nb = {};
	for attempt = 1, 50 do
		local perm = {};
		for i = 0, xxl_row*xxl_col-1 do perm[i+1] = i; end
		for i = #perm, 2, -1 do
			local j = math.random(1, i);
			perm[i], perm[j] = perm[j], perm[i];
		end
		for y = 0, xxl_row-1 do nb[y] = {}; end
		for y = 0, xxl_row-1 do
			for x = 0, xxl_col-1 do
				local t = perm[y*xxl_col + x + 1];
				nb[math.floor(t / xxl_col)][t % xxl_col] = old[y][x];
			end
		end
		xxl.board = nb; -- 让 has_match/has_valid_move 在候选盘上判定
		if not xxl_board_has_match() and xxl_has_valid_move() then
			return nb, perm;
		end
	end
	return nb, perm;
end

-- 洗牌动画第一段(数值动画):浮层副本以棋子当前位置为起点、棋盘中心为圆心
-- 绕行 1.0~3.0 圈(随机方向)。不取动画器的 rect 值,只取 GetFraction 驱动圆周:
-- rect range 恒等(from==to),角度 = ang0 + angDelta * fraction。
-- 🚨 包装 ani 必须存进 ctx 保活(同 xxl_begin_move,GC 摘监听则回调全失)。
-- 失败(建副本/启动失败)返回 nil,由调用方计入直接完成数。
-- tPos/tState 随 ctx 传递:第二段飞完后由 xxl_shuffle_arrive 立即落位显示。
function xxl_begin_shuffle_rot(aniframe, state, ele, rcFrom, rcTo, cx, cy, tPos)
	local ani_widget = xxl_build_ani_widget(aniframe, state);
	if ani_widget == nil then
		xxl_slog("shuffle_rot: build ani widget failed");
		return nil;
	end
	local w = rcFrom:Width(); local h = rcFrom:Height();
	local dx = rcFrom.left + w/2 - cx;
	local dy = rcFrom.top + h/2 - cy;
	local turns = 1.0 + math.random()*2.0;             -- 1.0~3.0 圈(可 2.5)
	local dir = math.random() < 0.5 and -1 or 1;       -- 随机旋转方向
	local c = {
		ani_widget = ani_widget,
		cx = cx, cy = cy,
		radius = math.sqrt(dx*dx + dy*dy),
		ang0 = math.atan(dy, dx),
		angDelta = dir * turns * 2 * math.pi,
		w = w, h = h,
		rcTo = rcTo,
		tPos = tPos,
		tState = state,
		rcEnd = nil,
	};
	c.ani_widget:Move(CRect(rcFrom.left, rcFrom.top, rcFrom.right, rcFrom.bottom));
	local ani = LuaValueAnimator();
	ani:CopyFrom(xxl.ani_move:GetIValueAnimator());
	-- 旋转时长与圈数成正比(约 400ms/圈):同速旋转,圈数多的转得更久
	ani:GetIValueAnimator():setDuration(math.floor(turns * 400));
	ani:SetRangeRect(rcFrom, rcFrom); -- rect 值恒定不用,仅借 fraction 驱动圆周
	ani:SetOnUpdate("xxl_shuffle_rot_update");
	ani:SetOnEnd("xxl_shuffle_rot_end");
	local ctxId = xxl_new_ctx("shuf_rot", c);
	c.ani = ani;
	ani:SetCtx(ctxId);
	-- 🚨 lua_tinker 的 BOOL 返回是数字,用 ==0 判(勿用 not)
	local okStart = ani:Start(aniframe);
	if okStart == 0 or okStart == false then
		xxl.ani_ctx[ctxId] = nil;
		ani_widget:Destroy();
		return nil;
	end
	return ani;
end

-- 旋转跟随:按插值后的 fraction 算圆周位置,把副本挪过去;末帧位置存 rcEnd
function xxl_shuffle_rot_update(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil or c.ani_widget == nil then return end
	local a = c.ang0 + c.angDelta * luaAni:GetFraction();
	local px = c.cx + math.cos(a) * c.radius - c.w/2;
	local py = c.cy + math.sin(a) * c.radius - c.h/2;
	local rc = CRect(math.floor(px), math.floor(py), math.floor(px)+c.w, math.floor(py)+c.h);
	c.rcEnd = rc;
	c.ani_widget:Move(rc);
end

-- 旋转结束 => 第二段:从轨道终点(副本当前所在位置)直线飞往目标棋格
-- (SAnimatorGroup 不适合此"每枚棋子旋转完立即接力"的逐个衔接,直接链式启动)
function xxl_shuffle_rot_end(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil then return end
	xxl.ani_ctx[ctxId] = nil;
	local rcFrom = c.rcEnd;
	if rcFrom == nil then
		-- 一次 update 都没来(异常)也按轨道终点=fraction 1 的位置接力
		local a = c.ang0 + c.angDelta;
		local px = c.cx + math.cos(a)*c.radius - c.w/2;
		local py = c.cy + math.sin(a)*c.radius - c.h/2;
		rcFrom = CRect(math.floor(px), math.floor(py), math.floor(px)+c.w, math.floor(py)+c.h);
	end
	local ani2 = LuaValueAnimator();
	ani2:CopyFrom(xxl.ani_move:GetIValueAnimator());
	ani2:GetIValueAnimator():setDuration(250);
	ani2:SetRangeRect(rcFrom, c.rcTo);
	ani2:SetOnUpdate("xxl_ani_update");
	ani2:SetOnEnd("xxl_shuffle_move_end");
	local ctxId2 = xxl_new_ctx("shuf_move", { ani_widget=c.ani_widget, ani=ani2, tPos=c.tPos, tState=c.tState });
	ani2:SetCtx(ctxId2);
	-- 🚨 lua_tinker 的 BOOL 返回是数字,用 ==0 判(勿用 not)
	local okStart = ani2:Start(xxl.aniframe);
	if okStart == 0 or okStart == false then
		xxl.ani_ctx[ctxId2] = nil;
		c.ani_widget:Destroy();
		xxl_shuffle_arrive(c.tPos, c.tState); -- 启动失败也按到达落位,不让目标格空着
		xxl_shuffle_piece_done();
		return
	end
end

-- 飞行结束:销毁副本,立即在目标格落位显示该棋子(不等全盘归位);
-- 新盘数据已在置换确定时提交,xxl_on_grid_changed 读到的即该棋子状态
function xxl_shuffle_move_end(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil then return end
	if c.ani_widget then c.ani_widget:Destroy(); end
	xxl.ani_ctx[ctxId] = nil;
	xxl_shuffle_arrive(c.tPos, c.tState);
	xxl_shuffle_piece_done();
end

-- 棋子落位:恢复目标格可见并翻页显示(置换单射 => 每格恰好被落位一次)
function xxl_shuffle_arrive(tPos, tState)
	if tPos == nil then return end
	local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(tPos),-1);
	if ele == nil then return end
	ele:SetVisible(true,true); -- 恢复可见必须补失效(动画期间无副本覆盖此格)
	xxl_on_grid_changed(tPos, false);
end

function xxl_shuffle_piece_done()
	if xxl.shuffle_pending == nil then return end
	xxl.shuffle_pending = xxl.shuffle_pending - 1;
	if xxl.shuffle_pending <= 0 then
		xxl.shuffle_pending = nil;
		xxl_shuffle_finalize();
	end
end

-- 全部棋子归位:逐格兜底恢复可见/显示(正常路径已被逐个到达落位接管,
-- 只补动画失败棋子的目标格),ani_count 归零后再检查可消除行列
function xxl_shuffle_finalize()
	local nb = xxl.shuffle_newboard;
	xxl.shuffle_newboard = nil;
	if nb == nil then return end
	xxl.ani_count = xxl.ani_count - 1;
	if xxl.ani_count < 0 then xxl.ani_count = 0 end
	xxl.wndBoard:EnableWindow(1, 1); -- 洗牌结束恢复棋盘交互
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			local ele = xxl.wndBoard:FindChildByID(xxl_pos2id({x=x,y=y}),-1);
			if ele then
				ele:SetVisible(true,true); -- 恢复可见必须补失效(动画期间无副本覆盖)
				xxl_on_grid_changed({x=x,y=y}, false);
			end
		end
	end
	xxl_check_board();
end

-- 死局洗牌:现有棋子位置置换 + 全盘两段式洗牌动画,全部归位后才检查消除
function xxl_redeal()
	xxl.click_id = -1;
	local oldBoard = xxl.board;
	local nb, perm = xxl_gen_permutation();
	-- 新盘数据随置换即时提交(数据先行):飞行期间被 ani_count 门禁无读取方,
	-- 棋子到达时 xxl_on_grid_changed(tPos) 读到的即该棋子状态,可立即落位显示
	xxl.shuffle_newboard = nb;
	-- 棋盘中心(宿主坐标系:GetWindowRect2 全树共享宿主坐标,直接可用)
	local rcBoard = xxl.wndBoard:GetWindowRect2();
	local cx = rcBoard.left + rcBoard:Width()/2;
	local cy = rcBoard.top + rcBoard:Height()/2;
	xxl.ani_count = xxl.ani_count + 1; -- 整个洗牌链算一条动画链,期间禁止点击
	-- 洗牌期间禁用棋盘防误操作:点击经 hover 链路派发,IsDisabled(TRUE) 查父链,
	-- 禁用棋盘窗口即可挡掉全部格子交互(浮层副本是兄弟窗口,不受影响);
	-- 恢复入口:xxl_shuffle_finalize(正常)与 xxl_init_board(中途重开兜底)
	xxl.wndBoard:EnableWindow(0, 1);
	local total = 0; local nFail = 0;
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			local ele = xxl.wndBoard:FindChildByID(xxl_pos2id({x=x,y=y}),-1);
			if ele == nil then
				nFail = nFail + 1; -- 缺格:无动画可播,finalize 时一并恢复
			else
				total = total + 1;
				ele:SetVisible(false,false);
				local tIdx = perm[y*xxl_col + x + 1]; -- 该棋子的目标格
				local tPos = { x = tIdx % xxl_col, y = math.floor(tIdx / xxl_col) };
				local rcTo = xxl_get_ele_rect(tPos);
				local ani = xxl_begin_shuffle_rot(xxl.aniframe, oldBoard[y][x], ele, ele:GetWindowRect2(), rcTo, cx, cy, tPos);
				if ani == nil then nFail = nFail + 1 end
			end
		end
	end
	xxl.shuffle_pending = total - nFail;
	if xxl.shuffle_pending <= 0 then
		xxl.shuffle_pending = nil;
		xxl_shuffle_finalize();
	end
end

-- 提示:找一个可行交换,两枚棋子做选中脉冲动画(复用选中动画,点击即被替换)
function xxl_on_hint(args)
	-- 提示按钮上播放青色冰环特效(与重开按钮的金色星芒区分)
	local btn = xxl.root:FindChildByNameA("btn_xxl_hint",-1);
	if btn then
		xxl_pop_fx(btn:GetWindowRect2(), false, "skin_xxl_fx2", "anim:xxl_fx_ring");
	end
	if xxl.wndBoard == nil or xxl.coin <= 0 then return 0 end
	if xxl.ani_count ~= 0 then return 0 end -- 动画进行中不给提示
	local m = xxl_find_move();
	if m == nil then return 0 end
	xxl.hint_ids = { xxl_pos2id(m.p1), xxl_pos2id(m.p2) };
	for _,p in ipairs({m.p1, m.p2}) do
		local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(p),-1);
		if ele and xxl.ani_sel then
			local ani = xxl.ani_sel:clone();
			ele:SetAnimation(ani);
			ani:Release();
		end
	end
	return 1;
end

-- 清掉上次提示的脉冲动画(下次任意棋盘点击时调用)
function xxl_clear_hint()
	if xxl.hint_ids == nil then return end
	for _,hid in ipairs(xxl.hint_ids) do
		local he = xxl.wndBoard:FindChildByID(hid,-1);
		if he then he:ClearAnimation(); end
	end
	xxl.hint_ids = {};
end

-- js buildAniWidget: 在浮层上创建与格子同状态的副本。
-- 副本 id 全局唯一(与棋盘 id 空间隔离),见文件头"刻意偏离 1"。
-- 🚨 创建后必须 SelectPage(state):模板 curSel="0" 默认停在 0 号棋子,
--    漏掉会让下沉/交换期间所有副本显示同一枚棋子(js 同款步骤)。
function xxl_build_ani_widget(aniframe, state)
	xxl.copy_seq = xxl.copy_seq + 1;
	local cid = xxl_base_id + 100000 + xxl.copy_seq;
	local xml = "<t:g.xxl_ele><data id=\"" .. cid .. "\"/></t:g.xxl_ele>";
	aniframe:CreateChildrenFromXml(xml);
	local ele = aniframe:FindChildByID(cid,-1);
	if ele and state then
		local stackApi = QiIStackView(ele);
		if stackApi then
			stackApi:SelectPage(state,false);
			stackApi:Release();
		end
	end
	return ele;
end

-- js 内联的 new SValueAnimator + CopyFrom + SetRange + 回调 + Start:
-- 隐藏原格子,浮层副本从 rcFrom 移到 rcTo。失败时回滚格子可见性(见文件头"刻意偏离 3")。
-- 🚨 包装对象 ani 必须存进 ctx 保活:它把自己注册为 IValueAnimator 的
--    update/end 监听器,而 C++ 动画器只 AddRef 了 IValueAnimator 本体,不持有
--    包装;若包装被 GC(~LuaValueAnimator→Detach 摘监听),onUpdate/onEnd 不再
--    回调 => 副本冻结在浮层上冒充棋子(该消除的还在显示)、原格子永久隐藏
--    (不能点击)。存入 ctx 直到 ani_end 清掉后才允许回收。
function xxl_begin_move(aniframe, state, ele, rcFrom, rcTo)
	ele:SetVisible(false,false);
	local ani_widget = xxl_build_ani_widget(aniframe, state);
	if ani_widget == nil then
		ele:SetVisible(true,true); -- 失败回滚:恢复可见必须补失效(无副本覆盖此区域)
		xxl_slog("begin_move: build ani widget failed");
		return nil;
	end
	local ani = LuaValueAnimator();
	ani:CopyFrom(xxl.ani_move:GetIValueAnimator());
	ani:SetRangeRect(rcFrom, rcTo);
	ani:SetOnUpdate("xxl_ani_update");
	ani:SetOnEnd("xxl_ani_end");
	local ctxId = xxl_new_ctx("move", { ele=ele, ani_widget=ani_widget, ani=ani });
	ani:SetCtx(ctxId);
	-- SAnimatorGroup 只聚合回调,不启动子动画 => 必须逐个 Start(js 同款)
	ani:Start(aniframe);
	return ani;
end

-- js onAnimationUpdate: 副本跟随动画值移动
function xxl_ani_update(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c and c.ani_widget then
		c.ani_widget:Move(luaAni:GetRectValue());
	end
end

-- js onAnimationEnd: 销毁副本,恢复原格子可见
function xxl_ani_end(luaAni, ctxId)
	local c = xxl.ani_ctx[ctxId];
	if c == nil then
		xxl_slog("ani_end: ctx " .. ctxId .. " already gone (anomaly)");
		return
	end
	if c.ani_widget then c.ani_widget:Destroy(); end
	-- 恢复原格子可见(带失效)
	c.ele:SetVisible(true,true);
	xxl.ani_ctx[ctxId] = nil;
end

-- 浮层常驻可见(msgTransparent=1 只显示不挡交互,见 page_script.xml),
-- 不再随动画启停切换可见性;特效窗口也因此不受浮层显隐影响
function xxl_check_aniframe()
end

-- 动画组结束分发(js onAnimatorGroupEnd / 2 / 3 / Y2 / Y3)
function xxl_group_end(group, ctxId, nID)
	-- 🚨 不能用 v == group 摘除:C++ 回调把组指针重新 push 成新 userdata,
	-- lua 的 == 是 userdata 裸身份比较,恒为 false => ani_list 只增不减。
	-- ctxId 是组创建时登记的键,回调原样带回,按它摘除才可靠。
	if ctxId and xxl.ani_list[ctxId] then
		xxl.ani_list[ctxId] = nil;
		xxl.ani_count = xxl.ani_count - 1;
	end
	local g = xxl.ani_ctx[ctxId];
	if g == nil then
		xxl_slog("group_end: ctx " .. tostring(ctxId) .. " gone (anomaly), ani_count=" .. xxl.ani_count);
		xxl_check_aniframe();
		return
	end
	xxl.ani_ctx[ctxId] = nil;
	xxl_slog("group_end kind=" .. g.kind .. " ctx=" .. ctxId .. " left=" .. xxl.ani_count);
	if g.kind == "swap" then
		xxl_on_swap_end(g);
	elseif g.kind == "clear" then
		xxl_on_clear_end(g);
	elseif g.kind == "drop" then
		xxl_on_drop_end(g);
	elseif g.kind == "samey" then
		xxl_on_clear_end_y(g);
	elseif g.kind == "drop_y" then
		xxl_on_drop_end_y(g);
	end
	xxl_check_aniframe();
end

-- js onAnimatorGroupEnd: 交换提交(不消除也不回退),扣金币
function xxl_on_swap_end(g)
	xxl_swap(g.pos1, g.pos2);
	xxl.coin = xxl.coin - 1;
	xxl_show_coin();
end

-- js onAnimatorGroupEnd2: 消除动画结束,启动上方元素下落(每列被清格子上方的格子各落一格)
function xxl_on_clear_end(g)
	local samex = g.samex;
	-- 金币入账链 ① 特效段:消除瞬间在消除中心播星芒,播完飞金币区再滚动计数
	-- (数据层在此即时 +1;显示层等 ③ 计数段才滚)
	xxl.coin = xxl.coin + 1;
	xxl_coin_fx(xxl_get_ele_rect({x=samex.x + math.floor((samex.len-1)/2), y=samex.y}), samex.len >= 4);
	if samex.y > 0 then
		local ctxId = xxl_new_ctx("drop", { samex=samex });
		local group = LuaAnimatorGroup();
		group:SetOnGroupEnd("xxl_group_end");
		group:SetCtx(ctxId);
		local nAdded = 0;
		for i = 1, #g.posLst do
			local posClear = g.posLst[i];
			for j = 0, posClear.y - 1 do
				local pos = { x=posClear.x, y=j };
				local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
				if ele then
					local rc1 = ele:GetWindowRect2();
					local rc2 = CRect(rc1.left,rc1.top,rc1.right,rc1.bottom);
					rc2:OffsetRect(0, rc2:Height());
					local ani = xxl_begin_move(xxl.aniframe, xxl.board[pos.y][pos.x], ele, rc1, rc2);
					if ani then
						group:AddAnimator(ani:GetIValueAnimator());
						nAdded = nAdded + 1;
					end
				end
			end
		end
		if nAdded == 0 then
			xxl.ani_ctx[ctxId] = nil;
			xxl_on_drop_end(g);
			return
		end
		xxl.ani_count = xxl.ani_count + 1;
		xxl.ani_list[ctxId] = group;
	else
		xxl_on_drop_end(g);
	end
end

-- js onAnimatorGroupEnd3: 下沉结束,计分(连击倍乘)并补位
-- (金币已在 clear_end 的入账链 ① 里 +1,这里只管得分与补位)
function xxl_on_drop_end(g)
	local samex = g.samex;
	xxl.score = xxl.score + samex.len * xxl.combo;
	xxl_show_score();
	xxl_free_same_x(samex.y, samex.x, samex.len);
end

function xxl_get_ele_rect(pos)
	local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
	return ele:GetWindowRect2();
end

-- 消除爆发特效(参考 cnchess CChessGame::ShowGameFx + sprite_fx_*/kfx_pop 模式):
-- 每次在浮层上动态创建一个独立特效窗口播放 clone 出来的 anim:xxl_fx_pop,
-- 动画结束由 on_animation_stop 事件(xxw_fx_stop)自动销毁 => 支持多处特效并发。
xxl_fx_base_id = 250000; -- 特效窗口 id 空间(棋盘 30000+,浮层副本 130000+,互不重叠)

-- 特效动画播完(SOUI EventSwndAnimationStop) => 销毁特效窗口
function xxw_fx_stop(args)
	local fx = toSWindow(args:Sender());
	if fx then fx:Destroy(); end
end

-- 在 rc 中心播放一次特效;boost=true 时放大(大消除)。
-- szSkin/szAnim 可指定不同外观与动画:重开按钮=金星芒(skin_xxl_fx/anim:xxl_fx_pop),
-- 提示按钮=青色冰环(skin_xxl_fx2/anim:xxl_fx_ring);缺省即游戏内消除用的金星芒。
-- szStopFun:动画播完(on_animation_stop)回调,缺省 xxw_fx_stop=自毁;
-- 金币入账链传 xxl_fx_fly_start(特效播完转飞行段,见 xxl_coin_fx)。
-- 返回特效窗口(失败返回 nil),供调用方接管。
-- 坐标模型:SOUI4 全树共享宿主窗口坐标系(Swnd.cpp DispatchPaint 无逐级平移,
-- GetWindowRect2 一律相对宿主)——格子/按钮的 rect 直接可用,无需任何换算。
-- 定位用显式 Move(浮动模式,立即生效):pos 属性要等下一次 relayout 才生效,
-- 上一版靠 pos 定位导致特效时有时无/错位。
function xxl_pop_fx(rc, boost, szSkin, szAnim, fScale, szStopFun)
	if xxl.aniframe == nil then
		xxl_slog("pop_fx SKIP: aniframe nil");
		return nil
	end
	local skin = szSkin or "skin_xxl_fx";
	local key = szAnim or "anim:xxl_fx_pop";
	local stopFun = szStopFun or "xxw_fx_stop";
	local aniCache = xxl.fx_ani[key];
	if aniCache == nil then
		aniCache = GetApp():LoadAnimation(key);
		if aniCache == nil then
			xxl_slog("pop_fx SKIP: LoadAnimation failed: " .. key);
			return
		end
		xxl.fx_ani[key] = aniCache;
	end
	xxl.fx_seq = xxl.fx_seq + 1;
	local fid = xxl_fx_base_id + xxl.fx_seq;
	-- fScale:显式缩放(可选);缺省 1.5,boost 再 +0.5
	local scale = fScale or (1.5 + (boost and 0.5 or 0));
	local w = math.floor(rc:Width()*scale); local h = math.floor(rc:Height()*scale);
	local px = rc.left + math.floor(rc:Width()/2);
	local py = rc.top + math.floor(rc:Height()/2);
	local xml = '<img id="' .. fid .. '" skin="' .. skin .. '"'
		.. ' msgTransparent="1" visible="0" on_animation_stop="' .. stopFun .. '"/>';
	xxl.aniframe:CreateChildrenFromXml(xml);
	local fx = xxl.aniframe:FindChildByID(fid,-1);
	if fx == nil then
		xxl_slog("pop_fx FAIL: fx window not created, id=" .. fid);
		return
	end
	fx:Move(CRect(px - math.floor(w/2), py - math.floor(h/2), px - math.floor(w/2) + w, py - math.floor(h/2) + h));
	-- cnchess 同款:clone 缓存动画再挂上,SetAnimation 后下一帧自动启动
	-- 缓存的 master 绝不能直接交给窗口:窗口 stop 时会 Release 它,缓存里会成悬空指针
	local ani = aniCache:clone();
	fx:SetAnimation(ani);
	ani:Release();
	-- 显示必须在 SetAnimation 之后:替换运行中动画可能触发旧动画 stop 事件
	fx:SetVisible(true,true);
	return fx;
end

-- js onGetSameX: 横向 3 连聚拢到中心
function xxl_on_get_same_x(y,x,len)
	xxl.aniframe:SetVisible(true,true);
	xxl.combo = xxl.combo + 1;
	xxl_show_combo();
	local ctxId = xxl_new_ctx("clear", { samex={y=y,x=x,len=len}, posLst={} });
	local group = LuaAnimatorGroup();
	group:SetOnGroupEnd("xxl_group_end");
	group:SetCtx(ctxId);
	local state = xxl.board[y][x];
	local rcStart = xxl_get_ele_rect({x=x,y=y});
	local rcEnd = xxl_get_ele_rect({x=x+len-1,y=y});
	local rcCenter = CRect(rcStart.left,rcStart.top,rcStart.right,rcStart.bottom);
	rcCenter:OffsetRect((rcEnd.right-rcStart.right)/2, 0);
	local nAdded = 0;
	for i = 0, len-1 do
		local pos = {x=x+i, y=y};
		local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
		if ele then
			local ani = xxl_begin_move(xxl.aniframe, state, ele, ele:GetWindowRect2(), rcCenter);
			if ani then
				group:AddAnimator(ani:GetIValueAnimator());
				nAdded = nAdded + 1;
				table.insert(xxl.ani_ctx[ctxId].posLst, pos);
			end
		end
	end
	if nAdded == 0 then
		xxl.ani_ctx[ctxId] = nil;
		xxl_on_clear_end({samex={y=y,x=x,len=len}, posLst={}});
		return
	end
	xxl.ani_count = xxl.ani_count + 1;
	xxl.ani_list[ctxId] = group;
end

-- js onGetSameY: 纵向 3 连聚拢到中心
function xxl_on_get_same_y(x,y,len)
	xxl.aniframe:SetVisible(true,true);
	xxl.combo = xxl.combo + 1;
	xxl_show_combo();
	local ctxId = xxl_new_ctx("samey", { samey={x=x,y=y,len=len}, posLst={} });
	local group = LuaAnimatorGroup();
	group:SetOnGroupEnd("xxl_group_end");
	group:SetCtx(ctxId);
	local state = xxl.board[y][x];
	local rcStart = xxl_get_ele_rect({x=x,y=y});
	local rcEnd = xxl_get_ele_rect({x=x,y=y+len-1});
	local rcCenter = CRect(rcStart.left,rcStart.top,rcStart.right,rcStart.bottom);
	rcCenter:OffsetRect(0, (rcEnd.bottom-rcStart.bottom)/2);
	local nAdded = 0;
	for i = 0, len-1 do
		local pos = {x=x, y=y+i};
		local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
		if ele then
			local ani = xxl_begin_move(xxl.aniframe, state, ele, ele:GetWindowRect2(), rcCenter);
			if ani then
				group:AddAnimator(ani:GetIValueAnimator());
				nAdded = nAdded + 1;
				table.insert(xxl.ani_ctx[ctxId].posLst, pos);
			end
		end
	end
	if nAdded == 0 then
		xxl.ani_ctx[ctxId] = nil;
		xxl_on_clear_end_y({samey={x=x,y=y,len=len}, posLst={}});
		return
	end
	xxl.ani_count = xxl.ani_count + 1;
	xxl.ani_list[ctxId] = group;
end

-- js onAnimatorGroupEndY2: 纵向消除结束,上方格子整体下移 len 格
function xxl_on_clear_end_y(g)
	local samey = g.samey;
	-- 金币入账链 ① 特效段(横向/纵向同款,len>=4 放大)
	xxl.coin = xxl.coin + 1;
	xxl_coin_fx(xxl_get_ele_rect({x=samey.x, y=samey.y + math.floor((samey.len-1)/2)}), samey.len >= 4);
	if samey.y > 0 then
		local ctxId = xxl_new_ctx("drop_y", { samey=samey });
		local group = LuaAnimatorGroup();
		group:SetOnGroupEnd("xxl_group_end");
		group:SetCtx(ctxId);
		local nAdded = 0;
		for j = 0, samey.y - 1 do
			local pos = { x=samey.x, y=j };
			local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
			if ele then
				local rc1 = ele:GetWindowRect2();
				local rc2 = CRect(rc1.left,rc1.top,rc1.right,rc1.bottom);
				rc2:OffsetRect(0, rc2:Height()*samey.len);
				local ani = xxl_begin_move(xxl.aniframe, xxl.board[pos.y][pos.x], ele, rc1, rc2);
				if ani then
					group:AddAnimator(ani:GetIValueAnimator());
					nAdded = nAdded + 1;
				end
			end
		end
		if nAdded == 0 then
			xxl.ani_ctx[ctxId] = nil;
			xxl_on_drop_end_y(g);
			return
		end
		xxl.ani_count = xxl.ani_count + 1;
		xxl.ani_list[ctxId] = group;
	else
		xxl_on_drop_end_y(g);
	end
end

-- js onAnimatorGroupEndY3: 下沉结束,计分(连击倍乘)并补位
-- (金币已在 clear_end_y 的入账链 ① 里 +1,这里只管得分与补位)
function xxl_on_drop_end_y(g)
	local samey = g.samey;
	xxl.score = xxl.score + samey.len * xxl.combo;
	xxl_show_score();
	xxl_free_same_y(samey.x, samey.y, samey.len);
end

-- js onCmd 的核心逻辑(拆出来便于自测走与玩家完全相同的路径)
function xxl_on_click(idFrom, eleSender)
	if xxl.coin <= 0 then return 0 end
	if idFrom < xxl_base_id or idFrom >= xxl_base_id + xxl_row*xxl_col then
		return 0;
	end
	if xxl.ani_count ~= 0 then
		-- 🚨 动画链进行中禁止新点击:交换/消除/下沉各链都在组结束回调里才
		-- 提交数据(xxl_swap)与补位(free_same_x/y),并发链捕获的 pos/board 是
		-- 中间态,前后链互相踩踏后数据与显示分叉且无法自愈。留日志取证。
		xxl_slog("on_click BUSY drop id=" .. idFrom .. " ani_count=" .. xxl.ani_count);
		return 0;
	end
	xxl_clear_hint(); -- 任意棋盘点击都收掉提示脉冲
	if xxl.click_id ~= -1 then
		-- 先清掉旧选中格子的缩放动画(js: ele.ClearAnimation())
		local eleOld = xxl.wndBoard:FindChildByID(xxl.click_id,-1);
		if eleOld then eleOld:ClearAnimation(); end
		local pos1 = xxl_id2pos(xxl.click_id);
		local pos2 = xxl_id2pos(idFrom);
		if xxl_can_swap(pos1,pos2) then
			-- 交换动画(js onCmd canSwap 分支:两副本交叉飞行)
			xxl.combo = 0; -- 新一次交换,连击重新计
			xxl_show_combo();
			xxl.aniframe:SetVisible(true,true);
			local id1 = xxl.click_id;
			local id2 = idFrom;
			local ele1 = xxl.wndBoard:FindChildByID(id1,-1);
			local ele2 = eleSender or xxl.wndBoard:FindChildByID(id2,-1);
			xxl.click_id = -1;
			if ele1 == nil or ele2 == nil then return 0 end
			local rc1 = ele1:GetWindowRect2();
			local rc2 = ele2:GetWindowRect2();
			local ctxId = xxl_new_ctx("swap", { pos1=pos1, pos2=pos2 });
			local group = LuaAnimatorGroup();
			group:SetOnGroupEnd("xxl_group_end");
			group:SetCtx(ctxId);
			local nAdded = 0;
			local ani1 = xxl_begin_move(xxl.aniframe, xxl.board[pos1.y][pos1.x], ele1, rc1, rc2);
			if ani1 then
				group:AddAnimator(ani1:GetIValueAnimator());
				nAdded = nAdded + 1;
			end
			local ani2 = xxl_begin_move(xxl.aniframe, xxl.board[pos2.y][pos2.x], ele2, rc2, rc1);
			if ani2 then
				group:AddAnimator(ani2:GetIValueAnimator());
				nAdded = nAdded + 1;
			end
			if nAdded == 0 then
				xxl.ani_ctx[ctxId] = nil;
				-- 回滚:两格恢复可见,补失效(此分支无动画副本覆盖)
				ele1:SetVisible(true,true);
				ele2:SetVisible(true,true);
				return 0;
			end
			xxl.ani_count = xxl.ani_count + 1;
			xxl.ani_list[ctxId] = group;
		else
			-- 不可交换:取消当前选中,按新点击处理(js: click_id=-1; this.onCmd(e))
			xxl.click_id = -1;
			xxl_on_click(idFrom, eleSender);
		end
	else
		-- 首次点击:选中标记动画(js: ani_sel.clone + SetAnimation)
		local ele = eleSender or xxl.wndBoard:FindChildByID(idFrom,-1);
		if ele == nil then return 0 end
		if xxl.ani_sel then
			local ani = xxl.ani_sel:clone();
			ele:SetAnimation(ani);
			ani:Release();
		end
		xxl.click_id = idFrom;
	end
	return 1;
end

-- 格子点击入口(模板 on_command="xxl_on_cmd")
function xxl_on_cmd(args)
	return xxl_on_click(args:IdFrom(), toSWindow(args:Sender()));
end

-- 重新开始(按钮与游戏结束弹窗共用)
function xxl_restart_internal()
	xxl.coin = 20; xxl.score = 0;
	xxl.combo = 0; xxl.gameover = false;
	xxl_show_coin();
	xxl_show_score();
	xxl_show_combo();
	xxl_init_board();
end

-- 重新开始按钮:先复位棋盘,再在按钮上播放金色星芒爆发特效
-- (按钮 rect 与棋盘格子同处宿主坐标系,直接传给 pop_fx,无需换算)
function xxl_on_restart(args)
	if xxl.wndBoard == nil then return 0 end
	xxl_restart_internal();
	local btn = xxl.root:FindChildByNameA("btn_xxl_restart",-1);
	if btn then
		xxl_pop_fx(btn:GetWindowRect2(), true);
	end
	return 1;
end

-- js initBoard + init 的全盘 SelectPage 循环
function xxl_init_board()
	-- 先丢弃所有动画上下文:格子即将销毁,存活动画回调(ctx=nil)自动变 no-op。
	-- 🚨 存活的动画组包装挪入 orphan 表保活到下次重建:LuaAnimatorGroup 是子
	-- 动画器的监听者(SAnimatorGroup::AddAnimator 存裸指针),若让它被 GC 而子
	-- 动画器还在跑,子动画器结束回调会踩悬挂指针;orphan 表保证组活得比子长。
	xxl.ani_orphan = xxl.ani_list;
	xxl.ani_ctx = {};
	xxl.ani_list = {};
	xxl.ani_count = 0;
	-- 洗牌中途重开的兜底:洗牌禁用了棋盘而 finalize 不会再来,这里必须恢复交互
	xxl.wndBoard:EnableWindow(1, 1);
	xxl.wndBoard:DestroyAllChildren();
	-- 浮层只承载动态创建的副本/特效窗口(见 page_script.xml),一并清空:
	-- restart 时可能有副本正在飞行,其 ani_end 因 ctx 被抹掉而 no-op,不清掉
	-- 就会永远停在浮层上冒充棋子("该消除的还在显示")且 msgTransparent 不挡点击。
	xxl.aniframe:DestroyAllChildren();
	xxl.wndBoard:SetAttribute(T"columnCount", T"8", false);
	local xml = "";
	local eles = xxl_row * xxl_col;
	for i = 0, eles-1 do
		xml = xml .. "<t:g.xxl_ele><data id=\"" .. (xxl_base_id+i) .. "\"/></t:g.xxl_ele>";
	end
	xxl.wndBoard:CreateChildrenFromXml(xml);
	-- 棋盘格底色(市场消消乐的格子感):按 (x+y) 奇偶交替
	for i = 0, eles-1 do
		local ele = xxl.wndBoard:FindChildByID(xxl_base_id+i,-1);
		if ele then
			local par = ((i % xxl_col) + math.floor(i / xxl_col)) % 2;
			if par == 0 then
				ele:SetAttribute(T"colorBkgnd", T"rgba(255,255,255,55)", false);
			else
				ele:SetAttribute(T"colorBkgnd", T"rgba(0,0,0,16)", false);
			end
		end
	end
	xxl.wndBoard:RequestRelayout();
	-- 受约束随机:无初始 3 连且保证有解(旧版裸随机,开局即自动消,送分且棋盘缺格)
	xxl_gen_board();
	-- 逐格 SelectPage 显示当前状态(soxxl init() 同款循环)
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			xxl_on_grid_changed({x=x,y=y}, false);
		end
	end
	xxl.click_id = -1;
	xxl_check_board();
end

function xxl_exit()
	-- 释放 pop_fx 懒加载的特效动画缓存(每项引用计数 1,脚本持有)
	for _, aniCache in pairs(xxl.fx_ani) do
		aniCache:Release();
	end
	xxl.fx_ani = {};
	xxl.ani_sel:Release();
end
-- lua 构造器 + init
function xxl_init(root)
	local wndBoard = root:FindChildByNameA("wnd_xxl_board",-1);
	local aniframe = root:FindChildByNameA("wnd_xxl_aniframe",-1);
	if wndBoard == nil or aniframe == nil then
		return 0; -- demo 不含消消乐子页,跳过
	end
	xxl.root = root;
	xxl.wndBoard = wndBoard;
	xxl.aniframe = aniframe;
	xxl.ani_move = LuaValueAnimator();
	if not xxl.ani_move:LoadAnimator("animator:xxl_move") then
		xxl_slog("init: load animator:xxl_move failed");
	end
	xxl.ani_sel = GetApp():LoadAnimation("anim:xxl_scale_select");
	-- 特效动画由 xxl_pop_fx 按 key 懒加载(xxl.fx_ani 缓存)
	-- 统一走 restart 复位:coin/score/combo/gameover 清零 + 受约束发牌建盘
	-- (否则上一局 gameover=true 残留,重进页面金币 0 且不再弹结束框)
	xxl_restart_internal();
	xxl_slog("init done");
	return 1;
end

-- ============ 稳定态自检(常驻,成功时零输出) ============
-- 无 3 连且无动画在跑 => 棋盘稳定:校验所有格子存在、可见、数据完整。
-- 直接盯住"棋子缺失/消失不恢复"类问题,异常时打日志定位。
-- 稳定后再做市场玩法收尾:连击清零 / 游戏结束判定 / 死局等玩家花金币洗牌。
function xxl_on_settle()
	if xxl.wndBoard == nil then return end
	if xxl.ani_count ~= 0 then return end -- 还有动画在跑,不算稳定
	for ctxId,c in pairs(xxl.ani_ctx) do
		xxl_slog("settle: orphan ani_ctx " .. ctxId);
		return
	end
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			local ele = xxl.wndBoard:FindChildByID(xxl_pos2id({x=x,y=y}),-1);
			if ele == nil then
				xxl_slog("settle FAIL: cell missing (" .. x .. "," .. y .. ")");
				return
			end
			-- BOOL 返回是数字,用 ==0 判(勿用 not)
			if ele:IsVisible(FALSE) == 0 then
				xxl_slog("settle FAIL: cell hidden id=" .. xxl_pos2id({x=x,y=y}) .. " swnd=" .. tostring(ele:GetSwnd()));
				return
			end
			if xxl.board[y][x] == nil then
				xxl_slog("settle FAIL: board data nil (" .. x .. "," .. y .. ")");
				return
			end
		end
	end
	-- 稳定收尾 1:连击链结束,清零
	if xxl.combo ~= 0 then
		xxl.combo = 0;
		xxl_show_combo();
	end
	-- 稳定收尾 2:金币耗尽 => 游戏结束(只弹一次)
	if xxl.coin <= 0 then
		xxl_game_over();
		return
	end
	-- 稳定收尾 3:死局不自动洗牌,等玩家花金币手动洗(见 xxl_on_shuffle);
	-- 金币不足洗牌费 => 死局永远解不开,直接游戏结束
	if not xxl_has_valid_move() then
		if xxl.coin < xxl_shuffle_cost then
			xxl_game_over();
			return
		end
		xxl_slog("settle: dead board, wait player shuffle (cost=" .. xxl_shuffle_cost .. ")");
		xxl_show_deadboard_tip();
	end
end

-- 游戏结束(只弹一次):弹窗展示得分,确定后重开
function xxl_game_over()
	if xxl.wndBoard == nil or xxl.gameover then return end
	xxl.gameover = true;
	xxl_slog("settle: game over, score=" .. xxl.score);
	local hwnd = xxl.wndBoard:GetHostHwnd();
	SMessageBox(hwnd, L("本局结束!最终得分 " .. xxl.score .. "\n点击确定重新开始"), L("消消乐"), 0);
	xxl_restart_internal();
end

-- 死局提示:复用连击文本位显示提示(settle 时连击已清零,不冲突;
-- 洗牌成功/重开后由 xxl_show_combo 覆盖清掉)
function xxl_show_deadboard_tip()
	local txt = xxl.root:FindChildByNameA("txt_xxl_combo",-1);
	if txt then txt:SetWindowText(T("无路可走,请洗牌!")); end
end

-- 洗牌按钮:花金币把全盘棋子受约束重排(无死局门,随时可洗);
-- 金币不足 => 不能洗(文本位提示,不扣币不结算)
function xxl_on_shuffle(args)
	if xxl.wndBoard == nil or xxl.gameover then return 0 end
	if xxl.ani_count ~= 0 then return 0 end -- 动画进行中不响应
	if xxl.coin < xxl_shuffle_cost then
		xxl_slog("shuffle: coin " .. xxl.coin .. " < cost " .. xxl_shuffle_cost .. ", denied");
		local txt = xxl.root:FindChildByNameA("txt_xxl_combo",-1);
		if txt then txt:SetWindowText(T("金币不足,不能洗牌!")); end
		return 1;
	end
	xxl.coin = xxl.coin - xxl_shuffle_cost;
	xxl_show_coin();
	xxl_slog("shuffle: pay " .. xxl_shuffle_cost .. " coin, left=" .. xxl.coin);
	xxl_redeal();
	xxl_show_combo(); -- combo==0,清掉死局提示文本
	return 1;
end

