/**
 * @file SAnimatorSet-example.h
 * @brief Example usage of SAnimatorSet
 * @details This file demonstrates how to use SAnimatorSet to coordinate multiple animators
 */

#ifndef __SANIMATORSET_EXAMPLE__H__
#define __SANIMATORSET_EXAMPLE__H__

#include <valueAnimator/SAnimatorSet.h>

SNSBEGIN

/**
 * @brief Example of using SAnimatorSet with parallel execution
 *
 * @param pContainer The timeline handlers container
 * @param pWindow The window to animate
 * 
 * This example shows how to run multiple animators in parallel.
 * All animators start at the same time and run together.
 */
inline void ExampleParallelAnimators(ITimelineHandlersMgr *pContainer, ISWindow *pWindow)
{
    // Create position animator (0 to 100)
    SAutoRefPtr<SIntAnimator> posAnimator = new SIntAnimator();
    posAnimator->setValueFrom(0);
    posAnimator->setValueTo(100);
    posAnimator->setDuration(1000);
    posAnimator->addUpdateListener(new YourAnimatorListener());  // Your listener implementation

    // Create size animator (100 to 200)
    SAutoRefPtr<SIntAnimator> sizeAnimator = new SIntAnimator();
    sizeAnimator->setValueFrom(100);
    sizeAnimator->setValueTo(200);
    sizeAnimator->setDuration(1000);
    sizeAnimator->addUpdateListener(new YourAnimatorListener());

    // Create color animator (0 to COLORREF)
    SAutoRefPtr<SColorAnimator> colorAnimator = new SColorAnimator();
    colorAnimator->setValueFrom(RGB(255, 0, 0));
    colorAnimator->setValueTo(RGB(0, 0, 255));
    colorAnimator->setDuration(1000);
    colorAnimator->addUpdateListener(new YourAnimatorListener());

    // Create animator set
    SAutoRefPtr<SAnimatorSet> animatorSet = new SAnimatorSet();
    animatorSet->setPlayMode(PARALLEL);  // Set to parallel mode
    animatorSet->addAnimator(posAnimator);
    animatorSet->addAnimator(sizeAnimator);
    animatorSet->addAnimator(colorAnimator);

    // Start the set
    animatorSet->start(pContainer);
}

/**
 * @brief Example of using SAnimatorSet with sequential execution
 *
 * @param pContainer The timeline handlers container
 * @param pWindow The window to animate
 * 
 * This example shows how to run multiple animators sequentially.
 * Each animator waits for the previous one to finish.
 */
inline void ExampleSequentialAnimators(ITimelineHandlersMgr *pContainer, ISWindow *pWindow)
{
    // Create first position animator (0 to 100)
    SAutoRefPtr<SIntAnimator> posAnimator1 = new SIntAnimator();
    posAnimator1->setValueFrom(0);
    posAnimator1->setValueTo(100);
    posAnimator1->setDuration(500);
    posAnimator1->addUpdateListener(new YourAnimatorListener());

    // Create second position animator (100 to 200)
    SAutoRefPtr<SIntAnimator> posAnimator2 = new SIntAnimator();
    posAnimator2->setValueFrom(100);
    posAnimator2->setValueTo(200);
    posAnimator2->setDuration(500);
    posAnimator2->addUpdateListener(new YourAnimatorListener());

    // Create color animator
    SAutoRefPtr<SColorAnimator> colorAnimator = new SColorAnimator();
    colorAnimator->setValueFrom(RGB(255, 0, 0));
    colorAnimator->setValueTo(RGB(0, 0, 255));
    colorAnimator->setDuration(500);
    colorAnimator->addUpdateListener(new YourAnimatorListener());

    // Create animator set
    SAutoRefPtr<SAnimatorSet> animatorSet = new SAnimatorSet();
    animatorSet->setPlayMode(SEQUENCE);  // Set to sequence mode
    
    // Add animators in order
    animatorSet->addAnimator(posAnimator1);
    animatorSet->addAnimator(posAnimator2);
    animatorSet->addAnimator(colorAnimator);

    // Start the set
    animatorSet->start(pContainer);
}

/**
 * @brief Example of using SAnimatorSet with mixed execution
 *
 * @param pContainer The timeline handlers container
 * @param pWindow The window to animate
 * 
 * This example shows how to use addAnimatorAfter and addAnimatorWith
 * to create complex animation sequences.
 */
inline void ExampleMixedAnimators(ITimelineHandlersMgr *pContainer, ISWindow *pWindow)
{
    // Create animators
    SAutoRefPtr<SIntAnimator> animator1 = new SIntAnimator();
    animator1->setValueFrom(0);
    animator1->setValueTo(100);
    animator1->setDuration(500);

    SAutoRefPtr<SIntAnimator> animator2 = new SIntAnimator();
    animator2->setValueFrom(100);
    animator2->setValueTo(200);
    animator2->setDuration(500);

    SAutoRefPtr<SIntAnimator> animator3 = new SIntAnimator();
    animator3->setValueFrom(200);
    animator3->setValueTo(300);
    animator3->setDuration(500);

    SAutoRefPtr<SColorAnimator> colorAnimator = new SColorAnimator();
    colorAnimator->setValueFrom(RGB(255, 0, 0));
    colorAnimator->setValueTo(RGB(0, 0, 255));
    colorAnimator->setDuration(500);

    // Create animator set
    SAutoRefPtr<SAnimatorSet> animatorSet = new SAnimatorSet();
    
    // animator1 runs first
    animatorSet->addAnimator(animator1);
    
    // animator2 and colorAnimator run after animator1 finishes
    animatorSet->addAnimatorAfter(animator2, animator1);
    animatorSet->addAnimatorAfter(colorAnimator, animator1);
    
    // animator3 runs in parallel with animator2
    animatorSet->addAnimatorWith(animator3, animator2);

    // Start the set
    animatorSet->start(pContainer);
}

/**
 * @brief Example AnimatorSet listener implementation
 */
class ExampleAnimatorSetListener : public TObjRefImpl<IAnimatorSetListener> {
public:
    STDMETHOD_(void, onAnimatorSetStart)(THIS_ IAnimatorSet * pAnimatorSet) OVERRIDE
    {
        // Called when animator set starts
    }

    STDMETHOD_(void, onAnimatorSetEnd)(THIS_ IAnimatorSet * pAnimatorSet) OVERRIDE
    {
        // Called when animator set ends
        pAnimatorSet->Release();
    }

    STDMETHOD_(void, onAnimatorSetRepeat)(THIS_ IAnimatorSet * pAnimatorSet) OVERRIDE
    {
        // Called when animator set repeats
    }
};

SNSEND

#endif // __SANIMATORSET_EXAMPLE__H__
