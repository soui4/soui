/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef _SRW_LOCK_H_
#define _SRW_LOCK_H_

#include <utilities-def.h>
#include <helper/SNoCopyable.hpp>

SNSBEGIN

struct IRwLock;
class SRwLock : public SNoCopyable {
 public:
	SRwLock();
	~SRwLock();

  void LockExclusive();
  void UnlockExclusive();

  void LockShared();
  void UnlockShared();

 private:
	 IRwLock* impl;
};

SNSEND

#endif  // _SRW_LOCK_H_
