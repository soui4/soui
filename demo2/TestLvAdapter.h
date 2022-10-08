#pragma once
#include <Windows.h>
#include <interface/SAdapter-i.h>

typedef struct TestLvAdapter
{
	ILvAdapter adapter;
	long nRefs;
	ILvDataSetObserver *obz;
	int nItems;
}TestLvAdapter;

BOOL TestLvAdapter_Init(ILvAdapter **ppAdapter);

void TestLvAdapter_notifyChanged(ILvAdapter *ppAdapter);

void TestLvAdapter_notifyInvalidated(ILvAdapter *ppAdapter);

void TestLvAdapter_notifyItemChanged(ILvAdapter *ppAdapter,int iItem);

void TestLvAdapter_Add(ILvAdapter *pAdapter);

void TestLvAdapter_Clean(ILvAdapter *pAdapter);