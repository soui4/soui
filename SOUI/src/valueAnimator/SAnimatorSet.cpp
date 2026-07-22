#include <souistd.h>
#include <valueAnimator/SAnimatorSet.h>
#include <helper/STime.h>

#define INVALID_SIZE -1

SNSBEGIN

SAnimatorSet::SAnimatorSet()
    : mPlayMode(PARALLEL)
    , mCurrentPlayTime(0)
    , mSetStartTime(-1)
    , mAnimatedFraction(0.0f)
{
    _this_for_callback = (IAnimatorSet *)this;
}

SAnimatorSet::~SAnimatorSet()
{
    cleanup();
}

void SAnimatorSet::cleanup()
{
    // Clean up all nodes
    SPOSITION pos = mAnimatorToNodeMap.GetStartPosition();
    while (pos)
    {
        IValueAnimator *pAnimator = NULL;
        AnimatorNode *pNode = NULL;
        mAnimatorToNodeMap.GetNextAssoc(pos, pAnimator, pNode);
        delete pNode;
    }
    mAnimatorToNodeMap.RemoveAll();
    mAnimatorNodes.RemoveAll();
    mRunningAnimators.RemoveAll();
}

SAnimatorSet::AnimatorNode *SAnimatorSet::findOrCreateNode(IValueAnimator *pAnimator)
{
    AnimatorNode *pNode = NULL;
    if (mAnimatorToNodeMap.Lookup(pAnimator, pNode))
    {
        return pNode;
    }

    pNode = new AnimatorNode(pAnimator);
    mAnimatorToNodeMap[pAnimator] = pNode;
    mAnimatorNodes.Add(pNode);
    return pNode;
}

SAnimatorSet::AnimatorNode *SAnimatorSet::findNode(IValueAnimator *pAnimator) const
{
    AnimatorNode *pNode = NULL;
    if (mAnimatorToNodeMap.Lookup(pAnimator, pNode))
    {
        return pNode;
    }
    return NULL;
}

void SAnimatorSet::AddAnimator(IValueAnimator *pAnimator)
{
    if (!pAnimator)
        return;

    AnimatorNode *pNode = findOrCreateNode(pAnimator);

    // In PARALLEL mode, all animators run together by default
    // In SEQUENCE mode, they will be arranged sequentially
}

void SAnimatorSet::AddAnimatorAfter(IValueAnimator *pAnimator, IValueAnimator *pAfterAnimator)
{
    if (!pAnimator || !pAfterAnimator)
        return;

    AnimatorNode *pNode = findOrCreateNode(pAnimator);
    AnimatorNode *pAfterNode = findNode(pAfterAnimator);

    if (!pAfterNode)
    {
        // If the after animator doesn't exist, just add this animator
        AddAnimator(pAnimator);
        return;
    }

    // Add dependency: pAnimator should start after pAfterAnimator ends
    pAfterNode->afterNodes.Add(pNode);
    pNode->beforeNodes.Add(pAfterNode);
}

void SAnimatorSet::AddAnimatorWith(IValueAnimator *pAnimator, IValueAnimator *pWithAnimator)
{
    if (!pAnimator || !pWithAnimator)
        return;

    AnimatorNode *pNode = findOrCreateNode(pAnimator);
    AnimatorNode *pWithNode = findNode(pWithAnimator);

    if (!pWithNode)
    {
        // If the with animator doesn't exist, just add this animator
        AddAnimator(pAnimator);
        return;
    }

    // Add dependency: pAnimator should run in parallel with pWithAnimator
    pWithNode->withNodes.Add(pNode);
    pNode->beforeNodes.Add(pWithNode);
}

void SAnimatorSet::RemoveAnimator(IValueAnimator *pAnimator)
{
    if (!pAnimator)
        return;

    AnimatorNode *pNode = findNode(pAnimator);
    if (!pNode)
        return;

    // Remove this node from all dependencies
    for (int i = 0; i < mAnimatorNodes.GetCount(); i++)
    {
        AnimatorNode *other = mAnimatorNodes[i];
        other->afterNodes.RemoveElement(pNode);
        other->withNodes.RemoveElement(pNode);
        other->beforeNodes.RemoveElement(pNode);
    }

    mAnimatorToNodeMap.RemoveKey(pAnimator);
    mAnimatorNodes.RemoveElement(pNode);
    mRunningAnimators.RemoveElement(pNode);
    delete pNode;
}

void SAnimatorSet::RemoveAllAnimators()
{
    cleanup();
}

int SAnimatorSet::GetAnimatorCount() const
{
    return (int)mAnimatorNodes.GetCount();
}

IValueAnimator *SAnimatorSet::GetAnimatorAt(int index) const
{
    if (index < 0 || index >= (int)mAnimatorNodes.GetCount())
        return NULL;
    return mAnimatorNodes[index]->animator;
}

void SAnimatorSet::setDuration(long duration)
{
    if (duration < 0)
        duration = 0;
    mDuration = duration;
}

long SAnimatorSet::getDuration() const
{
    return mDuration;
}

long SAnimatorSet::getTotalDuration() const
{
    // Calculate total duration based on animation dependencies and play mode
    if (mAnimatorNodes.GetCount() == 0)
        return 0;

    long totalDuration = mStartDelay;

    if (mPlayMode == PARALLEL)
    {
        // In parallel mode, total duration is the maximum duration of all animators
        long maxDuration = 0;
        for (int i = 0; i < (int)mAnimatorNodes.GetCount(); i++)
        {
            AnimatorNode *pNode = mAnimatorNodes[i];
            long nodeDuration = pNode->animator->getDuration();
            if (nodeDuration > maxDuration)
                maxDuration = nodeDuration;
        }
        totalDuration += maxDuration;
    }
    else
    {
        // In sequence mode, total duration is the sum of all animators
        for (int i = 0; i < (int)mAnimatorNodes.GetCount(); i++)
        {
            AnimatorNode *pNode = mAnimatorNodes[i];
            totalDuration += pNode->animator->getDuration();
        }
    }

    return totalDuration;
}

void SAnimatorSet::setStartDelay(long startDelay)
{
    if (startDelay < 0)
        startDelay = 0;
    mStartDelay = startDelay;
}

long SAnimatorSet::getStartDelay() const
{
    return mStartDelay;
}

void SAnimatorSet::calculateStartTimes()
{
    // Calculate start times for each animator based on dependencies
    long currentTime = 0;

    if (mPlayMode == PARALLEL)
    {
        // All animators start at the same time
        for (int i = 0; i < (int)mAnimatorNodes.GetCount(); i++)
        {
            mAnimatorNodes[i]->startTime = currentTime;
        }
    }
    else
    {
        // SEQUENCE mode: calculate start times based on dependencies
        SArray<AnimatorNode *> processedNodes;
        SArray<AnimatorNode *> toProcess;

        // Find root nodes (nodes with no before dependencies)
        for (int i = 0; i < (int)mAnimatorNodes.GetCount(); i++)
        {
            AnimatorNode *pNode = mAnimatorNodes[i];
            if (pNode->beforeNodes.GetCount() == 0)
            {
                toProcess.Add(pNode);
            }
        }

        while (toProcess.GetCount() > 0)
        {
            AnimatorNode *pNode = toProcess[0];
            toProcess.RemoveAt(0);

            if (processedNodes.Find(pNode) != INVALID_SIZE)
                continue;

            // Set start time
            long maxBeforeTime = 0;
            for (int i = 0; i < (int)pNode->beforeNodes.GetCount(); i++)
            {
                AnimatorNode *before = pNode->beforeNodes[i];
                long endTime = before->startTime + before->animator->getDuration();
                if (endTime > maxBeforeTime)
                    maxBeforeTime = endTime;
            }
            pNode->startTime = maxBeforeTime;

            processedNodes.Add(pNode);

            // Add after nodes to process
            for (int i = 0; i < (int)pNode->afterNodes.GetCount(); i++)
            {
                AnimatorNode *after = pNode->afterNodes[i];
                if (toProcess.Find(after) == INVALID_SIZE)
                {
                    toProcess.Add(after);
                }
            }
        }
    }
}

void SAnimatorSet::start(ITimelineHandlersMgr *pContainer)
{
    if (mRunning || mStarted)
        return;

    mContainer = pContainer;
    mStarted = true;
    mRunning = true;
    mSetStartTime = -1; // Will be initialized on first frame
    mCurrentPlayTime = 0;
    mStartListenersCalled = false;

    // Calculate start times for all animators based on dependencies
    calculateStartTimes();

    if (mContainer)
    {
        // Register with timeline handler
        mContainer->RegisterTimelineHandler(GetTimelineHandler());
    }
}

void SAnimatorSet::end()
{
    if (!mRunning && !mStarted)
        return;

    // End all running animators and unregister them from timeline handler
    for (int i = 0; i < (int)mRunningAnimators.GetCount(); i++)
    {
        AnimatorNode *pNode = mRunningAnimators[i];
        pNode->animator->end();
    }
    mRunningAnimators.RemoveAll();

    mRunning = false;
    mStarted = false;

    if (mContainer)
    {
        AddRef();
        mContainer->UnregisterTimelineHandler(GetTimelineHandler());
        mContainer = NULL;
        Release();
    }

    notifyEndListeners();
}

BOOL SAnimatorSet::isRunning() const
{
    return mRunning ? TRUE : FALSE;
}

BOOL SAnimatorSet::isStarted() const
{
    return mStarted ? TRUE : FALSE;
}

void SAnimatorSet::reverse()
{
    mReversing = !mReversing;

    for (int i = 0; i < (int)mRunningAnimators.GetCount(); i++)
    {
        AnimatorNode *pNode = mRunningAnimators[i];
        pNode->animator->reverse();
    }
}

void SAnimatorSet::SetPlayMode(AnimatorSetPlayMode mode)
{
    mPlayMode = mode;
}

AnimatorSetPlayMode SAnimatorSet::GetPlayMode() const
{
    return mPlayMode;
}
void SAnimatorSet::notifyStartListeners()
{
    if (mStartListenersCalled)
        return;

    mStartListenersCalled = true;
    // Notify IAnimatorListener listeners (inherited from SValueAnimator)
    // We get the listener list from SValueAnimator's mListeners and notify them
    SValueAnimator *pValueAnimator = static_cast<SValueAnimator *>(this);
    for (int i = 0; i < (int)mListeners.GetCount(); i++)
    {
        mListeners[i]->onAnimationStart(static_cast<IValueAnimator *>(pValueAnimator));
    }
}

void SAnimatorSet::notifyEndListeners()
{
    // Notify IAnimatorListener listeners (inherited from SValueAnimator)
    SValueAnimator *pValueAnimator = static_cast<SValueAnimator *>(this);
    for (int i = 0; i < (int)mListeners.GetCount(); i++)
    {
        mListeners[i]->onAnimationEnd(static_cast<IValueAnimator *>(pValueAnimator));
    }
}

void SAnimatorSet::notifyRepeatListeners()
{
    // Notify IAnimatorListener listeners (inherited from SValueAnimator)
    SValueAnimator *pValueAnimator = static_cast<SValueAnimator *>(this);
    for (int i = 0; i < (int)mListeners.GetCount(); i++)
    {
        mListeners[i]->onAnimationRepeat(static_cast<IValueAnimator *>(pValueAnimator));
    }
}

void SAnimatorSet::startAnimatorsAtTime(uint64_t currentTime)
{
    for (int i = 0; i < (int)mAnimatorNodes.GetCount(); i++)
    {
        AnimatorNode *pNode = mAnimatorNodes[i];

        if (pNode->started)
            continue;

        // Check if it's time to start this animator
        if (currentTime >= pNode->startTime)
        {
            pNode->animator->start(NULL);
            pNode->started = true;
            mRunningAnimators.Add(pNode);
        }
    }
}

void SAnimatorSet::updateRunningAnimators(uint64_t frameTime)
{
    if (!mRunning)
        return;

    // Update all running animators
    for (int i = (int)mRunningAnimators.GetCount() - 1; i >= 0; i--)
    {
        AnimatorNode *pNode = mRunningAnimators[i];

        // Commit animation frame to drive the animator forward
        // This is the key call that actually updates the animation state
        pNode->animator->commitAnimationFrame(frameTime);

        // Remove animator if it's no longer running and unregister from timeline handler
        if (!pNode->animator->isRunning())
        {
            mRunningAnimators.RemoveAt(i);
        }
    }

    // Check if all animators are finished
    if (allAnimatorsFinished())
    {
        end();
    }
}

bool SAnimatorSet::allAnimatorsFinished() const
{
    // Check if all animators that have been started are finished
    for (int i = 0; i < (int)mAnimatorNodes.GetCount(); i++)
    {
        AnimatorNode *pNode = mAnimatorNodes[i];
        if (pNode->started && pNode->animator->isRunning())
        {
            return false;
        }
    }
    return true;
}

BOOL SAnimatorSet::commitAnimationFrame(uint64_t currentTime)
{
    if (!mRunning && !mStarted)
        return FALSE;

    // Initialize start time on first frame
    if (mSetStartTime == -1)
    {
        mSetStartTime = currentTime;
        notifyStartListeners();
    }

    // Calculate elapsed time since animation set started
    long elapsedTime = (long)(currentTime - mSetStartTime);

    // Account for start delay - don't process animations until delay is over
    if (elapsedTime < mStartDelay)
        return FALSE;

    // Calculate animation time (after accounting for start delay)
    // This is the relative time from when animators should start (after delay)
    mCurrentPlayTime = elapsedTime - mStartDelay;

    // Start animators whose scheduled time has arrived
    // For PARALLEL mode: all animators have startTime = 0, so they start immediately
    // For SEQUENCE mode: animators are started in dependency order based on their calculated startTime
    startAnimatorsAtTime(mCurrentPlayTime);

    // Update all currently running animators by committing the frame
    // This drives the animation calculation and property updates
    updateRunningAnimators(currentTime);

    // Return TRUE if the animation is finished
    return mRunning ? FALSE : TRUE;
}

void SAnimatorSet::setCurrentPlayTime(long playTime)
{
    mCurrentPlayTime = playTime;
}

void SAnimatorSet::setCurrentFraction(float fraction)
{
    if (mDuration > 0)
    {
        long playTime = (long)(mDuration * fraction);
        setCurrentPlayTime(playTime);
    }
}

long SAnimatorSet::getCurrentPlayTime()
{
    return mCurrentPlayTime;
}

float SAnimatorSet::getAnimatedFraction() const
{
    if (mDuration == 0)
        return 0.0f;
    return (float)mCurrentPlayTime / (float)mDuration;
}

void SAnimatorSet::onEvaluateValue(float fraction)
{
    // AnimatorSet doesn't have property values to update like ValueAnimator
    // Just update the animated fraction
    mAnimatedFraction = fraction;
}

IValueAnimator *SAnimatorSet::clone() const
{
    SAnimatorSet *pRet = new SAnimatorSet();
    // 多继承下存在两条 IValueAnimator 虚基类路径，显式先转成 IAnimatorSet* 消除歧义
    pRet->copy(static_cast<const IAnimatorSet *>(this));
    return static_cast<IValueAnimator *>(static_cast<IAnimatorSet *>(pRet));
}

void SAnimatorSet::copy(const IValueAnimator *src)
{
    // 注意：SAnimatorSet 通过 TValueAnimatorProxy<IAnimatorSet> 有两条 IValueAnimator 子对象
    // （SValueAnimator 一条，IAnimatorSet 一条）。因此 sobj_cast 不能直接 cast 到 SAnimatorSet
    // （IObject* → SAnimatorSet* 有歧义）。正确做法：先 sobj_cast 到接口 IAnimatorSet*
    // （接口继承是唯一的：IValueAnimator → IAnimatorSet），再向下 static_cast 到 SAnimatorSet。
    const IAnimatorSet *pIASrc = sobj_cast<const IAnimatorSet>(src);
    if (!pIASrc)
        return;
    const SAnimatorSet *src2 = static_cast<const SAnimatorSet *>(pIASrc);

    // 1. 复制 SValueAnimator 基类的配置属性（duration/startDelay/repeat/interpolator 等）
    SValueAnimator::copy(src);

    // 2. 复制 AnimatorSet 自身属性
    mPlayMode = src2->mPlayMode;

    // 清空现有子节点（释放旧的 AnimatorNode 及其映射）
    cleanup();

    // 3. 为每个源 AnimatorNode 创建克隆的 Animator 和 新的 AnimatorNode，
    //    并建立 [源AnimatorNode指针 -> 目标AnimatorNode指针] 映射表以重建依赖关系。
    SMap<AnimatorNode *, AnimatorNode *> nodeMap;
    int count = (int)src2->mAnimatorNodes.GetCount();
    nodeMap.InitHashTable(smax(17, count * 2 + 1));
    for (int i = 0; i < count; i++)
    {
        AnimatorNode *srcNode = src2->mAnimatorNodes[i];
        SAutoRefPtr<IValueAnimator> clonedAni;
        clonedAni.Attach(srcNode->animator->clone());
        if (!clonedAni)
            continue;  // 子 animator 无法克隆时跳过，确保克隆流程不崩溃

        AnimatorNode *dstNode = new AnimatorNode(clonedAni);
        // 保留 startTime（因为 startTime 由 calculateStartTimes 基于依赖算出；但 started 这类运行态标志应保持默认 false）
        dstNode->startTime = srcNode->startTime;

        // clonedAni.Get() 返回 IValueAnimator* 是经过 IValueAnimator::clone() 标准偏移的指针，
        // 可直接用作 Map 键，不存在基类路径歧义。
        mAnimatorToNodeMap[clonedAni.Get()] = dstNode;
        mAnimatorNodes.Add(dstNode);
        nodeMap[srcNode] = dstNode;
    }

    // 4. 用映射表重建 afterNodes / withNodes / beforeNodes 依赖关系
    for (int i = 0; i < count; i++)
    {
        AnimatorNode *srcNode = src2->mAnimatorNodes[i];
        AnimatorNode *dstNode = NULL;
        if (!nodeMap.Lookup(srcNode, dstNode) || !dstNode)
            continue;

        // afterNodes
        for (int j = 0; j < (int)srcNode->afterNodes.GetCount(); j++)
        {
            AnimatorNode *srcAfter = srcNode->afterNodes[j];
            AnimatorNode *dstAfter = NULL;
            if (nodeMap.Lookup(srcAfter, dstAfter) && dstAfter)
                dstNode->afterNodes.Add(dstAfter);
        }
        // withNodes
        for (int j = 0; j < (int)srcNode->withNodes.GetCount(); j++)
        {
            AnimatorNode *srcWith = srcNode->withNodes[j];
            AnimatorNode *dstWith = NULL;
            if (nodeMap.Lookup(srcWith, dstWith) && dstWith)
                dstNode->withNodes.Add(dstWith);
        }
        // beforeNodes
        for (int j = 0; j < (int)srcNode->beforeNodes.GetCount(); j++)
        {
            AnimatorNode *srcBefore = srcNode->beforeNodes[j];
            AnimatorNode *dstBefore = NULL;
            if (nodeMap.Lookup(srcBefore, dstBefore) && dstBefore)
                dstNode->beforeNodes.Add(dstBefore);
        }
    }

    // mCurrentPlayTime/mSetStartTime/mAnimatedFraction/mRunningAnimators 都是运行期状态，
    // 保持默认（0/-1/空）即可，克隆只复制「配置属性+结构+依赖」，不复制运行状态。
}

SNSEND
