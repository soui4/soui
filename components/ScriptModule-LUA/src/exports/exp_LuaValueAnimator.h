#pragma once
// LuaValueAnimator / LuaAnimatorGroup: thin wrappers aligned with soui4js
// JsValueAnimator / JsAnimatorGroup, so a Lua port of a js mini-game can keep
// the same flow: LoadAnimator/CopyFrom -> SetRange -> Start -> callbacks.
//
// Callbacks are named global Lua functions invoked as
//   onUpdate(luaAni, ctxId) / onStart / onEnd
//   onGroupEnd(group, ctxId, nID)
// The ctxId is an arbitrary routing id set from Lua (SetCtx), so several live
// animators can share one callback and dispatch through a Lua table.
#include <valueAnimator/SValueAnimator.h>
#include "toobj.h"

SNSBEGIN

// ani types, keep the same values as soui4js exp_SValueAnimator.h
enum {
	kAni_None = 0,
	kAni_Int,
	kAni_Float,
	kAni_Color,
	kAni_Point,
	kAni_Size,
	kAni_Rect,
	kAni_Unknown = 1000
};

class LuaValueAnimator : protected IAnimatorListener, protected IAnimatorUpdateListener {
public:
	LuaValueAnimator(lua_State *L)
		: m_L(L)
		, m_aniType(kAni_None)
		, m_ctxId(0)
	{
	}

	~LuaValueAnimator()
	{
		Release();
	}

	void Release()
	{
		if (m_ani)
		{
			m_ani->removeListener(this);
			m_ani->removeUpdateListener(this);
			m_ani = NULL;
			m_aniType = kAni_None;
		}
	}

	void Init(IValueAnimator *ani)
	{
		Release();
		if (!ani)
			return;
		m_ani = ani;
		m_ani->addUpdateListener(this);
		m_ani->addListener(this);
		m_aniType = _GetAniType(m_ani);
	}

	bool LoadAnimator(const char *resId)
	{
		IValueAnimator *ani = SApplication::getSingleton().LoadValueAnimatorU8(resId);
		if (!ani)
			return false;
		Init(ani);
		ani->Release();
		return true;
	}

	// clone a template animator (typically loaded via LoadValueAnimator)
	bool CopyFrom(IValueAnimator *ani)
	{
		if (!ani)
			return false;
		IValueAnimator *ani2 = ani->clone();
		if (!ani2)
			return false;
		Init(ani2);
		ani2->Release();
		return true;
	}

	IValueAnimator *GetIValueAnimator()
	{
		return m_ani;
	}

	int GetAniType() const
	{
		return m_aniType;
	}

	float GetFraction() const
	{
		return m_ani ? m_ani->getAnimatedFraction() : 0.f;
	}

	CRect GetRectValue() const
	{
		CRect rc;
		SRectAnimator *ani = sobj_cast<SRectAnimator>((IObject *)m_ani);
		if (ani)
			rc = ani->getValue();
		return rc;
	}

	int GetIntValue() const
	{
		SIntAnimator *ani = sobj_cast<SIntAnimator>((IObject *)m_ani);
		return ani ? ani->getValue() : 0;
	}

	float GetFloatValue() const
	{
		SFloatAnimator *ani = sobj_cast<SFloatAnimator>((IObject *)m_ani);
		return ani ? ani->getValue() : 0.f;
	}

	bool SetRangeRect(const CRect &from, const CRect &to)
	{
		SRectAnimator *ani = sobj_cast<SRectAnimator>((IObject *)m_ani);
		if (!ani)
			return false;
		ani->setRange(from, to);
		return true;
	}

	bool Start(IWindow *pWnd)
	{
		if (!m_ani || !pWnd)
			return false;
		m_ani->start(pWnd->GetContainer());
		return true;
	}

	void SetCtx(int ctxId)
	{
		m_ctxId = ctxId;
	}

	int GetCtx() const
	{
		return m_ctxId;
	}

	void SetOnUpdate(const char *fn)
	{
		m_cbUpdate = fn;
	}

	void SetOnEnd(const char *fn)
	{
		m_cbEnd = fn;
	}

protected:
	static int _GetAniType(IValueAnimator *ani)
	{
		if (!ani)
			return kAni_None;
		if (ani->IsClass(SRectAnimator::GetClassName()))
			return kAni_Rect;
		if (ani->IsClass(SIntAnimator::GetClassName()))
			return kAni_Int;
		if (ani->IsClass(SFloatAnimator::GetClassName()))
			return kAni_Float;
		if (ani->IsClass(SColorAnimator::GetClassName()))
			return kAni_Color;
		if (ani->IsClass(SPointAnimator::GetClassName()))
			return kAni_Point;
		if (ani->IsClass(SSizeAnimator::GetClassName()))
			return kAni_Size;
		return kAni_Unknown;
	}

	// IAnimatorUpdateListener
	STDMETHOD_(void, onAnimationUpdate)(THIS_ IValueAnimator *pAnimator) OVERRIDE
	{
		if (!m_cbUpdate.IsEmpty())
			lua_tinker::call<void>(m_L, m_cbUpdate.c_str(), this, m_ctxId);
	}

	// IAnimatorListener
	STDMETHOD_(void, onAnimationStart)(THIS_ IValueAnimator *pAnimator) OVERRIDE
	{
	}

	STDMETHOD_(void, onAnimationEnd)(THIS_ IValueAnimator *pAnimator) OVERRIDE
	{
		if (!m_cbEnd.IsEmpty())
			lua_tinker::call<void>(m_L, m_cbEnd.c_str(), this, m_ctxId);
	}

	STDMETHOD_(void, onAnimationRepeat)(THIS_ IValueAnimator *pAnimator) OVERRIDE
	{
	}

	lua_State *m_L;
	SAutoRefPtr<IValueAnimator> m_ani;
	int m_aniType;
	int m_ctxId;
	SStringA m_cbUpdate;
	SStringA m_cbEnd;
};

// LuaAnimatorGroup fires onGroupEnd(group, ctxId, nID) when all child
// animators finish, mirroring js aniGroup.onAnimatorGroupEnd.
class LuaAnimatorGroup : public SAnimatorGroup, public IAnimatorGroupListerer {
public:
	LuaAnimatorGroup(lua_State *L, int nID = 0)
		: SAnimatorGroup(nID)
		, m_L(L)
		, m_ctxId(0)
	{
		SetListener(this);
	}

	~LuaAnimatorGroup()
	{
	}

	STDMETHOD_(void, OnAnimatorGroupEnd)(THIS_ IAnimatorGroup *pGroup, int nID) OVERRIDE
	{
		if (!m_cbEnd.IsEmpty())
			lua_tinker::call<void>(m_L, m_cbEnd.c_str(), this, m_ctxId, nID);
	}

	void SetCtx(int ctxId)
	{
		m_ctxId = ctxId;
	}

	int GetCtx() const
	{
		return m_ctxId;
	}

	void SetOnGroupEnd(const char *fn)
	{
		m_cbEnd = fn;
	}

protected:
	lua_State *m_L;
	int m_ctxId;
	SStringA m_cbEnd;
};

static LuaValueAnimator *Lua_NewValueAnimator()
{
	return new LuaValueAnimator(lua_tinker::get_state());
}

static LuaAnimatorGroup *Lua_NewAnimatorGroup(int nID = 0)
{
	return new LuaAnimatorGroup(lua_tinker::get_state(), nID);
}

static int ExpLua_LuaValueAnimator(lua_State *L)
{
	try
	{
		lua_tinker::class_add<LuaValueAnimator>(L, "LuaValueAnimator");
		lua_tinker::class_def<LuaValueAnimator>(L, "LoadAnimator", &LuaValueAnimator::LoadAnimator);
		lua_tinker::class_def<LuaValueAnimator>(L, "CopyFrom", &LuaValueAnimator::CopyFrom);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetIValueAnimator", &LuaValueAnimator::GetIValueAnimator);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetAniType", &LuaValueAnimator::GetAniType);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetFraction", &LuaValueAnimator::GetFraction);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetRectValue", &LuaValueAnimator::GetRectValue);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetIntValue", &LuaValueAnimator::GetIntValue);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetFloatValue", &LuaValueAnimator::GetFloatValue);
		lua_tinker::class_def<LuaValueAnimator>(L, "SetRangeRect", &LuaValueAnimator::SetRangeRect);
		lua_tinker::class_def<LuaValueAnimator>(L, "Start", &LuaValueAnimator::Start);
		lua_tinker::class_def<LuaValueAnimator>(L, "SetCtx", &LuaValueAnimator::SetCtx);
		lua_tinker::class_def<LuaValueAnimator>(L, "GetCtx", &LuaValueAnimator::GetCtx);
		lua_tinker::class_def<LuaValueAnimator>(L, "SetOnUpdate", &LuaValueAnimator::SetOnUpdate);
		lua_tinker::class_def<LuaValueAnimator>(L, "SetOnEnd", &LuaValueAnimator::SetOnEnd);
		lua_tinker::def(L, "NewValueAnimator", &Lua_NewValueAnimator);

		lua_tinker::class_add<LuaAnimatorGroup>(L, "LuaAnimatorGroup");
		lua_tinker::class_inh<LuaAnimatorGroup, IAnimatorGroup>(L);
		// IAnimatorSet methods inherited by IAnimatorGroup are already bound on
		// IAnimatorGroup in exp_IAnimatorSet.h; expose group-specific ones here.
		lua_tinker::class_def<LuaAnimatorGroup>(L, "SetCtx", &LuaAnimatorGroup::SetCtx);
		lua_tinker::class_def<LuaAnimatorGroup>(L, "GetCtx", &LuaAnimatorGroup::GetCtx);
		lua_tinker::class_def<LuaAnimatorGroup>(L, "SetOnGroupEnd", &LuaAnimatorGroup::SetOnGroupEnd);
		lua_tinker::def(L, "NewAnimatorGroup", &Lua_NewAnimatorGroup);
	}
	catch (...)
	{
		lua_tinker::print_error(L, "ExpLua_LuaValueAnimator init error");
		return 0;
	}
	return 0;
}

SNSEND
