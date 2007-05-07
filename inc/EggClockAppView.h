/*
============================================================================
 Name        : EggClockView.h
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application container
============================================================================
*/

#ifndef __EGGCLOCKAPPVIEW_H__
#define __EGGCLOCKAPPVIEW_H__

// INCLUDES
#include <coecntrl.h>
#include <gulicon.h>               // Icons
#include <MdaAudioSamplePlayer.h>  // MMdaAudioPlayerCallback
#include <drmaudiosampleplayer.h>  // MDrmAudioPlayerCallback

#define DRM_PLAYER

enum eAudioState
{
  eAudioIdle,
  eAudioIniting,
  eAudioReady,
  eAudioPlaying
};

const TInt INFINITE_MINUTES = 0xFF;

// Forward declaration
class MAknsControlContext;

// CLASS DECLARATION
class CEggClockAppView : public CCoeControl, public MMdaAudioPlayerCallback, public MDrmAudioPlayerCallback
{
  public: // Constructors
    static CEggClockAppView* NewL( const TRect& aRect );
    static CEggClockAppView* NewLC( const TRect& aRect );
    virtual ~CEggClockAppView();

  public: // Functions from CCoeControl
    void Draw( const TRect& aRect ) const;
    virtual void SizeChanged();
    virtual TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

  public: // Functions from MMdaAudioPlayerCallback
    virtual void MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
    virtual void MapcPlayComplete(TInt aError);

  public: // Functions from MDrmAudioPlayerCallback
    virtual void MdapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds &aDuration);
    virtual void MdapcPlayComplete(TInt aError);

  public: // New methods
    TInt  GetDuration();
    void  SetDuration(TInt n_iDuration);
    TBool IsRunning();
    TBool IsReset();
    void  SetNotificationL(const TDesC& aFileName);
    void  SetRepeatMinutesL(const TInt iMinutes);
    TInt  GetRepeatMinutes();
    void  ChangeVolume(TInt iStep);
    TBool StartTimer();
    TBool StopTimer();
    TBool ResetTimer();

  private: // Constructors
    void ConstructL(const TRect& aRect);
    CEggClockAppView();
  
  private: // Timer methods
    static TInt TimerFired(TAny* aPtr);
    void DoTimerFired();
    static TInt FlashTimerFired(TAny* aPtr);
    void DoFlashTimerFired();

  private: // New private methods
    void DrawLargeNumberL(const TRect& aRect, TInt iNumber) const;
    void DrawSmallNumberL(const TRect& aRect, TInt iNumber) const;
    void DrawVolumeL(const TRect& aRect) const;
    void PlayNotificationL();
    void LoadSettingsL();
    void SaveSettingsL();

  private:  // Data
    CPeriodic*                  m_pTimer;                   // Main timer
    TInt                        m_iTotalDuration;           // Total duration of timer in seconds
    TInt                        m_iRemainingDuration;       // Seconds remaining to 0

    CPeriodic*                  m_pFlashTimer;              // Timer to flash screen after notification
    TBool                       m_bFlash;                   // Flag to specify if screen is flashed

    CArrayPtr<CGulIcon>*        m_pDigitIcons;              // Array of icons with digits
    CGulIcon*                   m_pColonIcon;               // Icon with colon
    CGulIcon*                   m_pVolumeIcon;              // Icon for volume
    CGulIcon*                   m_pVolumeMuteIcon;          // Icon for volume muted
    CArrayPtr<CGulIcon>*        m_pVolumeValueIcons;        // Array of icons with volume
    CGulIcon*                   m_pRepeatOnceIcon;          // Icon for repeat once
    CGulIcon*                   m_pRepeatLoopIcon;          // Icon for repeat loop
    CGulIcon*                   m_pRepeatEveryXIcon;        // Icon for repeat every X minutes
    
    TFileName                   m_iNotificationFile;        // Name of notification file
#ifndef DRM_PLAYER
    CMdaAudioPlayerUtility*     m_pFilePlayer;              // Audio player
#else
    CDrmPlayerUtility*          m_pFilePlayer;              // Audio player
#endif
    eAudioState                 m_iAudioState;              // Audio state
    TInt                        m_iVolume;                  // Audio volume
    TInt                        m_iRepeatMinutes;
    
    MAknsControlContext*        m_pSkinBackground;
};

#endif // __EGGCLOCKAPPVIEW_H__

// End of File