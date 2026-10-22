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

-- E2E: verify the C++ handler registered via IScriptFactory::RegisterNativeCallHandler
-- at script load time (DemoNativeCallHandler in demo.cpp; variadic args passed
-- as a VARIANT array; 1+2+3+int(4.5) = 10)
local ok, ret = pcall(NativeCall, "cppSum", 1, 2, 3, 4.5);
slog("NativeCall cppSum ok=" .. tostring(ok) .. " ret=" .. tostring(ret));

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
	--init the lua tetris game (page_script 的"俄罗斯方块"子页)
	tetris_init(root);
	--init the lua tank game (page_script 的"坦克大战"子页)
	tank_init(root);
end

function on_exit(args)
	slog("execute script function: on_exit");
	win = getHostFromInitEvent(args);
	HostWnd_SetMsgHandler(win,"",nil);--remove msg handler
	runTimer:Release();
	xxl_exit();
	tetris_exit();
	tank_exit();
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
    1.0~3.0 圈(统一顺时针),第二段从轨道终点直线飞往目标棋格;
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
-- 绕行 1.0~3.0 圈(统一顺时针)。不取动画器的 rect 值,只取 GetFraction 驱动圆周:
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
	local dir = 1;                                     -- 统一顺时针(屏幕坐标 Y 向下,角度递增即顺时针)
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


--[[
==================== 俄罗斯方块(lua 版) ====================
参考上面消消乐的实现方式移植的经典俄罗斯方块:
  - 纯数据层(方块矩阵/旋转/碰撞/消行/下落/计分)与渲染层分离,数据层函数
    不触碰 SOUI 导出,可在裸 lua 解释器中做回归测试:
      lua -e 'slog=function() end; dofile("test.lua"); ...逻辑断言...'
  - 棋盘 10x20 与"下一块"预览 4x4 都由 tetris_init 在宿主初始化时动态创建,
    每个格子是一个 img,统一引用 svg/tetris_blocks.svg 雪碧图
    (skin.xml: svg_tetris_blocks,17 帧):
      帧 0=空格 1-7=七种方块(I青/O黄/T紫/S绿/Z红/J蓝/L橙)
      帧 8-14=对应 1-7 的幽灵块(落点预览) 15=消行闪白 16=标题图标
    渲染只改 iconIndex,渲染缓存避免每拍全量 SetAttribute;
  - 计时用 CreateSouiFactory().CreateTimer 独立 50ms 节拍(与跑马机互不影响),
    状态机:idle -> playing <-> paused,playing/clearing -> over;
    消行走"锁定 -> 闪白 300ms -> 消除下沉 -> 继续下落"流程;
  - 计分:消 1/2/3/4 行 = 100/300/500/800 × 等级,软降 +1/格,硬降 +2/格;
    每 10 行升 1 级并加快下落(800ms 起,每级 -70ms,下限 100ms);
  - 7-bag 随机(每袋洗牌 7 种)保证出块均匀;旋转带简单踢墙(水平 0/-1/1/-2/2);
  - 棋盘 focusable,SubscribeEvent(EVT_KEYDOWN=8200) 接键盘(与 btn_lrc 的
    SubscribeEvent 同一套路):← → 移动 / ↑ 旋转 / ↓ 软降 / 空格 硬降 / P 暂停;
  - 子页隐藏(on_visible_changed)自动暂停;结束弹窗确认后自动开新一局。
棋盘格 id 约定: tetris_base_id + y*10 + x(x,y 从 0 开始);预览格 id: tetris_next_base_id + r*4 + c。
]]

tetris_base_id = 60000;
tetris_next_base_id = 61000;
tetris_row = 20;
tetris_col = 10;
tetris_cell = 24;		-- 棋盘格尺寸(lua 生成棋盘 img 时使用)
tetris_next_cell = 18;	-- 预览格尺寸
tetris_tick_ms = 50;	-- 计时器节拍
tetris_flash_ms = 300;	-- 消行闪白总时长
tetris_frame_flash = 15;-- 闪白帧号

-- 七种方块的标准矩阵(I 为 4x4,O 为 2x2,其余 3x3)
tetris_shapes = {
	{ {0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0} },	-- I
	{ {1,1},{1,1} },								-- O
	{ {0,1,0},{1,1,1},{0,0,0} },					-- T
	{ {0,1,1},{1,1,0},{0,0,0} },					-- S
	{ {1,1,0},{0,1,1},{0,0,0} },					-- Z
	{ {1,0,0},{1,1,1},{0,0,0} },					-- J
	{ {0,0,1},{1,1,1},{0,0,0} },					-- L
};
-- 每行消行得分(×等级):1/2/3/4 行 = 100/300/500/800(一次锁定最多同时消 4 行)
tetris_line_scores = {100, 300, 500, 800};
-- 4 个旋转态的格子偏移:tetris_cells[kind][rot] = {cells={{x,y}...}, w=, h=}
tetris_cells = {};

tetris = {
	root=nil, wndBoard=nil, wndNext=nil, timer=nil,
	grid = {};			-- grid[y][x] = 0(空)或 1..7(方块种类,即雪碧图帧号)
	cur = nil;			-- 当前方块 {kind,rot,x,y}
	nextKind = 0;		-- 下一个方块种类
	bag = {};			-- 7-bag 随机袋
	score=0, lines=0, level=1,
	state = "idle";		-- idle/playing/clearing/paused/over
	pausedFrom = nil;	-- 暂停前的状态(clearing 暂停后恢复回 clearing)
	ms = 0;				-- 重力计时累计
	flash_ms = 0;		-- 闪白计时累计
	flash_rows = nil;	-- 待消除行
	flash_on = false;	-- 闪白开关(渲染)
	cache = {};			-- 棋盘渲染缓存(id -> iconIndex)
	nextCache = {};		-- 预览渲染缓存
	score_shown = 0;	-- 得分 LED 显示值(滚动动画中滞后于 score)
	count_ctx = 0;		-- 进行中的得分滚动动画 ctxId(0=无;新滚动重入时废弃旧段)
	over_wnd = nil;		-- 结束窗口 wnd_gameover(布局内静态窗口,init 后惰性查找)
	over_visible = false;	-- 结束窗口显示态(lua 侧维护,true=展示中/隐藏动画进行中)
	over_hiding = false;	-- 隐藏段标志:out 动画 stop 回调里据此 SetVisible(false)
	over_ani_mask_in = nil;	-- 遮罩展示动画 master(anim:tetris_over_mask_in)
	over_ani_mask_out = nil;	-- 遮罩收起动画 master(anim:tetris_over_mask_out)
	over_ani_in = nil;	-- 结束卡片展示动画 master(anim:tetris_over_card_in,clone 挂载)
	over_ani_out = nil;	-- 结束卡片收起动画 master(anim:tetris_over_card_out)
	aniframe = nil;		-- 动画浮层(参考消消乐 wnd_xxl_aniframe)
	ani_move = nil;		-- animator:xxl_move 模板(无插值器 => 线性进度)
	fx_ani_cache = nil;	-- anim:tetris_fx_beam 缓存(LoadAnimation 一次,clone 使用)
	fx_seq = 0;			-- 浮层特效窗口 id 序列
	ani_ctx = {};		-- 飞行/滚动动画上下文(id -> ctx)
	act = nil;			-- 活动方块层状态({rcs=4 格当前显示 rect},浮层绘制)
	slide_ctx = 0;		-- 进行中的平移动画 ctxId(0=无)
	snd_dir = nil;		-- 音效目录缓存(GetAppDir()/snd)
};

function tetris_slog(msg)
	slog("tetris: " .. msg);
end

-- 播放音效:程序资源目录 snd/ 下的 wav(与 uires 分离,构建时由 CMake 拷入;
-- PlaySound 为导出的异步短音效,headless 回归环境无该导出时静默跳过)
function tetris_play_snd(name)
	if GetAppDir == nil or PlaySound == nil then return end
	if tetris.snd_dir == nil then
		tetris.snd_dir = GetAppDir() .. "/snd/";
	end
	PlaySound(tetris.snd_dir .. name .. ".wav");
end

-- 矩阵顺时针旋转 90°
function tetris_rot_cw(m)
	local n = #m; local r = {};
	for y = 1, n do r[y] = {}; end
	for y = 1, n do
		for x = 1, n do
			r[x][n+1-y] = m[y][x];
		end
	end
	return r;
end

-- 提取矩阵中格子的偏移(平移到包围盒左上角)与包围盒尺寸
function tetris_cells_of(m)
	local cells = {}; local minx, miny, maxx, maxy;
	for y = 1, #m do
		for x = 1, #m do
			if m[y][x] == 1 then
				if minx == nil or x < minx then minx = x end
				if maxx == nil or x > maxx then maxx = x end
				if miny == nil or y < miny then miny = y end
				if maxy == nil or y > maxy then maxy = y end
				cells[#cells+1] = {x = x, y = y};
			end
		end
	end
	for _,c in ipairs(cells) do c.x = c.x - minx; c.y = c.y - miny; end
	return {cells = cells, w = maxx-minx+1, h = maxy-miny+1};
end

-- 预计算每个方块 4 个旋转态
function tetris_build_cells()
	for kind = 1, 7 do
		tetris_cells[kind] = {};
		local m = tetris_shapes[kind];
		for rot = 1, 4 do
			tetris_cells[kind][rot] = tetris_cells_of(m);
			m = tetris_rot_cw(m);
		end
	end
end

-- 判定 kind/rot 形状放在 (px,py) 是否合法(越界/重叠都算不行;py<0 视为顶部缓冲区,允许)
function tetris_can_place(kind, rot, px, py)
	for _,c in ipairs(tetris_cells[kind][rot].cells) do
		local gx = px + c.x; local gy = py + c.y;
		if gx < 0 or gx >= tetris_col or gy >= tetris_row then
			return false;
		end
		if gy >= 0 and tetris.grid[gy][gx] ~= 0 then
			return false;
		end
	end
	return true;
end

function tetris_try_move(dx, dy)
	local cur = tetris.cur;
	if cur == nil then return false end
	if tetris_can_place(cur.kind, cur.rot, cur.x+dx, cur.y+dy) then
		cur.x = cur.x + dx; cur.y = cur.y + dy;
		return true;
	end
	return false;
end

-- 旋转(带简单踢墙:水平偏移 0/-1/1/-2/2 依次尝试)
function tetris_try_rotate()
	local cur = tetris.cur;
	if cur == nil then return false end
	local nr = cur.rot % 4 + 1;
	local kicks = {0, -1, 1, -2, 2};
	for _,dx in ipairs(kicks) do
		if tetris_can_place(cur.kind, nr, cur.x+dx, cur.y) then
			cur.rot = nr; cur.x = cur.x + dx;
			return true;
		end
	end
	return false;
end

-- 7-bag 随机:每袋洗牌 7 种方块,保证出块分布均匀
function tetris_next_kind()
	if #tetris.bag == 0 then
		tetris.bag = {1,2,3,4,5,6,7};
		for i = 7, 2, -1 do
			local j = math.random(1, i);
			tetris.bag[i], tetris.bag[j] = tetris.bag[j], tetris.bag[i];
		end
	end
	return table.remove(tetris.bag);
end

-- 生成当前方块并取出下一个;出生点即重叠则顶满,游戏结束
function tetris_spawn()
	local kind = tetris.nextKind;
	if kind == 0 then kind = tetris_next_kind() end
	tetris.nextKind = tetris_next_kind();
	local x = math.floor((tetris_col - tetris_cells[kind][1].w)/2);
	tetris.cur = {kind = kind, rot = 1, x = x, y = 0};
	if not tetris_can_place(kind, 1, x, 0) then
		tetris.cur = nil;
		tetris_game_over();
		return;
	end
	tetris_render_next();
	tetris_active_build();	-- 新块进入活动层(浮层绘制)
end

-- 重力节拍(毫秒):随等级加速,下限 100ms
function tetris_gravity_ms()
	local ms = 800 - (tetris.level-1)*70;
	if ms < 100 then ms = 100 end
	return ms;
end

-- 锁定当前方块到棋盘并检测满行:有则进入闪白消除态,否则直接生成下一块
function tetris_lock()
	local cur = tetris.cur;
	if cur == nil then return end
	for _,c in ipairs(tetris_cells[cur.kind][cur.rot].cells) do
		local gx = cur.x + c.x; local gy = cur.y + c.y;
		if gy >= 0 then
			tetris.grid[gy][gx] = cur.kind;
		end
	end
	tetris.cur = nil;
	tetris_active_destroy();	-- 活动层随锁定交还 grid 渲染(滑入动画已表达落定,不再闪白)
	local full = {};
	for y = 0, tetris_row-1 do
		local fullRow = true;
		for x = 0, tetris_col-1 do
			if tetris.grid[y][x] == 0 then fullRow = false; break end
		end
		if fullRow then full[#full+1] = y end
	end
	if #full > 0 then
		tetris.state = "clearing";
		tetris.flash_rows = full;
		tetris.flash_ms = 0;
		tetris.flash_on = true;
		tetris_play_snd("tetris_clear");
	else
		tetris_spawn();
		tetris_play_snd("tetris_lock");
	end
	tetris_render();
end

-- 消行闪白结束后:去掉满行、上方行整体下落、计分升级、生成下一块
function tetris_collapse()
	local n = #tetris.flash_rows;
	local keep = {};
	for y = 0, tetris_row-1 do
		local isFull = false;
		for _,fy in ipairs(tetris.flash_rows) do
			if fy == y then isFull = true; break end
		end
		if not isFull then keep[#keep+1] = tetris.grid[y]; end
	end
	local newGrid = {};
	for y = 0, tetris_row-1 do
		local src = y - n;	-- 新第 y 行来自原第 y-n 行,顶部不足补空行(keep 是 1 基数组)
		if src >= 0 then
			newGrid[y] = keep[src + 1];
		else
			newGrid[y] = {};
			for x = 0, tetris_col-1 do newGrid[y][x] = 0 end
		end
	end
	tetris.grid = newGrid;
	local pts = tetris_line_scores[n] * tetris.level;	-- 消行得分(按消除前等级)
	local levelOld = tetris.level;
	tetris.score = tetris.score + pts;
	tetris.lines = tetris.lines + n;
	tetris.level = math.floor(tetris.lines / 10) + 1;
	local rowsCleared = tetris.flash_rows;
	tetris.flash_rows = nil;
	tetris.state = "playing";
	if tetris.level ~= levelOld then
		tetris_play_snd("tetris_levelup");	-- 速度升级:较长上行琶音(0.7s)
	else
		tetris_play_snd("tetris_clear");
	end
	tetris_show_lines(); tetris_show_level();
	tetris_spawn();
	tetris_render();
	-- 动画:光波/碎片已在 on_timer 闪白结束时弹出;这里飞分 + LED 滚动
	-- (headless 无浮层时 tetris_score_fly 内部直接对齐显示)
	tetris_score_fly(pts, tetris.level ~= levelOld, tetris.level, rowsCleared);
end

-- 硬降:直落到底,快速下滑动画结束后锁定
function tetris_hard_drop()
	if tetris.cur == nil then return end
	tetris_active_finish_slide();	-- 滑动动画中硬降:键盘输入立即完成动画
	local n = 0;
	while tetris_try_move(0,1) do n = n + 1 end
	tetris.score = tetris.score + n*2;
	tetris_score_count_begin();	-- LED 滚动跟随(消消乐同款,不瞬时对齐)
	if n > 0 and tetris_slide_start(tetris_hard_drop_ms, "drop") then
		return;					-- 下滑动画结束由 tetris_slide_end 接力锁定
	end
	tetris_active_build();			-- 原地硬降/无动画能力:贴齐落点直接锁定
	tetris_lock();
end

-- ==================== 动画(参考消消乐 xxl_pop_fx/xxl_coin_fly/xxl_coin_count) ====================
-- 浮层特效窗口 id 基址(与棋盘 60000/预览 61000 不冲突)
tetris_fx_base_id = 62000;
tetris_debris_n = 8;		-- 每行碎片数
tetris_debris_ms = 600;		-- 碎片飞行时长(ms)
tetris_debris_g = 900;		-- 碎片重力(px/s^2)
tetris_active_base_id = 63000;	-- 活动方块层 img id 基址(浮层上,与棋盘/预览/特效段不冲突)
tetris_slide_ms = 40;		-- 活动方块平移一格时长(ms);须小于软降 tick(50ms),
							-- 否则连续输入频繁废弃动画重起,速率波动导致顿挫
tetris_hard_drop_ms = 80;	-- 硬降整段下滑时长(ms):固定短时"砸落",不随下落格数放大

-- 新建动画上下文(消消乐 xxl_new_ctx 同款:表持有 widget/参数,回调按 id 取回)
function tetris_new_ani_ctx(ctx)
	tetris.fx_seq = tetris.fx_seq + 1;
	tetris.ani_ctx[tetris.fx_seq] = ctx;
	return tetris.fx_seq;
end

-- 格子的宿主矩形(浮层与页面根同构,GetWindowRect2 结果可直接作为浮层子窗口位置)
function tetris_get_cell_rect(x, y)
	if tetris.wndBoard == nil then return nil end
	local ele = tetris.wndBoard:FindChildByID(tetris_base_id + y*tetris_col + x, -1);
	if ele == nil then return nil end
	return ele:GetWindowRect2();
end

-- 一行格子的合并矩形(光波爆发/碎片/飞分起点)
function tetris_get_row_rect(row)
	local rcL = tetris_get_cell_rect(0, row);
	local rcR = tetris_get_cell_rect(tetris_col-1, row);
	if rcL == nil or rcR == nil then return nil end
	return CRect(rcL.left, rcL.top, rcR.right, rcR.bottom);
end

-- LED 数字串的合并矩形(prefix_0..prefix_n-1,消消乐 xxl_get_coin_rect 同款思路)
function tetris_get_led_rect(prefix, ndigits)
	local l, t, r, b;
	for i = 0, ndigits-1 do
		local d = tetris.root and tetris.root:FindChildByNameA(prefix .. "_" .. i, -1);
		if d then
			local rc = d:GetWindowRect2();
			l = (l == nil) and rc.left or math.min(l, rc.left);
			t = (t == nil) and rc.top or math.min(t, rc.top);
			r = (r == nil) and rc.right or math.max(r, rc.right);
			b = (b == nil) and rc.bottom or math.max(b, rc.bottom);
		end
	end
	if l == nil then return nil end
	return CRect(l, t, r, b);
end

-- 整行特效:光波扫行 + 方块碎片飞散(替代消消乐星芒,更贴合方块玩法)
function tetris_pop_row_fx(row)
	local rc = tetris_get_row_rect(row);
	if rc == nil then return end
	tetris_fx_beam(rc);
	tetris_fx_debris(row, rc);
end

-- 光波:skin_tetris_fx 横向光带 + anim:tetris_fx_beam(行中心向两侧爆开后淡出,stop 自毁)
function tetris_fx_beam(rc)
	if tetris.aniframe == nil then return end
	tetris.fx_seq = tetris.fx_seq + 1;
	local fid = tetris_fx_base_id + tetris.fx_seq;
	local w = math.floor(rc:Width() * 1.25); local h = math.floor(rc:Height() * 1.4);
	local px = rc.left + math.floor(rc:Width()/2);
	local py = rc.top + math.floor(rc:Height()/2);
	local xml = '<img id="' .. fid .. '" skin="skin_tetris_fx" msgTransparent="1" visible="0" on_animation_stop="tetris_fx_stop"/>';
	tetris.aniframe:CreateChildrenFromXml(xml);
	local fx = tetris.aniframe:FindChildByID(fid, -1);
	if fx == nil then return end
	fx:Move(CRect(px - math.floor(w/2), py - math.floor(h/2), px - math.floor(w/2) + w, py - math.floor(h/2) + h));
	-- cnchess 同款:clone 缓存动画再挂上(master 会在窗口 stop 时被 Release,不能直接交缓存)
	if tetris.fx_ani_cache == nil then
		tetris.fx_ani_cache = GetApp():LoadAnimation("anim:tetris_fx_beam");
	end
	if tetris.fx_ani_cache == nil then fx:Destroy(); return end
	local ani = tetris.fx_ani_cache:clone();
	fx:SetAnimation(ani);
	ani:Release();
	-- 显示必须在 SetAnimation 之后:替换运行中动画可能触发旧动画 stop 事件
	fx:SetVisible(true, true);
end

-- 方块碎片:取被消行原方块的雪碧图帧(塌陷前 grid 还在),从行内向上抛散,
-- 一行碎片共用一个 LuaValueAnimator 统一驱动(重力下坠 + 整体淡出)
function tetris_fx_debris(row, rc)
	if tetris.aniframe == nil or tetris.ani_move == nil then return end
	local cw = math.floor(rc:Width() / tetris_col);
	local sz = math.max(8, math.floor(cw * 0.42));
	local cy = rc.top + math.floor(rc:Height()/2);
	local n = tetris_debris_n;
	local list = {};
	for i = 1, n do
		tetris.fx_seq = tetris.fx_seq + 1;
		local fid = tetris_fx_base_id + tetris.fx_seq;
		local frame = 1 + (i * 3 + row * 5) % 7;	-- 兜底配色
		local gx = math.min(tetris_col-1, math.floor((i-0.5) * tetris_col / n));
		local g = tetris.grid and tetris.grid[row] and tetris.grid[row][gx] or 0;
		if g > 0 then frame = g end	-- 碎片颜色 = 该处原方块颜色
		local x0 = rc.left + (i - 0.5) * rc:Width() / n - sz/2 + (math.random() - 0.5) * cw * 0.4;
		local y0 = cy - sz/2;
		local xml = '<img id="' .. fid .. '" skin="svg_tetris_blocks" iconIndex="' .. frame
			.. '" size="' .. sz .. ',' .. sz .. '" msgTransparent="1"/>';
		tetris.aniframe:CreateChildrenFromXml(xml);
		local w = tetris.aniframe:FindChildByID(fid, -1);
		if w then
			w:Move(CRect(math.floor(x0), math.floor(y0), math.floor(x0) + sz, math.floor(y0) + sz));
			-- 初速度:向上抛,水平向行两端散开
			local dir = (i <= n/2) and -1 or 1;
			local vx = dir * (60 + math.random() * 160);
			local vy = -(200 + math.random() * 180);
			list[#list+1] = { wnd = w, x0 = x0, y0 = y0, vx = vx, vy = vy };
		end
	end
	if #list == 0 then return end
	local ani = LuaValueAnimator();
	ani:CopyFrom(tetris.ani_move:GetIValueAnimator());	-- 模板无插值器 => GetFraction 即线性 t
	ani:GetIValueAnimator():setDuration(tetris_debris_ms);
	ani:SetRangeRect(CRect(0,0,1,1), CRect(0,0,1,1));	-- 只取进度(消消乐 LED 翻页同款)
	ani:SetOnUpdate("tetris_debris_update");
	ani:SetOnEnd("tetris_debris_end");
	local ctxId = tetris_new_ani_ctx({ kind = "debris", list = list, sz = sz, T = tetris_debris_ms / 1000.0, ani = ani });
	ani:SetCtx(ctxId);
	-- lua_tinker 的 BOOL 返回是数字,用 ==0 判(勿用 not)
	local ok = ani:Start(tetris.aniframe);
	if ok == 0 or ok == false then
		tetris.ani_ctx[ctxId] = nil;
		for _,d in ipairs(list) do d.wnd:Destroy() end
	end
end

-- 碎片运动: x = x0 + vx*t', y = y0 + vy*t' + g*t'^2/2(向上抛后坠落),整体线性淡出
function tetris_debris_update(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	if c == nil then return end
	local t = luaAni:GetFraction();
	local tm = t * c.T;
	local alpha = math.floor(255 * (1 - t) + 0.5);
	for _,d in ipairs(c.list) do
		if d.wnd then
			local x = d.x0 + d.vx * tm;
			local y = d.y0 + d.vy * tm + 0.5 * tetris_debris_g * tm * tm;
			local l = math.floor(x); local tp = math.floor(y);
			d.wnd:Move(CRect(l, tp, l + c.sz, tp + c.sz));
			d.wnd:SetAlpha(alpha);
		end
	end
end

-- 碎片动画结束:统一销毁(与 tetris_fly_end 同款清理)
function tetris_debris_end(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	tetris.ani_ctx[ctxId] = nil;
	if c == nil then return end
	for _,d in ipairs(c.list) do
		if d.wnd then d.wnd:Destroy() end
	end
end

-- 清场:销毁 ani_ctx 记录的全部特效窗口(碎片/飞分),重开/新局时兜底。
-- 正常路径由 OnEnd 自毁;若动画监听器被提前摘除(如包装对象被 GC)OnEnd
-- 丢失,窗口会残留,这里统一收掉。
function tetris_fx_clear()
	for _,c in pairs(tetris.ani_ctx) do
		if c.widget then c.widget:Destroy() end
		if c.list then
			for _,d in ipairs(c.list) do
				if d.wnd then d.wnd:Destroy() end
			end
		end
	end
	tetris.ani_ctx = {};
end

-- 光波动画播完(EventSwndAnimationStop):销毁特效窗口
function tetris_fx_stop(args)
	local fx = toSWindow(args:Sender());
	if fx then fx:Destroy() end
end

-- ==================== 活动方块层(浮层上的 4 个 img,移动时平移动画) ====================
-- grid 层只画已锁定格子与幽灵落点;当前方块由这一层绘制,左右移动/下落/软降
-- 用 40ms 平移动画滑入新格;动画中收到新输入立即完成旧动画(贴齐目标格)再起新动画。

-- 终止进行中的滑动动画(销毁路径用):旧 ctx 置 dead,update/end 跳过,
-- 窗口控制权交给调用方(即将销毁窗口,不贴位)
function tetris_active_kill_slide()
	if tetris.slide_ctx ~= 0 and tetris.ani_ctx[tetris.slide_ctx] then
		tetris.ani_ctx[tetris.slide_ctx].dead = true;
	end
	tetris.slide_ctx = 0;
end

-- 立即完成进行中的滑动动画:贴齐目标格并标记 dead(其 update/end 跳过)。
-- 动画中收到新输入(移动/旋转/重建)时调用,保证方块不滞留在半程位置。
function tetris_active_finish_slide()
	local ctxId = tetris.slide_ctx;
	tetris.slide_ctx = 0;
	if ctxId == 0 then return end
	local c = tetris.ani_ctx[ctxId];
	if c == nil then return end
	c.dead = true;
	for i = 1, 4 do
		if c.to[i] ~= nil then
			local w = tetris.aniframe and tetris.aniframe:FindChildByID(c.ids[i], -1);
			if w then w:Move(c.to[i]) end
		end
	end
	tetris.act = { rcs = c.to };
end

function tetris_active_destroy()
	tetris_active_kill_slide();
	if tetris.aniframe ~= nil then
		for i = 0, 3 do
			local w = tetris.aniframe:FindChildByID(tetris_active_base_id + i, -1);
			if w then w:Destroy() end
		end
	end
	tetris.act = nil;
end

-- 按当前数据位置重建活动层(幂等:已有 img 复用,缺的补建)。
-- 旋转/无动画兜底路径用它直接贴位;spawn 时建立。
function tetris_active_build()
	tetris_active_finish_slide();
	if tetris.aniframe == nil or tetris.cur == nil then tetris.act = nil; return end
	local cur = tetris.cur;
	local info = tetris_cells[cur.kind][cur.rot];
	local rcs = {};
	for i = 1, 4 do
		local c = info.cells[i];
		local rc = tetris_get_cell_rect(cur.x + c.x, cur.y + c.y);
		if rc ~= nil then
			rcs[i] = rc;
			local fid = tetris_active_base_id + i - 1;
			local w = tetris.aniframe:FindChildByID(fid, -1);
			if w == nil then
				tetris.aniframe:CreateChildrenFromXml('<img id="' .. fid
					.. '" skin="svg_tetris_blocks" iconIndex="' .. cur.kind
					.. '" size="' .. rc:Width() .. ',' .. rc:Height() .. '" msgTransparent="1"/>');
				w = tetris.aniframe:FindChildByID(fid, -1);
			end
			if w then
				w:SetAttribute(T"iconIndex", T(tostring(cur.kind)), false);
				w:Move(rc);
				w:SetVisible(true, true);
			end
		end
	end
	tetris.act = { rcs = rcs };
end

-- 平移一格动画(左右移动/重力下落/软降后调用;数据已更新,按目标格取终点)
function tetris_active_slide()
	if tetris.aniframe == nil or tetris.ani_move == nil or tetris.cur == nil then
		tetris_active_build();	-- 无动画能力:直接贴到数据位
		return
	end
	tetris_slide_start(tetris_slide_ms, "slide");
end

-- 滑动动画核心:4 个 img 从当前显示位滑向数据位;kind 供结束接力
-- ("drop"结束时锁定)。返回是否起飞;起飞失败/无格子时回退 build 贴位。
function tetris_slide_start(ms, kind)
	tetris_active_finish_slide();	-- 必须先立即完成旧动画:贴齐其目标格并把
	                                -- tetris.act.rcs 同步到真实显示位;否则新动画
	                                -- 会用过期的起点接力,且旧动画未标 dead 会与新
	                                -- 动画同时驱动同一批 img(快速连按软降可见方块回退)
	local cur = tetris.cur;
	local info = tetris_cells[cur.kind][cur.rot];
	local from = {}; local to = {}; local ids = {};
	for i = 1, 4 do
		local c = info.cells[i];
		local rc2 = tetris_get_cell_rect(cur.x + c.x, cur.y + c.y);
		if rc2 ~= nil then
			local rc1 = (tetris.act and tetris.act.rcs[i]) or rc2;	-- 接力:从当前显示位出发
			from[i] = rc1; to[i] = rc2;
			ids[i] = tetris_active_base_id + i - 1;
		end
	end
	if #to == 0 then tetris_active_build(); return false end
	local ani = LuaValueAnimator();
	ani:CopyFrom(tetris.ani_move:GetIValueAnimator());	-- 模板无插值器 => GetFraction 即线性 t
	ani:GetIValueAnimator():setDuration(ms);
	ani:SetRangeRect(CRect(0,0,1,1), CRect(0,0,1,1));	-- 只取进度
	ani:SetOnUpdate("tetris_slide_update");
	ani:SetOnEnd("tetris_slide_end");
	local ctxId = tetris_new_ani_ctx({ kind = kind, from = from, to = to, ids = ids, rcs = from, ani = ani });
	tetris.slide_ctx = ctxId;
	ani:SetCtx(ctxId);
	local ok = ani:Start(tetris.aniframe);
	if ok == 0 or ok == false then
		tetris.ani_ctx[ctxId] = nil;
		tetris.slide_ctx = 0;
		tetris_active_build();
		return false
	end
	return true
end

-- 滑动跟随:4 个 img 同步插值,并记录当前位置供下一次输入接力
function tetris_slide_update(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	if c == nil or c.dead then return end
	local t = luaAni:GetFraction();
	for i = 1, 4 do
		local a = c.from[i]; local b = c.to[i];
		if a ~= nil and b ~= nil then
			local l = math.floor(a.left + (b.left - a.left) * t);
			local tp = math.floor(a.top + (b.top - a.top) * t);
			local rc = CRect(l, tp, l + b:Width(), tp + b:Height());
			c.rcs[i] = rc;
			local w = tetris.aniframe and tetris.aniframe:FindChildByID(c.ids[i], -1);
			if w then w:Move(rc) end
		end
	end
end

-- 滑动结束:精确贴齐目标格(被接力/中断的旧动画不贴位,避免覆盖新状态);
-- 硬降动画落地即锁定(暂停中只落位,恢复后由重力节拍锁定)
function tetris_slide_end(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	tetris.ani_ctx[ctxId] = nil;
	if tetris.slide_ctx == ctxId then tetris.slide_ctx = 0 end
	if c == nil or c.dead then return end
	for i = 1, 4 do
		local w = tetris.aniframe and tetris.aniframe:FindChildByID(c.ids[i], -1);
		if w and c.to[i] then w:Move(c.to[i]) end
	end
	tetris.act = { rcs = c.to };
	if c.kind == "drop" and tetris.state == "playing" then
		tetris_lock();
	end
end

-- 飞行文本通用段:"+分数"/"LV N" 沿抛物线从 rcFrom 飞到 rcTo(消消乐 xxl_coin_fly 同款)
-- kind: "score" 落地触发得分 LED 滚动,"level" 落地刷新等级 LED;返回 false=没起飞,调用方走瞬时兜底
function tetris_fly_text(rcFrom, rcTo, text, colorText, kind)
	if tetris.aniframe == nil or tetris.ani_move == nil then return false end
	tetris.fx_seq = tetris.fx_seq + 1;
	local fid = tetris_fx_base_id + tetris.fx_seq;
	local w = 72; local h = 26;
	local px = rcFrom.left + math.floor(rcFrom:Width()/2) - math.floor(w/2);
	local py = rcFrom.top + math.floor(rcFrom:Height()/2) - math.floor(h/2);
	local xml = '<text id="' .. fid .. '" text="' .. text .. '" font="size:20,bold:1"'
		.. ' colorText="' .. colorText .. '" align="center" valign="center" msgTransparent="1"/>';
	tetris.aniframe:CreateChildrenFromXml(xml);
	local fy = tetris.aniframe:FindChildByID(fid, -1);
	if fy == nil then return false end
	fy:Move(CRect(px, py, px + w, py + h));
	local tx = rcTo.left + math.floor(rcTo:Width()/2) - math.floor(w/2);
	local ty = rcTo.top + math.floor(rcTo:Height()/2) - math.floor(h/2);
	local rc1 = CRect(px, py, px + w, py + h);
	local rc2 = CRect(tx, ty, tx + w, ty + h);
	-- 弓高:与飞行距离成比例(30%),限制在 50~120px,方向向上
	local arc = math.max(50, math.min(120, math.abs(tx - px) * 0.3));
	local ani = LuaValueAnimator();
	ani:CopyFrom(tetris.ani_move:GetIValueAnimator());	-- 模板无插值器 => GetFraction 即线性 t
	ani:GetIValueAnimator():setDuration(500);
	ani:SetRangeRect(rc1, rc2);	-- 线性基准(实际位置在 update 里叠加抛物线偏移)
	ani:SetOnUpdate("tetris_fly_update");
	ani:SetOnEnd("tetris_fly_end");
	local ctxId = tetris_new_ani_ctx({ kind = kind, widget = fy, r1 = rc1, r2 = rc2, arc = arc, ani = ani });
	ani:SetCtx(ctxId);
	-- lua_tinker 的 BOOL 返回是数字,用 ==0 判(勿用 not)
	local ok = ani:Start(tetris.aniframe);
	if ok == 0 or ok == false then
		tetris.ani_ctx[ctxId] = nil;
		fy:Destroy();
		return false
	end
	return true
end

-- 飞行跟随:x/宽/高线性插值,y 叠加向上弓起 arc*4t(1-t)(消消乐 xxl_fly_update 同款)
function tetris_fly_update(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	if c == nil or c.widget == nil then return end
	local t = luaAni:GetFraction();
	local x = c.r1.left + (c.r2.left - c.r1.left) * t;
	local y = c.r1.top + (c.r2.top - c.r1.top) * t - c.arc * 4 * t * (1 - t);
	local l = math.floor(x); local tp = math.floor(y);
	c.widget:Move(CRect(l, tp, l + c.r1:Width(), tp + c.r1:Height()));
end

-- 飞行落地:销毁文本窗口,按类型接力
function tetris_fly_end(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	if c == nil then return end
	if c.widget then c.widget:Destroy() end
	tetris.ani_ctx[ctxId] = nil;
	if c.kind == "score" then
		tetris_score_count_begin();
	elseif c.kind == "level" then
		tetris_show_level();
	end
end

-- 消行计分动画入口:被消行中部弹 "+分数" 飞向得分 LED,同时 LED 滚动;
-- 升级时再飞一条 "LV N" 到等级 LED;浮层不可用或起飞失败 => 瞬时对齐显示
function tetris_score_fly(pts, levelUp, newLevel, rowsCleared)
	local rcFrom = nil;
	if tetris.aniframe and rowsCleared and #rowsCleared > 0 then
		rcFrom = tetris_get_row_rect(rowsCleared[math.ceil(#rowsCleared / 2)]);
	end
	if rcFrom ~= nil then
		local rcTo = tetris_get_led_rect("digit_tscore", 4);
		if rcTo ~= nil and tetris_fly_text(rcFrom, rcTo, "+" .. pts, "#FFD700", "score") then
			tetris_score_count_begin();
			if levelUp then tetris_level_fly(newLevel) end
			return
		end
	end
	tetris_score_count_abort();	-- 兜底瞬时对齐前丢弃可能还在跑的滚动段
	tetris.score_shown = tetris.score;
	tetris_show_score();
end

-- 升级提示:"LV N" 从棋盘竖直中部飞向等级 LED
function tetris_level_fly(newLevel)
	local rcFrom = tetris_get_row_rect(9);
	local rcTo = tetris_get_led_rect("digit_tlevel", 2);
	if rcFrom == nil or rcTo == nil then
		tetris_show_level();
		return
	end
	if not tetris_fly_text(rcFrom, rcTo, "LV " .. newLevel, "#22D3EE", "level") then
		tetris_show_level();
	end
end

-- 得分 LED 滚动段(shown -> score,消消乐 xxl_coin_count 同款):
-- 软降/硬降/消行加分统一入口;滚动重入时废弃旧段,从当前显示值滚向最新分数
function tetris_score_count_begin()
	if tetris.root == nil or tetris.aniframe == nil or tetris.ani_move == nil then
		tetris.score_shown = tetris.score;
		tetris_show_score();
		return
	end
	if tetris.score_shown == tetris.score then return end
	tetris_score_count_abort();	-- 上一段滚动还在跑:废弃,避免两段拉扯显示值
	local ani = LuaValueAnimator();
	ani:CopyFrom(tetris.ani_move:GetIValueAnimator());
	ani:GetIValueAnimator():setDuration(400);
	ani:SetRangeRect(CRect(0,0,1,1), CRect(0,0,1,1));	-- rect 恒等,仅取 fraction 驱动
	ani:SetOnUpdate("tetris_score_count_update");
	ani:SetOnEnd("tetris_score_count_end");
	local ctxId = tetris_new_ani_ctx({ kind = "count", ani = ani, from = tetris.score_shown, to = tetris.score });
	tetris.count_ctx = ctxId;
	ani:SetCtx(ctxId);
	local ok = ani:Start(tetris.aniframe);
	if ok == 0 or ok == false then
		tetris.ani_ctx[ctxId] = nil;
		tetris.count_ctx = 0;
		tetris.score_shown = tetris.score;
		tetris_show_score();
	end
end

-- 丢弃进行中的滚动段(reset/瞬时兜底前调用):ctx 置空后旧段的 update/end 均为 no-op
function tetris_score_count_abort()
	if tetris.count_ctx and tetris.count_ctx ~= 0 then
		tetris.ani_ctx[tetris.count_ctx] = nil;
		tetris.count_ctx = 0;
	end
end

-- 滚动跟随:按 fraction 在 [from,to] 间取整翻 LED
function tetris_score_count_update(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	if c == nil then return end
	local v = math.floor(c.from + (c.to - c.from) * luaAni:GetFraction() + 0.5);
	if v ~= tetris.score_shown then
		tetris.score_shown = v;
		tetris_show_score();
	end
end

-- 滚动结束:显示值对齐真实分数
function tetris_score_count_end(luaAni, ctxId)
	local c = tetris.ani_ctx[ctxId];
	if c == nil then return end
	tetris.ani_ctx[ctxId] = nil;
	if tetris.count_ctx == ctxId then tetris.count_ctx = 0 end
	tetris.score_shown = tetris.score;
	tetris_show_score();
end

-- 50ms 节拍:推进重力 / 消行闪白
function tetris_on_timer(args)
	if tetris.state == "playing" then
		tetris.ms = tetris.ms + tetris_tick_ms;
		if tetris.ms >= tetris_gravity_ms() then
			tetris.ms = 0;
			if tetris_try_move(0,1) then
				tetris_active_slide();	-- 下落一格也走平移动画
				tetris_render();
			else
				tetris_lock();
			end
		end
	elseif tetris.state == "clearing" then
		tetris.flash_ms = tetris.flash_ms + tetris_tick_ms;
		local on = math.floor(tetris.flash_ms / 100) % 2 == 0;
		if on ~= tetris.flash_on then
			tetris.flash_on = on;
			tetris_render();
		end
		if tetris.flash_ms >= tetris_flash_ms then
			-- 光波+碎片爆发:每个被消行一组(collapse 抹掉格子前,按当前布局取位取色)
			for _,row in ipairs(tetris.flash_rows) do tetris_pop_row_fx(row) end
			tetris_collapse();
		end
	end
	return 0;
end

-- 键盘处理(棋盘通过 SubscribeEvent(EVT_KEYDOWN) 订阅)
function tetris_on_key(args)
	if tetris.wndBoard == nil then return 0 end
	local evt = toStEventKeyDown(args);
	if evt == nil then return 0 end
	local key = evt.nChar;
	if tetris.state == "playing" then
		if key == 37 then			-- VK_LEFT
			if tetris_try_move(-1,0) then tetris_active_slide(); tetris_render() end
			return 1;
		elseif key == 39 then		-- VK_RIGHT
			if tetris_try_move(1,0) then tetris_active_slide(); tetris_render() end
			return 1;
		elseif key == 40 then		-- VK_DOWN 软降
			if tetris_try_move(0,1) then
				tetris.score = tetris.score + 1;
				tetris.ms = 0;
				tetris_score_count_begin();	-- LED 滚动跟随(消消乐同款,重入自动重启)
				tetris_active_slide();
				tetris_render();
			end
			return 1;
		elseif key == 38 then		-- VK_UP 旋转
			if tetris_try_rotate() then tetris_active_build(); tetris_play_snd("tetris_rotate"); tetris_render() end
			return 1;
		elseif key == 32 then		-- VK_SPACE 硬降
			tetris_hard_drop();
			return 1;
		elseif key == 80 then		-- 'P' 暂停
			tetris_pause();
			return 1;
		end
	elseif tetris.state == "paused" and key == 80 then
		tetris_resume();
		return 1;
	end
	return 0;
end

function tetris_set_start_btn(text)
	if tetris.root == nil then return end
	local btn = tetris.root:FindChildByNameA("btn_tetris_start",-1);
	if btn then btn:SetWindowText(text) end
end

-- 七段 LED 翻页计数(与 xxl_set_digits 同款,按位数泛化;不滚动,直接翻页)
function tetris_set_digits(prefix, num, ndigits)
	if tetris.root == nil then return end
	for i = 0, ndigits-1 do
		local stack = tetris.root:FindChildByNameA(prefix .. "_" .. i, -1);
		if stack then
			local d = math.floor(num / 10^i) % 10;
			local stackApi = QiIStackView(stack);
			if stackApi then
				stackApi:SelectPage(d, false);
				stackApi:Release();
			end
		end
	end
end

function tetris_show_score() tetris_set_digits("digit_tscore", (tetris.score_shown or 0) % 10000, 4) end
function tetris_show_lines() tetris_set_digits("digit_tlines", tetris.lines % 1000, 3) end
function tetris_show_level() tetris_set_digits("digit_tlevel", tetris.level % 100, 2) end

function tetris_show_state()
	if tetris.root == nil then return end
	local txt = tetris.root:FindChildByNameA("txt_tetris_state",-1);
	if txt == nil then return end
	local s = {idle = "待开始", playing = "进行中", clearing = "消除中", paused = "已暂停", over = "游戏结束"};
	txt:SetWindowText(T("状态:" .. (s[tetris.state] or "")));
end

-- 渲染缓存辅助:值变化才写属性,减少无效重绘
function tetris_set_frame(wnd, cache, id, frame)
	if cache[id] == frame then return end
	cache[id] = frame;
	local ele = wnd:FindChildByID(id, -1);
	if ele then
		ele:SetAttribute(T"iconIndex", T(tostring(frame)), false);
		ele:Invalidate();
	end
end

-- 渲染棋盘:已锁定格子 + 幽灵落点 + 消行闪白
-- (活动方块本体由浮层活动层绘制,tetris_active_*,不在此画)
function tetris_render()
	if tetris.wndBoard == nil then return end
	local overlay = {};
	if tetris.cur ~= nil then
		local cur = tetris.cur;
		local gy = cur.y;	-- 幽灵:当前形状一路落到不能再落的位置
		while tetris_can_place(cur.kind, cur.rot, cur.x, gy+1) do gy = gy + 1 end
		if gy > cur.y then
			for _,c in ipairs(tetris_cells[cur.kind][cur.rot].cells) do
				overlay[(gy+c.y)*tetris_col + cur.x+c.x] = cur.kind + 7;
			end
		end
	end
	for y = 0, tetris_row-1 do
		local flashing = false;
		if tetris.state == "clearing" and tetris.flash_on then
			for _,fy in ipairs(tetris.flash_rows) do
				if fy == y then flashing = true; break end
			end
		end
		for x = 0, tetris_col-1 do
			local frame;
			local cellId = y*tetris_col + x;
			if overlay[cellId] then
				frame = overlay[cellId];
			elseif flashing then
				frame = tetris_frame_flash;
			else
				frame = tetris.grid[y][x];	-- 0=空格,1-7=方块,恰好是雪碧图帧号
			end
			tetris_set_frame(tetris.wndBoard, tetris.cache, tetris_base_id + cellId, frame);
		end
	end
end

-- 渲染下一块预览(4x4 内居中)
function tetris_render_next()
	if tetris.wndNext == nil then return end
	local overlay = {};
	local kind = tetris.nextKind;
	if kind ~= 0 then
		local info = tetris_cells[kind][1];
		local ox = math.floor((4-info.w)/2); local oy = math.floor((4-info.h)/2);
		for _,c in ipairs(info.cells) do
			overlay[(oy+c.y)*4 + ox+c.x] = kind;
		end
	end
	for i = 0, 15 do
		tetris_set_frame(tetris.wndNext, tetris.nextCache, tetris_next_base_id + i, overlay[i] or 0);
	end
end

-- 复位数据(不动 UI 状态)
function tetris_reset_data()
	tetris.grid = {};
	for y = 0, tetris_row-1 do
		tetris.grid[y] = {};
		for x = 0, tetris_col-1 do tetris.grid[y][x] = 0 end
	end
	tetris.bag = {};
	tetris.nextKind = tetris_next_kind();
	tetris.cur = nil;
	tetris.score = 0; tetris.lines = 0; tetris.level = 1;
	tetris.ms = 0; tetris.flash_ms = 0; tetris.flash_rows = nil; tetris.flash_on = false;
	tetris.score_shown = 0;
	tetris_score_count_abort();	-- 丢弃旧局还在跑的滚动段(其 end 会把显示值拉回旧分数)
	tetris_fx_clear();			-- 兜底清掉残留特效窗口(碎片/飞分)
	tetris_active_destroy();
end

-- 开始新的一局(开始按钮在 idle/over 时、重新开始按钮)
function tetris_start()
	tetris_over_hide();			-- 有结束窗口在显示则动画隐藏(开始/重开共用入口)
	tetris_reset_data();
	tetris_render_next();
	tetris_show_score(); tetris_show_lines(); tetris_show_level();
	tetris.state = "playing";
	tetris_show_state();
	tetris_set_start_btn(T"暂 停");
	if tetris.timer then
		tetris.timer:KillTimer();
		tetris.timer:StartTimer(tetris_tick_ms, 1, 0);
	end
	tetris_spawn();
	tetris_render();
end

function tetris_pause()
	if tetris.state ~= "playing" and tetris.state ~= "clearing" then return end
	tetris.pausedFrom = tetris.state;
	tetris.state = "paused";
	if tetris.timer then tetris.timer:KillTimer() end
	tetris_set_start_btn(T"继 续");
	tetris_show_state();
end

function tetris_resume()
	if tetris.state ~= "paused" then return end
	tetris.state = tetris.pausedFrom or "playing";
	tetris.pausedFrom = nil;
	if tetris.timer then tetris.timer:StartTimer(tetris_tick_ms, 1, 0) end
	tetris_set_start_btn(T"暂 停");
	tetris_show_state();
end

-- ==================== 游戏结束窗口(wnd_gameover,布局内静态窗口,不拼 XML) ====================
-- 展示/隐藏动画全走 anim 模板 + SWindow::SetAnimation:
--   遮罩 wnd_gameover 挂 tetris_over_mask_in/out(alpha 淡入淡出),
--   卡片 wnd_gameover_card 挂 tetris_over_card_in/out(scale+alpha)。
-- wnd_gameover 在 XML 上绑定 on_animation_stop="tetris_on_over_stop",
-- 隐藏段(over_hiding)动画结束后由回调真正 SetVisible(false)。
-- lua 不再用数值动画逐帧驱动 alpha;窗口自身 alpha 保持 255,可见度完全
-- 由 SAnimation 变换控制(注意变换与窗口 alpha 相乘,勿把窗口 alpha 置 0)。

function tetris_over_wnd()
	if tetris.over_wnd == nil and tetris.root ~= nil then
		tetris.over_wnd = tetris.root:FindChildByNameA("wnd_gameover",-1);
	end
	return tetris.over_wnd;
end

-- 挂载展示/隐藏动画对:遮罩与卡片各从 master clone 一个实例交给窗口接管
-- (fx_beam 同款:clone 后 SetAnimation 再 Release,master 由 exit 释放)。
-- 先挂动画后 SetVisible:动画 startNow 首帧即从 fromAlpha 起步,防闪帧。
function tetris_over_ani_play(maskKey, cardKey, maskName, cardName)
	local wnd = tetris_over_wnd();
	if wnd == nil then return end
	if tetris[maskKey] == nil then
		tetris[maskKey] = GetApp():LoadAnimation("anim:" .. maskName);
	end
	if tetris[maskKey] then
		local ani = tetris[maskKey]:clone();
		wnd:SetAnimation(ani);
		ani:Release();
	end
	local card = wnd:FindChildByNameA("wnd_gameover_card",-1);
	if card == nil then return end
	if tetris[cardKey] == nil then
		tetris[cardKey] = GetApp():LoadAnimation("anim:" .. cardName);
	end
	if tetris[cardKey] then
		local ani = tetris[cardKey]:clone();
		card:SetAnimation(ani);
		ani:Release();
	end
end

-- wnd_gameover 的动画结束回调(XML on_animation_stop 绑定):隐藏段收尾,
-- 真正隐藏窗口;展示段仅清标志。卡片动画 stop 冒泡进来时幂等。
function tetris_on_over_stop(args)
	local evt = toStEventSwndAnimationStop(args);
	if evt == nil then return 0 end
	local hiding = tetris.over_hiding;
	tetris.over_hiding = false;
	if hiding then
		local wnd = tetris_over_wnd();
		if wnd then wnd:SetVisible(false, true) end
	end
	return 0;
end

-- 游戏结束:刷新分数文本 + 结束窗口淡入(遮罩 alpha + 卡片 scale/alpha)
function tetris_over_show()
	local wnd = tetris_over_wnd();
	if wnd == nil then return end
	local txt = tetris.root:FindChildByNameA("txt_score",-1);
	if txt then txt:SetWindowText(T("本局得分 " .. tetris.score)) end
	if tetris.over_visible then return end	-- 已在展示(或隐藏动画收尾中)
	tetris_over_ani_play("over_ani_mask_in", "over_ani_in",
		"tetris_over_mask_in", "tetris_over_card_in");
	wnd:SetVisible(true, true);
	tetris.over_visible = true;
end

-- 开始游戏:若有结束窗口在显示则动画隐藏(out 动画 stop 回调里 SetVisible)
function tetris_over_hide()
	local wnd = tetris_over_wnd();
	if wnd == nil then return end
	if not tetris.over_visible then
		wnd:SetVisible(false, true);	-- 兜底:确保隐藏态
		return
	end
	tetris.over_visible = false;
	tetris.over_hiding = true;	-- out 动画结束后 tetris_on_over_stop 里隐藏
	tetris_over_ani_play("over_ani_mask_out", "over_ani_out",
		"tetris_over_mask_out", "tetris_over_card_out");
end

function tetris_game_over()
	tetris.state = "over";
	if tetris.timer then tetris.timer:KillTimer() end
	tetris_set_start_btn(T"开始游戏");
	tetris_show_state();
	tetris_active_destroy();	-- 结束画面清掉活动层(定格的方块写回 grid 由 render 呈现)
	tetris_render();
	tetris_play_snd("tetris_over");
	tetris_over_show();	-- 结束窗口动画展示(只显示结果,重开走左侧"重新开始"按钮)
end

-- 开始/暂停/继续 三态按钮
function tetris_on_start(args)
	if tetris.wndBoard == nil then return 0 end
	if tetris.state == "playing" or tetris.state == "clearing" then
		tetris_pause();
	elseif tetris.state == "paused" then
		tetris_resume();
	else
		tetris_start();
	end
	tetris.wndBoard:SetFocus();
	return 1;
end

-- 重新开始:任何状态都重开一局
function tetris_on_restart(args)
	if tetris.wndBoard == nil then return 0 end
	tetris_start();
	tetris.wndBoard:SetFocus();
	return 1;
end

-- 子页被隐藏时自动暂停,避免后台空转
function tetris_on_page_vis(args)
	if tetris.wndBoard == nil then return 0 end
	local evt = toStEventSwndVisibleChanged(args);
	if evt == nil then return 0 end
	if evt.bVisible == 0 and (tetris.state == "playing" or tetris.state == "clearing") then
		tetris_pause();
	end
	return 0;
end

-- 动态创建棋盘/预览格子(img + svg_tetris_blocks 雪碧图帧)
function tetris_init_board()
	tetris.wndBoard:DestroyAllChildren();
	tetris.wndNext:DestroyAllChildren();
	tetris.cache = {}; tetris.nextCache = {};
	local xml = "";
	for y = 0, tetris_row-1 do
		for x = 0, tetris_col-1 do
			xml = xml .. '<img id="' .. (tetris_base_id + y*tetris_col + x)
				.. '" size="' .. tetris_cell .. ',' .. tetris_cell
				.. '" skin="svg_tetris_blocks" iconIndex="0"/>';
		end
	end
	tetris.wndBoard:CreateChildrenFromXml(xml);
	tetris.wndBoard:RequestRelayout();
	xml = "";
	for i = 0, 15 do
		xml = xml .. '<img id="' .. (tetris_next_base_id + i)
			.. '" size="' .. tetris_next_cell .. ',' .. tetris_next_cell
			.. '" skin="svg_tetris_blocks" iconIndex="0"/>';
	end
	tetris.wndNext:CreateChildrenFromXml(xml);
	tetris.wndNext:RequestRelayout();
end

function tetris_init(root)
	local wndBoard = root:FindChildByNameA("wnd_tetris_board",-1);
	local wndNext = root:FindChildByNameA("wnd_tetris_next",-1);
	if wndBoard == nil or wndNext == nil then
		return 0; -- demo 不含俄罗斯方块子页,跳过
	end
	tetris.root = root;
	tetris.wndBoard = wndBoard;
	tetris.wndNext = wndNext;
	tetris_build_cells();
	-- 动画:浮层 + animator:xxl_move 模板(与消消乐共用 animator 资源)
	-- 模板/浮层缺失只影响动画,游戏本体照常(各动画入口有兜底)
	tetris.aniframe = root:FindChildByNameA("wnd_tetris_aniframe", -1);
	if tetris.aniframe then
		tetris.ani_move = LuaValueAnimator();
		if not tetris.ani_move:LoadAnimator("animator:xxl_move") then
			tetris_slog("init: load animator:xxl_move failed");
			tetris.ani_move = nil;
		end
	end
	-- 独立计时器(与跑马机的 on_timer 互不影响)
	local souiFac = CreateSouiFactory();
	local timerSlot = CreateEventSlot("tetris_on_timer");
	tetris.timer = souiFac:CreateTimer(timerSlot);
	timerSlot:Release();
	souiFac:Release();
	-- 键盘:棋盘可聚焦,订阅 EVT_KEYDOWN(与 btn_lrc 订阅 EVT_CMD 同一套路)
	tetris_init_board();
	tetris_reset_data();
	local keySlot = CreateEventSlot("tetris_on_key");
	wndBoard:SubscribeEvent(8200, keySlot);	-- 8200 == EVT_KEYDOWN
	keySlot:Release();
	tetris_render_next();
	tetris_show_score(); tetris_show_lines(); tetris_show_level();
	tetris_show_state();
	tetris_slog("init done");
	return 1;
end

function tetris_exit()
	if tetris.timer then
		tetris.timer:KillTimer();
		tetris.timer:Release();
		tetris.timer = nil;
	end
	if tetris.fx_ani_cache then
		tetris.fx_ani_cache:Release();	-- LoadAnimation 返回接管引用,须显式释放
		tetris.fx_ani_cache = nil;
	end
	if tetris.over_ani_mask_in then
		tetris.over_ani_mask_in:Release();
		tetris.over_ani_mask_in = nil;
	end
	if tetris.over_ani_mask_out then
		tetris.over_ani_mask_out:Release();
		tetris.over_ani_mask_out = nil;
	end
	if tetris.over_ani_in then
		tetris.over_ani_in:Release();
		tetris.over_ani_in = nil;
	end
	if tetris.over_ani_out then
		tetris.over_ani_out:Release();
		tetris.over_ani_out = nil;
	end
	tetris.ani_ctx = {};
end

--[[
==================== 坦克大战(lua 版) ====================
经典 Battle City 玩法: 13x13 网格,守住底部基地,消灭本关全部敌人。
地形与爆炸画在棋盘格上,坦克/炮弹/无敌护盾画在棋盘上方的精灵层:
逻辑仍按格推进,渲染层用 vx/vy(可带小数的格坐标)在 60fps 定时器里逐帧插值,
所以坦克与炮弹是连续滑动而不是逐格跳动。
数据层(地图/移动/碰撞/炮弹/出生/计分)与渲染层分离,可在裸 lua 中回归测试。
地形: 0 空地 / 1 砖墙 / 2 残砖 / 3 钢墙 / 4 水 / 5 基地 / 6 基地被毁。
方向: 0 上 / 1 右 / 2 下 / 3 左。玩家用方向键或 WASD 移动/转向,空格开炮,P 暂停。
]]

tank_base_id = 64000;		-- 棋盘格子 id 段(195 个,按 y*13+x 递增)
tank_fx_base_id = 67000;	-- 飞分文本 id 段
tank_spr_base_id = 68000;	-- 活动精灵 id 段(护盾/玩家/敌人/炮弹)
tank_row = 13;
tank_col = 13;
tank_pad_rows = 1;		-- 战场上下各一行砖墙装饰,与地图砖墙共用 svg_tank 帧
tank_cell = 26;			-- 与 page_script.xml 中 152 宽侧边卡 + 8 间隔的布局匹配
tank_tick_ms = 16;		-- 60fps:移动/炮弹靠像素插值,不再按格跳动
tank_bullet_ms = 90;		-- 炮弹飞过一格所需时间
tank_player_move_cd = 150;	-- 玩家连续移动的最小间隔(按住方向键时按此节奏匀速前进)
tank_player_fire_cd = 260;
tank_player_bullet_max = 2;	-- 同屏玩家炮弹上限
tank_enemy_max = 4;			-- 同屏敌坦克上限
tank_bullet_pool = 8;		-- 炮弹/敌坦克精灵池大小
tank_invuln_ms = 2000;		-- 玩家重生后的无敌时间(护盾环缓慢呼吸+旋转,不闪烁)
tank_respawn_ms = 1200;		-- 被击毁后的重生等待
tank_shield_breath_ms = 1400;	-- 护盾环呼吸周期
tank_shield_spin_ms = 3600;		-- 护盾环转一圈的时间
tank_hit_dist = 0.45;			-- 子弹与坦克画面中心距离小于该值(格)才算接触

-- 方向键映射(方向键 + WASD 都按同一套状态处理,支持按住连续移动)
tank_key_dir = { [37] = 3, [39] = 1, [38] = 0, [40] = 2,
	[65] = 3, [68] = 1, [87] = 0, [83] = 2 };

-- 地形值(与雪碧图帧号 0-6 一一对应)
tank_T_EMPTY = 0;
tank_T_BRICK = 1;
tank_T_BRICK_HALF = 2;
tank_T_STEEL = 3;
tank_T_WATER = 4;
tank_T_BASE = 5;
tank_T_BASE_DEAD = 6;

-- 雪碧图帧号
tank_F_PLAYER = 7;			-- + 方向(7-10)
tank_F_ENEMY = 11;			-- + 方向(11-14)
tank_F_ENEMY_FAST = 15;		-- + 方向(15-18)
tank_F_BULLET_P = 19;
tank_F_BULLET_E = 20;
tank_F_EXP1 = 21;
tank_F_EXP2 = 22;
tank_F_EXP3 = 23;
tank_F_SHIELD = 26;			-- 无敌护盾环(叠加在玩家坦克上)

tank_dir_dx = {0, 1, 0, -1};
tank_dir_dy = {-1, 0, 1, 0};

tank_spawn_pts = { {0,0}, {6,0}, {12,0} };	-- 顶部三个出生点

-- 关卡地图(13x13,共 10 关): . 空地 B 砖 S 钢 W 水 E 基地(基地在底部中央,周围有砖墙)
-- 顶部三个敌人出生点(0,0)(6,0)(12,0)与玩家出生点(4,12)在各关都保持空地
tank_maps = {
	{
		".....B.B.....",
		".....B.B.....",
		"..BBBB.BBBB..",
		"..B.......B..",
		"..B.BB.BB.B..",
		"....B...B....",
		"BB..B.S.B..BB",
		"....B...B....",
		"..B.BB.BB.B..",
		"..B.......B..",
		"..BBBB.BBBB..",
		".....BBB.....",
		".....BEB.....",
	},
	{
		".....B.B.....",
		"..S..B.B..S..",
		"..B..B.B..B..",
		"..B.BB.BB.B..",
		".....B.B.....",
		".WW...B...WW.",
		".B.....S...B.",
		".....BBBB....",
		"..B..B..B..B.",
		"..B..B..B..B.",
		"..BBB...BBB..",
		".....BBB.....",
		".....BEB.....",
	},
	{
		".....B.B.....",
		"..B..B.B..B..",
		"..B..B.B..B..",
		"..B..BBB..B..",
		"..B.......B..",
		".WWW.B.B.WWW.",
		"..B..B.B..B..",
		"..B..B.B..B..",
		"..B..B.B..B..",
		"..BBBB.BBBB..",
		".............",
		".....BBB.....",
		".....BEB.....",
	},
	-- 4 双环要塞:上下对称的砖环,两翼钢墙保护补给线
	{
		".....B.B.....",
		"..S.......S..",
		"..B.BB.BB.B..",
		"...B.....B...",
		".BB.B.S.B.BB.",
		"...B.....B...",
		"..B.BB.BB.B..",
		"..S.......S..",
		"...B.BBB.B...",
		"...B.....B...",
		"..BBB...BBB..",
		".....BBB.....",
		".....BEB.....",
	},
	-- 5 砖林:密集横墙,靠打穿砖块开辟纵向通道
	{
		".....B.B.....",
		".BBBBB.BBBBB.",
		".....B.B.....",
		".BBB.B.B.BBB.",
		".....B.B.....",
		".BBBBB.BBBBB.",
		".....B.B.....",
		".BBB.B.B.BBB.",
		".....B.B.....",
		".BBBBB.BBBBB.",
		".....B.B.....",
		".....BBB.....",
		".....BEB.....",
	},
	-- 6 钢卫走廊:中央纵向走廊,两侧钢墙与水池夹击
	{
		".....B.B.....",
		"..SS.....SS..",
		"..SS.....SS..",
		".....BBB.....",
		"..B..B.B..B..",
		"..B..B.B..B..",
		".WW..B.B..WW.",
		"..B..B.B..B..",
		"..B..B.B..B..",
		".....BBB.....",
		"..SS.....SS..",
		".....BBB.....",
		".....BEB.....",
	},
	-- 7 十字回廊:四角砖房 + 中央钢柱
	{
		".....B.B.....",
		"..B.B...B.B..",
		"..B.B.B.B.B..",
		"..B...B...B..",
		"..BBB.B.BBB..",
		".....B.B.....",
		".SS.BBBBB.SS.",
		".....B.B.....",
		"..BBB.B.BBB..",
		"..B...B...B..",
		"..B.B.B.B.B..",
		".....BBB.....",
		".....BEB.....",
	},
	-- 8 迷墙:错位砖墙 + 水池截断中路
	{
		".....B.B.....",
		"...B...B.....",
		".BBBB.B.BBBB.",
		".....B.B.....",
		".BB.B...B.BB.",
		"...B.BBB.B...",
		".WWB.....BWW.",
		"...B.BBB.B...",
		".BB.B...B.BB.",
		".....B.B.....",
		".BBBB.B.BBBB.",
		".....BBB.....",
		".....BEB.....",
	},
	-- 9 钢岛:开阔地形,中央四块钢墙做掩体
	{
		".....B.B.....",
		".S...B.B...S.",
		"..B..B.B..B..",
		"..B.BB.BB.B..",
		".W.........W.",
		"..BB.S.S.BB..",
		".W.........W.",
		"..BB.S.S.BB..",
		".W.........W.",
		"..B.BB.BB.B..",
		"..B..B.B..B..",
		".....BBB.....",
		".....BEB.....",
	},
	-- 10 最终要塞:中央水城 + 钢墙门柱,砖墙多层保护基地
	{
		".....B.B.....",
		"..SSS.B.SSS..",
		"..B...B...B..",
		"..B.WWW.W.B..",
		"..B.W...W.B..",
		"..S.W.S.W.S..",
		"..B.W...W.B..",
		"..B.WWW.W.B..",
		"..B...B...B..",
		"..BBB.B.BBB..",
		"..S..B.B..S..",
		".....BBB.....",
		".....BEB.....",
	},
};

tank = {
	root = nil, wndBoard = nil, aniframe = nil, timer = nil,
	terrain = {}, player = nil, enemies = {}, bullets = {}, explosions = {},
	score = 0, score_shown = 0, lives = 3, stage = 1,
	state = "idle", total = 0, spawned = 0, killed = 0,
	time_ms = 0, spawn_ms = 0, spawn_idx = 0, respawn_ms = 0, clear_ms = 0,
	cache = {}, ani_move = nil, ani_ctx = {}, fx_seq = 0, count_ctx = 0,
	over_wnd = nil, over_visible = false, over_hiding = false, over_reason = nil,
	sprLayer = nil, geo = nil, sprCache = {}, sprEle = {},
	key_dir = nil, held = {}, held_order = {},
};

function tank_slog(msg)
	slog("tank: " .. msg);
end

-- ==================== 数据层: 地图 / 碰撞 / 移动 ====================

function tank_map_at(stage)
	return tank_maps[(stage - 1) % #tank_maps + 1];
end

function tank_parse_map(rows)
	local legend = { ["."] = tank_T_EMPTY, ["B"] = tank_T_BRICK, ["S"] = tank_T_STEEL,
		["W"] = tank_T_WATER, ["E"] = tank_T_BASE };
	local grid = {};
	for y = 0, tank_row-1 do
		grid[y] = {};
		local row = rows[y+1] or "";
		for x = 0, tank_col-1 do
			grid[y][x] = legend[string.sub(row, x+1, x+1)] or tank_T_EMPTY;
		end
	end
	return grid;
end

function tank_in_bounds(x, y)
	return x >= 0 and x < tank_col and y >= 0 and y < tank_row;
end

-- 地图格(x,y)对应棋盘子窗口 id;棋盘上下各有一行砖墙装饰,故行号整体下移
function tank_cellid(x, y)
	return tank_base_id + (y + tank_pad_rows)*tank_col + x;
end

function tank_terrain_blocks_tank(x, y)
	local t = tank.terrain[y][x];
	return t == tank_T_BRICK or t == tank_T_BRICK_HALF or t == tank_T_STEEL
		or t == tank_T_WATER or t == tank_T_BASE;
end

-- 指定格上的坦克(ignore 为移动者自身);玩家已阵亡时不再占位
function tank_tank_at(x, y, ignore)
	local p = tank.player;
	if p and p.alive and p ~= ignore and p.x == x and p.y == y then return p end
	for _, e in ipairs(tank.enemies) do
		if e.alive and e ~= ignore and e.x == x and e.y == y then return e end
	end
	return nil;
end

function tank_cell_free(x, y, mover)
	if not tank_in_bounds(x, y) then return false end
	if tank_terrain_blocks_tank(x, y) then return false end
	if tank_tank_at(x, y, mover) then return false end
	return true;
end

-- 坦克转向并尝试前进:被挡住也会转向(经典操作手感)
-- dur: 这一格在画面上滑过去的时长(毫秒),渲染层用它做像素插值
function tank_move_entity(ent, dir, dur)
	ent.dir = dir;
	local nx = ent.x + tank_dir_dx[dir+1];
	local ny = ent.y + tank_dir_dy[dir+1];
	if tank_cell_free(nx, ny, ent) then
		ent.x = nx; ent.y = ny;
		tank_anim_start(ent, dur or tank_player_move_cd);
		return true;
	end
	return false;
end

-- 逻辑位置仍在格子上;vx/vy 是画面上连续的位置(以格为单位,可带小数)
-- tox/toy: 动画终点,缺省为 ent.x/ent.y(坦克先把 x/y 改成目标格再调用)
-- 炮弹的逻辑位置在到达前不改,所以必须显式传入下一格,否则画面会落后一格
function tank_anim_start(ent, dur, tox, toy)
	if ent.vx == nil then ent.vx = ent.x; ent.vy = ent.y end
	ent.mx0 = ent.vx; ent.my0 = ent.vy;
	ent.mx1 = tox or ent.x; ent.my1 = toy or ent.y;
	ent.mv_ms = 0;
	ent.mv_dur = dur or 0;
end

function tank_anim_advance(ent, dt)
	if ent.vx == nil then ent.vx = ent.x; ent.vy = ent.y end
	if ent.mv_dur and ent.mv_dur > 0 then
		ent.mv_ms = ent.mv_ms + dt;
		local t = ent.mv_ms / ent.mv_dur;
		if t >= 1 then
			t = 1;
			ent.mv_dur = 0;
		end
		ent.vx = ent.mx0 + (ent.mx1 - ent.mx0) * t;
		ent.vy = ent.my0 + (ent.my1 - ent.my0) * t;
	else
		ent.vx = ent.x; ent.vy = ent.y;
	end
end

function tank_anim_snap(ent)
	ent.vx = ent.x; ent.vy = ent.y;
	ent.mv_dur = 0; ent.mv_ms = 0;
	ent.mx0 = ent.x; ent.my0 = ent.y; ent.mx1 = ent.x; ent.my1 = ent.y;
end

function tank_player_move(dir)
	local p = tank.player;
	if p == nil or not p.alive or tank.state ~= "playing" then return false end
	if p.move_cd > 0 then
		p.dir = dir;	-- 冷却中仍允许转向
		return false
	end
	p.move_cd = tank_player_move_cd;
	tank_move_entity(p, dir, tank_player_move_cd);
	return true
end

-- 按住方向键:记录按键栈,tank_tick 按 move_cd 节奏持续前进;
-- 只依赖系统按键重复会让移动一顿一顿,这里改由 60fps 定时器接管。
function tank_keys_clear()
	tank.held = {};
	tank.held_order = {};
	tank.key_dir = nil;
end

function tank_key_down(key)
	local d = tank_key_dir[key];
	if d == nil then return nil end
	local order = tank.held_order;
	local found = false;
	for i = 1, #order do
		if order[i] == key then found = true; break end
	end
	if not found then order[#order+1] = key end
	tank.key_dir = d;
	return d;
end

function tank_key_up(key)
	local d = tank_key_dir[key];
	if d == nil then return nil end
	local order = tank.held_order;
	for i = #order, 1, -1 do
		if order[i] == key then table.remove(order, i); break end
	end
	if tank.key_dir == d then
		local last = order[#order];
		tank.key_dir = last and tank_key_dir[last] or nil;
	end
	return d;
end

function tank_respawn_player()
	local x, y = 4, tank_row-1;
	if not tank_cell_free(x, y, nil) then
		for d = 1, tank_col-1 do
			local nx = x + d;
			if nx < tank_col and tank_cell_free(nx, y, nil) then x = nx; break end
			nx = x - d;
			if nx >= 0 and tank_cell_free(nx, y, nil) then x = nx; break end
		end
	end
	if not tank_cell_free(x, y, nil) then return false end
	tank.player = { x = x, y = y, dir = 0, alive = true,
		move_cd = 0, fire_cd = 0, invuln_ms = tank_invuln_ms };
	tank_anim_snap(tank.player);
	tank.respawn_ms = 0;
	return true;
end

-- ==================== 数据层: 炮弹 ====================

function tank_count_bullets(owner)
	local n = 0;
	for _, b in ipairs(tank.bullets) do
		if not b.dead and b.owner == owner then n = n + 1 end
	end
	return n;
end

function tank_enemy_bullet_count(e)
	local n = 0;
	for _, b in ipairs(tank.bullets) do
		if not b.dead and b.from == e then n = n + 1 end
	end
	return n;
end

-- 炮弹出生:逻辑位置是坦克所在格,画面从坦克当前视觉点立即滑向前方一格;
-- 命中按画面位置判定(见 tank_bullet_touch),打到坦克/砖块的那一帧才出爆炸。
function tank_spawn_bullet(owner, ent)
	local b = { x = ent.x, y = ent.y, vx = ent.vx or ent.x, vy = ent.vy or ent.y,
		dir = ent.dir, owner = owner, from = ent, ms = 0, leg_ms = 0, dead = false };
	b.pvx = b.vx; b.pvy = b.vy;	-- 上一帧画面位置,用于本帧的线段命中检测
	tank.bullets[#tank.bullets+1] = b;
	tank_bullet_aim(b);
	return b;
end

-- 设定炮弹下一格;画面按实际距离换算同一飞行速度,开炮瞬间不会突然加速
function tank_bullet_aim(b)
	local nx = b.x + tank_dir_dx[b.dir+1];
	local ny = b.y + tank_dir_dy[b.dir+1];
	if not tank_in_bounds(nx, ny) then
		b.dead = true;
		return false;
	end
	b.tx = nx; b.ty = ny;
	local dist = math.abs(nx - (b.vx or b.x)) + math.abs(ny - (b.vy or b.y));
	if dist < 0.001 then dist = 1 end
	b.leg_ms = tank_bullet_ms * dist;
	b.ms = 0;
	tank_anim_start(b, b.leg_ms, nx, ny);
	return true;
end

function tank_player_fire()
	local p = tank.player;
	if p == nil or not p.alive or tank.state ~= "playing" then return false end
	if p.fire_cd > 0 then return false end
	if tank_count_bullets("player") >= tank_player_bullet_max then return false end
	p.fire_cd = tank_player_fire_cd;
	return tank_spawn_bullet("player", p) ~= nil;
end

function tank_enemy_fire(e)
	if tank.state ~= "playing" then return end
	if tank_enemy_bullet_count(e) > 0 then return end
	tank_spawn_bullet("enemy", e);
end

-- 爆炸按格子记录,同格重复命中只保留一个
function tank_add_explosion(x, y, dur)
	tank.explosions[y*tank_col+x] = { x = x, y = y, ms = dur, dur = dur };
end

-- 画面坐标(可带小数)换算到最近的格,爆炸/飞分按命中点落位
function tank_cell_of(vx, vy)
	local x = math.floor(vx + 0.5);
	local y = math.floor(vy + 0.5);
	if x < 0 then x = 0 elseif x > tank_col-1 then x = tank_col-1 end
	if y < 0 then y = 0 elseif y > tank_row-1 then y = tank_row-1 end
	return x, y;
end

function tank_kill_enemy(index, vx, vy)
	local e = tank.enemies[index];
	if e == nil then return end
	table.remove(tank.enemies, index);
	tank.killed = tank.killed + 1;
	local hx, hy = tank_cell_of(vx or e.vx or e.x, vy or e.vy or e.y);
	tank_add_explosion(hx, hy, 350);
	-- 敌坦克被毁时它自己发出的炮弹一并消失(标记删除,避免和炮弹遍历互相干扰)
	for _, b in ipairs(tank.bullets) do
		if b.from == e then b.dead = true end
	end
	tank_show_enemy();
	local pts = (e.kind == 2) and 200 or 100;
	tank_score_add(pts, hx, hy, (e.kind == 2) and "#F87171" or "#FFD700");
end

function tank_player_hit(vx, vy)
	local p = tank.player;
	if p == nil or not p.alive then return end
	if p.invuln_ms > 0 then return end
	local hx, hy = tank_cell_of(vx or p.vx or p.x, vy or p.vy or p.y);
	tank_add_explosion(hx, hy, 350);
	tank.player = nil;
	tank_keys_clear();
	tank.lives = tank.lives - 1;
	tank_show_lives();
	if tank.lives <= 0 then
		tank_game_over("tank");
	else
		tank.respawn_ms = tank_respawn_ms;
		tank_show_state();
	end
end

-- 本帧炮弹走过的线段(从上一帧画面位置到当前)是否扫到目标坦克;
-- 一帧跨多毫秒时也不会从坦克身上穿过去。返回命中点和 true。
function tank_bullet_contact(b, px, py)
	local ax, ay = b.pvx or b.vx, b.pvy or b.vy;
	local dx, dy = b.vx - ax, b.vy - ay;
	local len2 = dx*dx + dy*dy;
	local t = 0;
	if len2 > 0.000001 then
		t = ((px-ax)*dx + (py-ay)*dy) / len2;
		if t < 0 then t = 0 elseif t > 1 then t = 1 end
	end
	local cx, cy = ax + dx*t, ay + dy*t;
	if math.abs(px-cx) <= tank_hit_dist and math.abs(py-cy) <= tank_hit_dist then
		return cx, cy;
	end
	return nil;
end

-- 按画面位置判定子弹与坦克的接触:逻辑格坐标在滑动开始时就已更新到目标格,
-- 只有用 vx/vy 才能保证命中发生在子弹真正贴上坦克的那一帧。
function tank_bullet_touch(b)
	if b.owner == "player" then
		for i, e in ipairs(tank.enemies) do
			if e.alive and e ~= b.from then
				local ix, iy = tank_bullet_contact(b, e.vx or e.x, e.vy or e.y);
				if ix then
					tank_kill_enemy(i, ix, iy);
					return true;
				end
			end
		end
	else
		local p = tank.player;
		if p and p.alive and p ~= b.from then
			local ix, iy = tank_bullet_contact(b, p.vx or p.x, p.vy or p.y);
			if ix then
				if p.invuln_ms > 0 then
					local hx, hy = tank_cell_of(ix, iy);
					tank_add_explosion(hx, hy, 150);
				else
					tank_player_hit(ix, iy);
				end
				return true;
			end
		end
	end
	return false;
end

-- 炮弹画面到达目标格,此时再处理地形;坦克命中在 tank_bullet_touch 里按画面位置判定
function tank_bullet_arrive(b)
	local nx, ny = b.tx, b.ty;

	local t = tank.terrain[ny][nx];
	if t == tank_T_BRICK then
		tank.terrain[ny][nx] = tank_T_BRICK_HALF;
		tank_add_explosion(nx, ny, 150);
		return false
	elseif t == tank_T_BRICK_HALF then
		tank.terrain[ny][nx] = tank_T_EMPTY;
		tank_add_explosion(nx, ny, 150);
		return false
	elseif t == tank_T_STEEL then
		tank_add_explosion(nx, ny, 150);
		return false
	elseif t == tank_T_BASE then
		tank.terrain[ny][nx] = tank_T_BASE_DEAD;
		tank_add_explosion(nx, ny, 400);
		tank_show_base();
		tank_game_over("base");
		return false
	end

	b.x = nx; b.y = ny;
	return tank_bullet_aim(b);	-- 继续滑向下一格
end

-- ==================== 数据层: 敌人 AI / 出生 ====================

function tank_enemy_choose_dir(e)
	local tx, ty;
	local p = tank.player;
	if p and p.alive and math.random() < 0.45 then
		tx, ty = p.x, p.y;
	else
		tx, ty = math.floor(tank_col/2), tank_row-1;
	end
	local dx = tx - e.x;
	local dy = ty - e.y;
	local candidates = {};
	if math.abs(dx) > math.abs(dy) then
		candidates[#candidates+1] = (dx > 0) and 1 or 3;
		candidates[#candidates+1] = (dy > 0) and 2 or 0;
	else
		candidates[#candidates+1] = (dy > 0) and 2 or 0;
		candidates[#candidates+1] = (dx > 0) and 1 or 3;
	end
	candidates[#candidates+1] = (e.dir + 2) % 4;	-- 兜底掉头
	for _, d in ipairs(candidates) do
		local nx = e.x + tank_dir_dx[d+1];
		local ny = e.y + tank_dir_dy[d+1];
		if tank_cell_free(nx, ny, e) then return d end
	end
	return math.random(0, 3);
end

function tank_enemy_move(e)
	local dir = e.dir;
	if math.random() < 0.30 then dir = tank_enemy_choose_dir(e) end
	if not tank_move_entity(e, dir, e.speed_ms) then
		tank_move_entity(e, tank_enemy_choose_dir(e), e.speed_ms);
	end
end

function tank_enemy_update(e, dt)
	e.move_ms = e.move_ms - dt;
	if e.move_ms <= 0 then
		e.move_ms = e.speed_ms;	-- 与滑行时长一致,连续推进不停顿
		tank_enemy_move(e);
	end
	e.fire_ms = e.fire_ms - dt;
	if e.fire_ms <= 0 then
		e.fire_ms = math.max(700, 1800 - tank.stage*120) + math.random(0, 900);
		tank_enemy_fire(e);
	end
end

function tank_try_spawn_enemy()
	if tank.spawned >= tank.total then return false end
	if #tank.enemies >= tank_enemy_max then return false end
	local sp = tank_spawn_pts[(tank.spawn_idx % #tank_spawn_pts) + 1];
	tank.spawn_idx = tank.spawn_idx + 1;
	if not tank_cell_free(sp[1], sp[2], nil) then return false end

	local kind = 1;
	if tank.stage >= 2 and math.random() < 0.35 then kind = 2 end
	local speed = math.max(280, 520 - (tank.stage-1)*40) + math.random(0, 80);
	local e = { x = sp[1], y = sp[2], dir = 2, kind = kind, alive = true,
		move_ms = speed, speed_ms = speed,
		fire_ms = 1000 + math.random(0, 900) };
	tank_anim_snap(e);
	tank.enemies[#tank.enemies+1] = e;
	tank.spawned = tank.spawned + 1;
	tank_add_explosion(sp[1], sp[2], 180);
	return true;
end

-- ==================== 数据层: 每帧推进 ====================

function tank_advance_visuals(dt)
	local p = tank.player;
	if p then tank_anim_advance(p, dt) end
	for _, e in ipairs(tank.enemies) do tank_anim_advance(e, dt) end
	for _, b in ipairs(tank.bullets) do
		b.pvx = b.vx; b.pvy = b.vy;
		tank_anim_advance(b, dt);
	end
end

function tank_tick(dt)
	tank.time_ms = tank.time_ms + dt;

	tank_advance_visuals(dt);	-- 先把画面位置推进到本帧,逻辑再决定下一次移动

	for k, ex in pairs(tank.explosions) do
		ex.ms = ex.ms - dt;
		if ex.ms <= 0 then tank.explosions[k] = nil end
	end

	if tank.state == "stageclear" then
		tank.clear_ms = tank.clear_ms - dt;
		if tank.clear_ms <= 0 then tank_next_stage() end
		return;
	end
	if tank.state ~= "playing" then return end

	local p = tank.player;
	if p then
		if p.move_cd > 0 then p.move_cd = p.move_cd - dt end
		if p.fire_cd > 0 then p.fire_cd = p.fire_cd - dt end
		if p.invuln_ms > 0 then
			p.invuln_ms = p.invuln_ms - dt;
			if p.invuln_ms < 0 then p.invuln_ms = 0 end
		end
		-- 按住方向键时由定时器按固定节奏放行,不依赖系统按键重复
		if p.alive and tank.key_dir ~= nil and p.move_cd <= 0 then
			tank_player_move(tank.key_dir);
		end
	elseif tank.respawn_ms > 0 then
		tank.respawn_ms = tank.respawn_ms - dt;
		if tank.respawn_ms <= 0 then
			tank_respawn_player();
			tank_show_state();
		end
	end

	tank.spawn_ms = tank.spawn_ms - dt;
	if tank.spawn_ms <= 0 then
		tank.spawn_ms = 1400;
		tank_try_spawn_enemy();
	end

	for i = #tank.enemies, 1, -1 do
		local e = tank.enemies[i];
		if e then tank_enemy_update(e, dt) end
	end

	-- 倒序遍历,便于删除消失的炮弹;dead 炮弹在推进前清掉
	for i = #tank.bullets, 1, -1 do
		local b = tank.bullets[i];
		if b.dead then
			table.remove(tank.bullets, i);
		elseif tank_bullet_touch(b) then
			table.remove(tank.bullets, i);
		else
			b.ms = b.ms + dt;
			if b.ms >= b.leg_ms then
				if not tank_bullet_arrive(b) then table.remove(tank.bullets, i) end
			end
		end
		if tank.state ~= "playing" then break end
	end

	if tank.state == "playing" and tank.killed >= tank.total and #tank.enemies == 0 then
		tank_stage_clear();
	end
end

function tank_stage_clear()
	tank.state = "stageclear";
	tank.clear_ms = 1800;
	tank_banner_show("第 " .. tank.stage .. " 关完成  +500");
	tank_score_add(500);
	tank_set_start_btn(T"开始游戏");
	tank_show_state();
end

function tank_next_stage()
	tank.stage = tank.stage + 1;
	tank_setup_stage();
end

-- ==================== 渲染 ====================
-- 棋盘 169 格只画地形与爆炸;坦克/炮弹/护盾是棋盘上方精灵层里的独立 img,
-- 按 vx/vy(可带小数的格坐标)每帧 Move,于是移动是连续滑动而不是逐格跳。

function tank_set_frame(id, frame)
	if tank.cache[id] == frame then return end
	tank.cache[id] = frame;
	local ele = tank.wndBoard and tank.wndBoard:FindChildByID(id, -1);
	if ele then
		ele:SetAttribute(T"iconIndex", T(tostring(frame)), false);
		ele:Invalidate();
	end
end

-- 棋盘格子尺寸/间距/原点,布局变化(窗口缩放)后自动重建精灵
function tank_ensure_geo()
	if tank.wndBoard == nil or tank.sprLayer == nil then return false end
	local c0 = tank.wndBoard:FindChildByID(tank_cellid(0, 0), -1);
	local c1 = tank.wndBoard:FindChildByID(tank_cellid(1, 0), -1);
	local c2 = tank.wndBoard:FindChildByID(tank_cellid(0, 1), -1);
	if c0 == nil or c1 == nil or c2 == nil then return false end
	local r0 = c0:GetWindowRect2();
	local r1 = c1:GetWindowRect2();
	local r2 = c2:GetWindowRect2();
	local cw = r0:Width(); local ch = r0:Height();
	if cw <= 0 or ch <= 0 then return false end
	local px = r1.left - r0.left;
	local py = r2.top - r0.top;
	if px <= 0 or py <= 0 then return false end

	-- 格子坐标相对棋盘客户区;精灵层与棋盘是同一父窗口下的兄弟,
	-- 把精灵层贴到棋盘位置,两者原点一致,Move 时就能直接用格子坐标。
	local rb = tank.wndBoard:GetWindowRect2();
	local lx, ly = rb.left, rb.top;
	local lw, lh = rb:Width(), rb:Height();
	tank.sprLayer:Move(CRect(lx, ly, lx + lw, ly + lh));

	local g = tank.geo;
	if g and g.ox == r0.left and g.oy == r0.top and g.px == px
		and g.py == py and g.cw == cw and g.ch == ch then
		return true;
	end
	tank.geo = { ox = r0.left, oy = r0.top, px = px, py = py, cw = cw, ch = ch };
	tank_sprite_pool_init();
	return true;
end

function tank_sprite_xml(id, w, h, frame)
	return '<img id="' .. id .. '" size="' .. w .. ',' .. h .. '" skin="svg_tank" iconIndex="'
		.. frame .. '" msgTransparent="1" visible="0"/>';
end

-- 精灵池一次建好,顺序决定 z 序:护盾环 -> 玩家 -> 敌人 -> 炮弹
function tank_sprite_pool_init()
	local layer = tank.sprLayer;
	local g = tank.geo;
	if layer == nil or g == nil then return end
	layer:DestroyAllChildren();
	tank.sprCache = {}; tank.sprEle = {};
	local xml = tank_sprite_xml(tank_spr_base_id + 1, g.cw, g.ch, tank_F_SHIELD);
	xml = xml .. tank_sprite_xml(tank_spr_base_id + 2, g.cw, g.ch, tank_F_PLAYER);
	for i = 1, tank_enemy_max do
		xml = xml .. tank_sprite_xml(tank_spr_base_id + 10 + i, g.cw, g.ch, tank_F_ENEMY);
	end
	for i = 1, tank_bullet_pool do
		xml = xml .. tank_sprite_xml(tank_spr_base_id + 20 + i, g.cw, g.ch, tank_F_BULLET_P);
	end
	layer:CreateChildrenFromXml(xml);
end

function tank_spr_ele(id)
	local ele = tank.sprEle[id];
	if ele == nil then
		ele = tank.sprLayer and tank.sprLayer:FindChildByID(id, -1) or false;
		tank.sprEle[id] = ele;
	end
	if ele == false then return nil end
	return ele;
end

-- scale/alpha 可选:护盾环靠两者做缓慢呼吸
function tank_spr_set(id, frame, fx, fy, scale, alpha)
	local ele = tank_spr_ele(id);
	local g = tank.geo;
	if ele == nil or g == nil then return end
	local sc = scale or 1;
	local w = math.floor(g.cw * sc + 0.5);
	local h = math.floor(g.ch * sc + 0.5);
	local px = math.floor(g.ox + fx*g.px + (g.cw - w)/2 + 0.5);
	local py = math.floor(g.oy + fy*g.py + (g.ch - h)/2 + 0.5);
	local a = alpha or 255;
	if a < 0 then a = 0 elseif a > 255 then a = 255 end
	local c = tank.sprCache[id];
	if c == nil then
		c = { frame = -1, x = -1, y = -1, w = -1, h = -1, a = -1 };
		tank.sprCache[id] = c;
	end
	if c.frame ~= frame then
		ele:SetAttribute(T"iconIndex", T(tostring(frame)), false);
		c.frame = frame;
	end
	if c.x ~= px or c.y ~= py or c.w ~= w or c.h ~= h then
		ele:Move(CRect(px, py, px + w, py + h));
		c.x = px; c.y = py; c.w = w; c.h = h;
	end
	if c.a ~= a then
		ele:SetAlpha(a);
		c.a = a;
	end
	if ele:IsVisible() == 0 then
		ele:SetVisible(true, true);
	end
end

function tank_spr_hide(id)
	local ele = tank_spr_ele(id);
	if ele and ele:IsVisible() ~= 0 then ele:SetVisible(false, true) end
	tank.sprCache[id] = nil;	-- 下次显示按新位置重新落位
end

-- 只对精灵本身做绕中心的矩阵旋转(护盾环);alpha 仍由 SetAlpha 控制
-- SOUI 矩阵按行向量存储: {Sx, SkewX, Tx, SkewY, Sy, Ty, P0, P1, P2}
function tank_spr_rot(id, deg)
	local ele = tank_spr_ele(id);
	local c = tank.sprCache[id];
	if ele == nil or c == nil then return end
	local r = deg * math.pi / 180;
	local cs = math.cos(r); local sn = math.sin(r);
	local cx = c.w / 2; local cy = c.h / 2;
	local tx = cx * (1 - cs) + sn * cy;
	local ty = cy * (1 - cs) - sn * cx;
	ele:SetMatrix({ cs, -sn, tx, sn, cs, ty, 0, 0, 1 }, true);
end

function tank_render_sprites()
	-- 玩家:无敌时叠加一个缓慢呼吸+旋转的护盾环,坦克本身始终可见(不闪);
	-- 无敌结束前 400ms 平滑淡出,避免护盾突然消失
	local p = tank.player;
	if p and p.alive then
		if p.invuln_ms > 0 then
			local bp = (tank.time_ms % tank_shield_breath_ms) / tank_shield_breath_ms;
			local k = 0.5 - 0.5 * math.cos(bp * 2 * math.pi);	-- 0..1 平滑起伏
			local a = math.floor(215 + 40 * k);
			if p.invuln_ms < 400 then a = math.floor(a * p.invuln_ms / 400) end
			-- 比坦克大一圈,罩在坦克外面才看得清
			tank_spr_set(tank_spr_base_id + 1, tank_F_SHIELD, p.vx, p.vy,
				1.3 + 0.08 * k, a);
			tank_spr_rot(tank_spr_base_id + 1,
				(tank.time_ms % tank_shield_spin_ms) / tank_shield_spin_ms * 360);
		else
			tank_spr_hide(tank_spr_base_id + 1);
		end
		tank_spr_set(tank_spr_base_id + 2, tank_F_PLAYER + p.dir, p.vx, p.vy);
	else
		tank_spr_hide(tank_spr_base_id + 1);
		tank_spr_hide(tank_spr_base_id + 2);
	end

	for i = 1, tank_enemy_max do
		local e = tank.enemies[i];
		if e then
			local frame = (e.kind == 2) and (tank_F_ENEMY_FAST + e.dir) or (tank_F_ENEMY + e.dir);
			tank_spr_set(tank_spr_base_id + 10 + i, frame, e.vx, e.vy);
		else
			tank_spr_hide(tank_spr_base_id + 10 + i);
		end
	end

	for i = 1, tank_bullet_pool do
		local b = tank.bullets[i];
		if b and not b.dead then
			local frame = (b.owner == "player") and tank_F_BULLET_P or tank_F_BULLET_E;
			tank_spr_set(tank_spr_base_id + 20 + i, frame, b.vx, b.vy);
		else
			tank_spr_hide(tank_spr_base_id + 20 + i);
		end
	end
end

function tank_render()
	if tank.wndBoard == nil or tank.terrain == nil then return end

	-- 静态层:地形,爆炸按格子覆盖在上面(爆炸本身就是格子大小的爆点)
	local overlay = {};
	for key, ex in pairs(tank.explosions) do
		local t = 1 - ex.ms/ex.dur;
		local frame = tank_F_EXP1;
		if t >= 0.72 then frame = tank_F_EXP3
		elseif t >= 0.36 then frame = tank_F_EXP2 end
		overlay[key] = frame;
	end
	for y = 0, tank_row-1 do
		for x = 0, tank_col-1 do
			local key = y*tank_col + x;
			local frame = overlay[key];
			if frame == nil then frame = tank.terrain[y][x] end
			tank_set_frame(tank_cellid(x, y), frame);
		end
	end

	if tank_ensure_geo() then tank_render_sprites() end
end

-- ==================== UI: LED / 文本 / 提示条 ====================

function tank_set_digits(prefix, num, ndigits)
	if tank.root == nil then return end
	for i = 0, ndigits-1 do
		local stack = tank.root:FindChildByNameA(prefix .. "_" .. i, -1);
		if stack then
			local stackApi = QiIStackView(stack);
			if stackApi then
				stackApi:SelectPage(math.floor(num / 10^i) % 10, false);
				stackApi:Release();
			end
		end
	end
end

function tank_show_score() tank_set_digits("digit_tank_score", (tank.score_shown or 0) % 10000, 4) end
function tank_show_lives() tank_set_digits("digit_tank_lives", tank.lives % 10, 1) end
function tank_show_stage() tank_set_digits("digit_tank_stage", tank.stage % 100, 2) end
function tank_show_enemy()
	local left = tank.total - tank.killed;
	if left < 0 then left = 0 end
	tank_set_digits("digit_tank_enemy", left % 100, 2);
end

function tank_show_state()
	if tank.root == nil then return end
	local txt = tank.root:FindChildByNameA("txt_tank_state",-1);
	if txt == nil then return end
	local s = { idle = "待开始", playing = "战斗中", paused = "已暂停",
		stageclear = "关卡完成", over = "战斗结束" };
	local name = s[tank.state] or "";
	if tank.state == "playing" and tank.player == nil then name = "等待重生" end
	txt:SetWindowText(T("状态:" .. name));
end

function tank_show_base()
	if tank.root == nil then return end
	local txt = tank.root:FindChildByNameA("txt_tank_base",-1);
	if txt == nil then return end
	local alive = true;
	for y = 0, tank_row-1 do
		for x = 0, tank_col-1 do
			if tank.terrain[y][x] == tank_T_BASE_DEAD then alive = false end
		end
	end
	txt:SetWindowText(T(alive and "基地完好" or "基地已毁"));
end

function tank_set_start_btn(text)
	if tank.root == nil then return end
	local btn = tank.root:FindChildByNameA("btn_tank_start",-1);
	if btn then btn:SetWindowText(text) end
end

function tank_banner_show(text)
	if tank.root == nil then return end
	local wnd = tank.root:FindChildByNameA("wnd_tank_banner",-1);
	local txt = tank.root:FindChildByNameA("txt_tank_banner",-1);
	if wnd and txt then
		txt:SetWindowText(T(text));
		wnd:SetVisible(true, true);
	end
end

function tank_banner_hide()
	if tank.root == nil then return end
	local wnd = tank.root:FindChildByNameA("wnd_tank_banner",-1);
	if wnd then wnd:SetVisible(false, true) end
end

-- ==================== 动画: 击杀飞分 + 得分 LED 滚动 ====================

function tank_new_ani_ctx(ctx)
	tank.fx_seq = tank.fx_seq + 1;
	tank.ani_ctx[tank.fx_seq] = ctx;
	return tank.fx_seq;
end

function tank_get_cell_rect(x, y)
	if tank.wndBoard == nil then return nil end
	local ele = tank.wndBoard:FindChildByID(tank_cellid(x, y), -1);
	if ele == nil then return nil end
	return ele:GetWindowRect2();
end

function tank_get_led_rect(prefix, ndigits)
	local l, t, r, b;
	for i = 0, ndigits-1 do
		local d = tank.root and tank.root:FindChildByNameA(prefix .. "_" .. i, -1);
		if d then
			local rc = d:GetWindowRect2();
			l = (l == nil) and rc.left or math.min(l, rc.left);
			t = (t == nil) and rc.top or math.min(t, rc.top);
			r = (r == nil) and rc.right or math.max(r, rc.right);
			b = (b == nil) and rc.bottom or math.max(b, rc.bottom);
		end
	end
	if l == nil then return nil end
	return CRect(l, t, r, b);
end

-- 加分:从击杀格弹 "+分数" 飞向得分 LED;浮层不可用时直接滚动计数
function tank_score_add(pts, x, y, color)
	tank.score = tank.score + pts;
	if x ~= nil and tank_fly_text(x, y, "+" .. pts, color) then return end
	tank_score_count_begin();
end

function tank_fly_text(x, y, text, colorText)
	if tank.aniframe == nil or tank.ani_move == nil then return false end
	local rcFrom = tank_get_cell_rect(x, y);
	local rcTo = tank_get_led_rect("digit_tank_score", 4);
	if rcFrom == nil or rcTo == nil then return false end

	tank.fx_seq = tank.fx_seq + 1;
	local fid = tank_fx_base_id + tank.fx_seq;
	local w = 64; local h = 24;
	local px = rcFrom.left + math.floor(rcFrom:Width()/2) - math.floor(w/2);
	local py = rcFrom.top + math.floor(rcFrom:Height()/2) - math.floor(h/2);
	local xml = '<text id="' .. fid .. '" text="' .. text .. '" font="size:18,bold:1"'
		.. ' colorText="' .. colorText .. '" align="center" valign="center" msgTransparent="1"/>';
	tank.aniframe:CreateChildrenFromXml(xml);
	local fy = tank.aniframe:FindChildByID(fid, -1);
	if fy == nil then return false end
	fy:Move(CRect(px, py, px + w, py + h));

	local tx = rcTo.left + math.floor(rcTo:Width()/2) - math.floor(w/2);
	local ty = rcTo.top + math.floor(rcTo:Height()/2) - math.floor(h/2);
	local rc1 = CRect(px, py, px + w, py + h);
	local rc2 = CRect(tx, ty, tx + w, ty + h);
	local arc = math.max(40, math.min(120, math.abs(tx - px) * 0.3));

	local ani = LuaValueAnimator();
	ani:CopyFrom(tank.ani_move:GetIValueAnimator());	-- 模板无插值器 => fraction 即线性 t
	ani:GetIValueAnimator():setDuration(450);
	ani:SetRangeRect(rc1, rc2);
	ani:SetOnUpdate("tank_fly_update");
	ani:SetOnEnd("tank_fly_end");
	local ctxId = tank_new_ani_ctx({ kind = "score", widget = fy, r1 = rc1, r2 = rc2, arc = arc, ani = ani });
	ani:SetCtx(ctxId);
	local ok = ani:Start(tank.aniframe);
	if ok == 0 or ok == false then
		tank.ani_ctx[ctxId] = nil;
		fy:Destroy();
		return false
	end
	return true
end

function tank_fly_update(luaAni, ctxId)
	local c = tank.ani_ctx[ctxId];
	if c == nil or c.widget == nil then return end
	local t = luaAni:GetFraction();
	local x = c.r1.left + (c.r2.left - c.r1.left) * t;
	local y = c.r1.top + (c.r2.top - c.r1.top) * t - c.arc * 4 * t * (1 - t);
	local l = math.floor(x); local tp = math.floor(y);
	c.widget:Move(CRect(l, tp, l + c.r1:Width(), tp + c.r1:Height()));
end

function tank_fly_end(luaAni, ctxId)
	local c = tank.ani_ctx[ctxId];
	if c == nil then return end
	if c.widget then c.widget:Destroy() end
	tank.ani_ctx[ctxId] = nil;
	tank_score_count_begin();	-- 落地后 LED 才滚动到新分数
end

-- 得分 LED 滚动(重入时废弃旧段,从当前显示值滚向最新分数)
function tank_score_count_begin()
	if tank.root == nil or tank.aniframe == nil or tank.ani_move == nil then
		tank.score_shown = tank.score;
		tank_show_score();
		return
	end
	if tank.score_shown == tank.score then return end
	tank_score_count_abort();
	local ani = LuaValueAnimator();
	ani:CopyFrom(tank.ani_move:GetIValueAnimator());
	ani:GetIValueAnimator():setDuration(320);
	ani:SetRangeRect(CRect(0,0,1,1), CRect(0,0,1,1));
	ani:SetOnUpdate("tank_score_count_update");
	ani:SetOnEnd("tank_score_count_end");
	local ctxId = tank_new_ani_ctx({ kind = "count", ani = ani,
		from = tank.score_shown, to = tank.score });
	tank.count_ctx = ctxId;
	ani:SetCtx(ctxId);
	local ok = ani:Start(tank.aniframe);
	if ok == 0 or ok == false then
		tank.ani_ctx[ctxId] = nil;
		tank.count_ctx = 0;
		tank.score_shown = tank.score;
		tank_show_score();
	end
end

function tank_score_count_abort()
	if tank.count_ctx and tank.count_ctx ~= 0 then
		tank.ani_ctx[tank.count_ctx] = nil;
		tank.count_ctx = 0;
	end
end

function tank_score_count_update(luaAni, ctxId)
	local c = tank.ani_ctx[ctxId];
	if c == nil then return end
	local v = math.floor(c.from + (c.to - c.from) * luaAni:GetFraction() + 0.5);
	if v ~= tank.score_shown then
		tank.score_shown = v;
		tank_show_score();
	end
end

function tank_score_count_end(luaAni, ctxId)
	local c = tank.ani_ctx[ctxId];
	if c == nil then return end
	tank.ani_ctx[ctxId] = nil;
	if tank.count_ctx == ctxId then tank.count_ctx = 0 end
	tank.score_shown = tank.score;
	tank_show_score();
end

function tank_fx_clear()
	tank_score_count_abort();	-- 旧滚动段 end 会把显示值拉回旧分数
	for _, c in pairs(tank.ani_ctx) do
		if c.widget then c.widget:Destroy() end
	end
	tank.ani_ctx = {};
end

-- ==================== 流程: 开局 / 暂停 / 通关 / 结束 ====================

function tank_setup_stage()
	tank_fx_clear();
	tank_keys_clear();
	tank.terrain = tank_parse_map(tank_map_at(tank.stage));
	tank.bullets = {};
	tank.explosions = {};
	tank.enemies = {};
	tank.spawned = 0;
	tank.killed = 0;
	tank.total = math.min(20, 8 + tank.stage*2);
	tank.spawn_idx = 0;
	tank.spawn_ms = 800;
	tank.respawn_ms = 0;
	tank.time_ms = 0;
	tank.score_shown = tank.score;
	tank.cache = {};
	tank.over_reason = nil;
	tank.state = "playing";
	tank.pausedFrom = nil;
	tank.player = nil;
	tank_respawn_player();
	tank_banner_hide();
	tank_set_start_btn(T"暂 停");
	tank_show_score(); tank_show_lives(); tank_show_stage();
	tank_show_enemy(); tank_show_state(); tank_show_base();
	tank_try_spawn_enemy();
	tank_try_spawn_enemy();
	tank_render();
end

function tank_start()
	slog("TANK start");
	tank_over_hide();
	tank.score = 0; tank.score_shown = 0;
	tank.lives = 3; tank.stage = 1;
	tank_setup_stage();
	if tank.timer then
		tank.timer:KillTimer();
		tank.timer:StartTimer(tank_tick_ms, 1, 0);
	end
end

function tank_pause(byPage)
	if tank.state ~= "playing" and tank.state ~= "stageclear" then return end
	tank.pausedFrom = tank.state;
	tank.state = "paused";
	tank.pausedByPage = byPage and true or false;
	tank_keys_clear();
	if tank.timer then tank.timer:KillTimer() end
	tank_set_start_btn(T"继 续");
	tank_show_state();
end

function tank_resume()
	if tank.state ~= "paused" then return end
	tank.state = tank.pausedFrom or "playing";
	tank.pausedFrom = nil;
	tank.pausedByPage = nil;
	if tank.timer then tank.timer:StartTimer(tank_tick_ms, 1, 0) end
	tank_set_start_btn(T"暂 停");
	tank_show_state();
end

function tank_game_over(reason)
	if tank.state == "over" then return end
	tank.state = "over";
	tank.over_reason = reason;
	tank.pausedFrom = nil;
	tank_keys_clear();
	tank_banner_hide();
	tank_set_start_btn(T"开始游戏");
	tank_show_state();
	tank_show_base();
	tank_render();
	tank_over_show();
end

-- ==================== 结束窗口(布局内静态遮罩 + 卡片,动画走 anim 模板) ====================

function tank_over_wnd()
	if tank.over_wnd == nil and tank.root ~= nil then
		tank.over_wnd = tank.root:FindChildByNameA("wnd_tank_gameover",-1);
	end
	return tank.over_wnd;
end

function tank_over_ani_play(maskKey, cardKey, maskName, cardName)
	local wnd = tank_over_wnd();
	if wnd == nil then return end
	if tank[maskKey] == nil then
		tank[maskKey] = GetApp():LoadAnimation("anim:" .. maskName);
	end
	if tank[maskKey] then
		local ani = tank[maskKey]:clone();
		wnd:SetAnimation(ani);
		ani:Release();
	end
	local card = wnd:FindChildByNameA("wnd_tank_gameover_card",-1);
	if card == nil then return end
	if tank[cardKey] == nil then
		tank[cardKey] = GetApp():LoadAnimation("anim:" .. cardName);
	end
	if tank[cardKey] then
		local ani = tank[cardKey]:clone();
		card:SetAnimation(ani);
		ani:Release();
	end
end

function tank_on_over_stop(args)
	local evt = toStEventSwndAnimationStop(args);
	if evt == nil then return 0 end
	local hiding = tank.over_hiding;
	tank.over_hiding = false;
	if hiding then
		local wnd = tank_over_wnd();
		if wnd then wnd:SetVisible(false, true) end
	end
	return 0;
end

function tank_over_show()
	local wnd = tank_over_wnd();
	if wnd == nil then return end
	local txt = tank.root and tank.root:FindChildByNameA("txt_tank_gameover_score",-1);
	if txt then txt:SetWindowText(T("本局得分 " .. tank.score)) end
	local reason = tank.root and tank.root:FindChildByNameA("txt_tank_gameover_reason",-1);
	if reason then
		reason:SetWindowText(T(tank.over_reason == "base" and "基地被摧毁" or "坦克全部损毁"));
	end
	if tank.over_visible then return end
	tank_over_ani_play("over_ani_mask_in", "over_ani_in",
		"tank_over_mask_in", "tank_over_card_in");
	wnd:SetVisible(true, true);
	tank.over_visible = true;
end

function tank_over_hide()
	local wnd = tank_over_wnd();
	if wnd == nil then return end
	if not tank.over_visible then
		wnd:SetVisible(false, true);
		return
	end
	tank.over_visible = false;
	tank.over_hiding = true;	-- out 动画结束后 tank_on_over_stop 里隐藏
	tank_over_ani_play("over_ani_mask_out", "over_ani_out",
		"tank_over_mask_out", "tank_over_card_out");
end

-- ==================== 事件: 计时器 / 键盘 / 按钮 / 子页可见性 ====================

function tank_on_timer(args)
	if tank.state == "playing" or tank.state == "stageclear" then
		tank_tick(tank_tick_ms);
		tank_render();
	elseif tank.state == "over" then
		-- 结束后只推进爆炸动画,播完停表(避免基地爆炸定格在首帧)
		local any = false;
		for k, ex in pairs(tank.explosions) do
			ex.ms = ex.ms - tank_tick_ms;
			if ex.ms <= 0 then tank.explosions[k] = nil else any = true end
		end
		tank_render();
		if not any and tank.timer then tank.timer:KillTimer() end
	end
	return 0;
end

function tank_on_key(args)
	if tank.wndBoard == nil then return 0 end
	local evt = toStEventKeyDown(args);
	if evt == nil then return 0 end
	local key = evt.nChar;
	if tank.state == "playing" then
		local dir = tank_key_dir[key];
		if dir ~= nil then
			tank_key_down(key);
			tank_player_move(dir);
			return 1;
		elseif key == 32 then					-- 空格开炮
			tank_player_fire();
			return 1;
		elseif key == 80 then					-- P 暂停
			tank_pause();
			return 1;
		end
	elseif tank.state == "paused" and key == 80 then
		tank_resume();
		return 1;
	elseif (tank.state == "idle" or tank.state == "over") and key == 32 then
		tank_start();
		return 1;
	end
	return 0;
end

-- 松键后停止持续移动;多个方向键同时按住时回退到仍按住的最后一个方向
function tank_on_keyup(args)
	local evt = toStEventKeyUp(args);
	if evt == nil then return 0 end
	tank_key_up(evt.nChar);
	return 0;
end

function tank_on_start(args)
	if tank.wndBoard == nil then return 0 end
	slog("TANK on_start state=" .. tostring(tank.state));
	if tank.state == "playing" or tank.state == "stageclear" then
		tank_pause();
	elseif tank.state == "paused" then
		tank_resume();
	else
		tank_start();
	end
	tank.wndBoard:SetFocus();
	return 1;
end

function tank_on_restart(args)
	if tank.wndBoard == nil then return 0 end
	tank_start();
	tank.wndBoard:SetFocus();
	return 1;
end

function tank_on_page_vis(args)
	if tank.wndBoard == nil then return 0 end
	local evt = toStEventSwndVisibleChanged(args);
	if evt == nil then return 0 end
	slog("TANK page_vis visible=" .. tostring(evt.bVisible));
	if evt.bVisible == 0 then
		if tank.state == "playing" or tank.state == "stageclear" then
			tank_pause(true);	-- 离开子页自动暂停,回来时自动继续
		end
	else
		-- 子页刚显示时窗口才完成布局,棋盘几何此时才有效,重画一次让精灵就位
		tank_render();
		if tank.state == "paused" and tank.pausedByPage then
			tank_resume();
		end
	end
	return 0;
end

-- ==================== 初始化 / 退出 ====================

function tank_init_board()
	tank.wndBoard:DestroyAllChildren();
	tank.cache = {};
	local xml = "";
	local rows = tank_row + tank_pad_rows*2;
	for y = 0, rows-1 do
		local frame = 0;
		if y < tank_pad_rows or y >= tank_pad_rows + tank_row then
			frame = tank_T_BRICK;	-- 上下装饰行直接铺满砖墙,和地图里的障碍同帧
		end
		for x = 0, tank_col-1 do
			xml = xml .. '<img id="' .. (tank_base_id + y*tank_col + x)
				.. '" size="' .. tank_cell .. ',' .. tank_cell
				.. '" skin="svg_tank" iconIndex="' .. frame .. '"/>';
		end
	end
	tank.wndBoard:CreateChildrenFromXml(xml);
	tank.wndBoard:RequestRelayout();
end

function tank_init(root)
	local wndBoard = root:FindChildByNameA("wnd_tank_board",-1);
	if wndBoard == nil then
		return 0;	-- demo 不含坦克大战子页,跳过
	end
	tank.root = root;
	tank.wndBoard = wndBoard;
	-- 棋盘上方的精灵层:坦克/炮弹/护盾环按像素位置绘制,不参与布局
	tank.sprLayer = root:FindChildByNameA("wnd_tank_sprites",-1);
	if tank.sprLayer == nil then
		tank_slog("init: wnd_tank_sprites not found");
	else
		tank.sprLayer:SetVisible(true, true);	-- stack 只显示第一页,精灵层要单独打开
	end
	-- 动画浮层与 animator 模板缺失只影响飞分/滚动动画,游戏本体照常
	tank.aniframe = root:FindChildByNameA("wnd_tank_aniframe",-1);
	if tank.aniframe then
		tank.ani_move = LuaValueAnimator();
		if not tank.ani_move:LoadAnimator("animator:xxl_move") then
			tank_slog("init: load animator:xxl_move failed");
			tank.ani_move = nil;
		end
	end

	local souiFac = CreateSouiFactory();
	local timerSlot = CreateEventSlot("tank_on_timer");
	tank.timer = souiFac:CreateTimer(timerSlot);
	timerSlot:Release();
	souiFac:Release();

	tank_init_board();
	local keySlot = CreateEventSlot("tank_on_key");
	wndBoard:SubscribeEvent(8200, keySlot);	-- 8200 == EVT_KEYDOWN
	keySlot:Release();
	local keyUpSlot = CreateEventSlot("tank_on_keyup");
	wndBoard:SubscribeEvent(8201, keyUpSlot);	-- 8201 == EVT_KEYUP
	keyUpSlot:Release();

	-- 进入子页先展示地图与空战场,点"开始游戏"才真正开局
	tank.score = 0; tank.score_shown = 0; tank.lives = 3; tank.stage = 1;
	tank.total = math.min(20, 8 + tank.stage*2);
	tank.killed = 0; tank.spawned = 0; tank.spawn_idx = 0;
	tank.player = nil; tank.enemies = {}; tank.bullets = {}; tank.explosions = {};
	tank.terrain = tank_parse_map(tank_map_at(tank.stage));
	tank.state = "idle";
	tank.cache = {};
	tank_show_score(); tank_show_lives(); tank_show_stage();
	tank_show_enemy(); tank_show_state(); tank_show_base();
	tank_render();
	tank_slog("init done");
	return 1;
end

function tank_exit()
	tank_keys_clear();
	if tank.timer then
		tank.timer:KillTimer();
		tank.timer:Release();
		tank.timer = nil;
	end
	if tank.over_ani_mask_in then
		tank.over_ani_mask_in:Release();
		tank.over_ani_mask_in = nil;
	end
	if tank.over_ani_mask_out then
		tank.over_ani_mask_out:Release();
		tank.over_ani_mask_out = nil;
	end
	if tank.over_ani_in then
		tank.over_ani_in:Release();
		tank.over_ani_in = nil;
	end
	if tank.over_ani_out then
		tank.over_ani_out:Release();
		tank.over_ani_out = nil;
	end
	tank.ani_ctx = {};
end

