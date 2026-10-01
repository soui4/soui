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
  - settle 稳定后检测无步可走 => 自动洗牌(xxl_redeal,带翻页动画);
  - 提示按钮(xxl_on_hint):找一个可行交换,两枚棋子复用选中脉冲动画;
  - 金币耗尽 => 游戏结束弹窗(SMessageBox),确定后重开。
棋盘 id 约定: xxl_base_id + y*7 + x (x,y 从 0 开始)。
]]

xxl_base_id = 30000;
xxl_row = 7; xxl_col = 7;
xxl_max_state = 7; xxl_min_same = 3;

xxl = {
	board = {};      -- board[y][x] = icon state(0..6)
	click_id = -1;
	coin = 20; score = 0;
	combo = 0;       -- 连击数(第 n 波消除得分 x n,settle 清零)
	gameover = false;-- 金币耗尽弹窗只出一次
	ani_list = {};   -- ctxId -> 动画组(js ani_list;按 ctxId 索引,勿用对象身份比较)
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
	xxl_set_digits("digit_coin", xxl.coin % 1000);
end

-- 连击提示文本(第 n 波消除 n>=2 才显示)
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

-- js onGridChanged
function xxl_on_grid_changed(pos, enableAni)
	local ele = xxl.wndBoard:FindChildByID(xxl_pos2id(pos),-1);
	if ele == nil then return end
	local stackApi = QiIStackView(ele);
	if stackApi == nil then return end
	stackApi:SelectPage(xxl.board[pos.y][pos.x], enableAni);
	stackApi:Release();
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

-- 死局洗牌:受约束重排 + 带翻页动画重放全盘
function xxl_redeal()
	xxl.click_id = -1;
	xxl_gen_board();
	for y = 0, xxl_row-1 do
		for x = 0, xxl_col-1 do
			xxl_on_grid_changed({x=x,y=y}, true);
		end
	end
	xxl_check_board();
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
function xxl_begin_move(aniframe, state, ele, rcFrom, rcTo)
	ele:SetVisible(false,false);
	local ani_widget = xxl_build_ani_widget(aniframe, state);
	if ani_widget == nil then
		ele:SetVisible(true,false);
		xxl_slog("begin_move: build ani widget failed");
		return nil;
	end
	local ctxId = xxl_new_ctx("move", { ele=ele, ani_widget=ani_widget });
	local ani = NewValueAnimator();
	ani:CopyFrom(xxl.ani_move:GetIValueAnimator());
	ani:SetRangeRect(rcFrom, rcTo);
	ani:SetOnUpdate("xxl_ani_update");
	ani:SetOnEnd("xxl_ani_end");
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
	c.ele:SetVisible(true,false);
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
		xxl_check_aniframe();
		return
	end
	xxl.ani_ctx[ctxId] = nil;
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
	-- 消除瞬间爆发特效:>=3 连全显示,len>=4 特效放大
	xxl_pop_fx(xxl_get_ele_rect({x=samex.x + math.floor((samex.len-1)/2), y=samex.y}), samex.len >= 4);
	if samex.y > 0 then
		local ctxId = xxl_new_ctx("drop", { samex=samex });
		local group = NewAnimatorGroup();
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
function xxl_on_drop_end(g)
	local samex = g.samex;
	xxl.coin = xxl.coin + 1;
	xxl_show_coin();
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
-- 坐标模型:SOUI4 全树共享宿主窗口坐标系(Swnd.cpp DispatchPaint 无逐级平移,
-- GetWindowRect2 一律相对宿主)——格子/按钮的 rect 直接可用,无需任何换算。
-- 定位用显式 Move(浮动模式,立即生效):pos 属性要等下一次 relayout 才生效,
-- 上一版靠 pos 定位导致特效时有时无/错位。
function xxl_pop_fx(rc, boost, szSkin, szAnim)
	if xxl.aniframe == nil then
		xxl_slog("pop_fx SKIP: aniframe nil");
		return
	end
	local skin = szSkin or "skin_xxl_fx";
	local key = szAnim or "anim:xxl_fx_pop";
	local aniCache = xxl.fx_ani[key];
	if aniCache == nil then
		aniCache = GetApp():LoadAnimation(key);
		xxl.fx_ani[key] = aniCache;
	end
	if aniCache == nil then
		xxl_slog("pop_fx SKIP: LoadAnimation failed: " .. key);
		return
	end
	xxl.fx_seq = xxl.fx_seq + 1;
	local fid = xxl_fx_base_id + xxl.fx_seq;
	local scale = 1.5 + (boost and 0.5 or 0);
	local w = math.floor(rc:Width()*scale); local h = math.floor(rc:Height()*scale);
	local px = rc.left + math.floor(rc:Width()/2);
	local py = rc.top + math.floor(rc:Height()/2);
	local xml = '<img id="' .. fid .. '" skin="' .. skin .. '"'
		.. ' msgTransparent="1" visible="0" on_animation_stop="xxw_fx_stop"/>';
	xxl.aniframe:CreateChildrenFromXml(xml);
	local fx = xxl.aniframe:FindChildByID(fid,-1);
	if fx == nil then
		xxl_slog("pop_fx FAIL: fx window not created, id=" .. fid);
		return
	end
	fx:Move(CRect(px - math.floor(w/2), py - math.floor(h/2), px - math.floor(w/2) + w, py - math.floor(h/2) + h));
	-- cnchess 同款:clone 缓存动画再挂上,SetAnimation 后下一帧自动启动
	local ani = aniCache:clone();
	fx:SetAnimation(ani);
	ani:Release();
	-- 显示必须在 SetAnimation 之后:替换运行中动画可能触发旧动画 stop 事件
	fx:SetVisible(true,true);
	xxl_slog("pop_fx: #" .. xxl.fx_seq .. " " .. skin .. " @" .. (px - math.floor(w/2)) .. "," .. (py - math.floor(h/2)) .. " " .. w .. "x" .. h);
end

-- js onGetSameX: 横向 3 连聚拢到中心
function xxl_on_get_same_x(y,x,len)
	xxl.aniframe:SetVisible(true,true);
	xxl.combo = xxl.combo + 1;
	xxl_show_combo();
	local ctxId = xxl_new_ctx("clear", { samex={y=y,x=x,len=len}, posLst={} });
	local group = NewAnimatorGroup();
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
	local group = NewAnimatorGroup();
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
	-- 消除瞬间爆发特效(横向/纵向同款,len>=4 放大)
	xxl_pop_fx(xxl_get_ele_rect({x=samey.x, y=samey.y + math.floor((samey.len-1)/2)}), samey.len >= 4);
	if samey.y > 0 then
		local ctxId = xxl_new_ctx("drop_y", { samey=samey });
		local group = NewAnimatorGroup();
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
function xxl_on_drop_end_y(g)
	local samey = g.samey;
	xxl.coin = xxl.coin + 1;
	xxl_show_coin();
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
			local group = NewAnimatorGroup();
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
				ele1:SetVisible(true,false);
				ele2:SetVisible(true,false);
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
	-- 先丢弃所有动画上下文:格子即将销毁,存活动画回调(ctx=nil)自动变 no-op
	xxl.ani_ctx = {};
	xxl.ani_list = {};
	xxl.ani_count = 0;
	xxl.wndBoard:DestroyAllChildren();
	xxl.wndBoard:SetAttribute(T"columnCount", T"7", false);
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

-- js 构造器 + init
function xxl_init(root)
	local wndBoard = root:FindChildByNameA("wnd_xxl_board",-1);
	local aniframe = root:FindChildByNameA("wnd_xxl_aniframe",-1);
	if wndBoard == nil or aniframe == nil then
		return 0; -- demo 不含消消乐子页,跳过
	end
	xxl.root = root;
	xxl.wndBoard = wndBoard;
	xxl.aniframe = aniframe;
	xxl.ani_move = NewValueAnimator();
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
-- 稳定后再做市场玩法收尾:连击清零 / 游戏结束判定 / 死局自动洗牌。
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
	if xxl.coin <= 0 and not xxl.gameover then
		xxl.gameover = true;
		xxl_slog("settle: game over, score=" .. xxl.score);
		local hwnd = xxl.wndBoard:GetHostHwnd();
		SMessageBox(hwnd, L("本局结束!最终得分 " .. xxl.score .. "\n点击确定重新开始"), L("消消乐"), 0);
		xxl_restart_internal();
		return
	end
	-- 稳定收尾 3:无步可走 => 自动洗牌
	if not xxl_has_valid_move() then
		xxl_slog("settle: dead board, auto redeal");
		xxl_redeal();
	end
end

