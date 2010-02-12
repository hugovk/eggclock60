/*
============================================================================
 Name        : EggClockAppView.cpp
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application container
============================================================================
*/

// INCLUDE FILES
#include <akniconarray.h>                       // Icon array
#include <aknnotewrappers.h>                    // Error note
#include <AknsBasicBackgroundControlContext.h>
#include <AknsDrawUtils.h>                      // Skin support
#include <coemain.h>
#include <eikenv.h>
#include <gulicon.h>                            // Icons
#include <remconinterfaceselector.h>            // CRemConInterfaceSelector
#include <remconcoreapitarget.h>                // CRemConCoreApiTarget
#ifdef __S60_50__
#include <touchfeedback.h>                      // Touch feedback
#endif

#include "EggClockAppView.h"
#include <EggClock_numbers_icons.mbg>
#include <EggClock.rsg>

#define  ONE_SECOND            1000 * 1000
#define  FLASH_TIME            700 * 1000
#define  CLOCK_SIDE_PADDING    15
#define  DIGIT_PADDING         2
#define  MAX_VOLUME            4

_LIT(KSettingsFileName, "eggtimer.txt");

struct stSettings
{
  TFileName iNotificationFile;
  TInt      iTotalTime;
  TInt      iVolume;
  TInt      iRepeatMinutes;
  TBool     bUseSkin;
};
typedef TPckgBuf<stSettings> SETTINGS_BUF;


// ============================ MEMBER FUNCTIONS ===============================
CEggClockAppView* CEggClockAppView::NewL( const TRect& aRect )
{
  CEggClockAppView* self(CEggClockAppView::NewLC( aRect ));
  CleanupStack::Pop( self );
  return self;
}

CEggClockAppView* CEggClockAppView::NewLC( const TRect& aRect )
{
  CEggClockAppView* self(new (ELeave) CEggClockAppView);
  CleanupStack::PushL( self );
  self->ConstructL( aRect );
  return self;
}

void CEggClockAppView::ConstructL( const TRect& aRect )
{
#ifdef  __S60_50__
  m_bLongTap = EFalse;
#endif

  // Create the timers
  m_pTimer = CPeriodic::NewL(CActive::EPriorityIdle);
  m_pFlashTimer = CPeriodic::NewL(CActive::EPriorityIdle);

  // Load icons with numbers
  _LIT(KEggClockIconsFilename, "\\resource\\apps\\EggClock_numbers_icons.mbm");
  m_pDigitIcons = new (ELeave) CAknIconArray(10);
  for (TInt i(0); i < 10; ++i)
  {
    CFbsBitmap* pBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_icons0 + i));
    CleanupStack::PushL(pBitmap);  
    m_pDigitIcons->AppendL(CGulIcon::NewL(pBitmap, pBitmap));
    CleanupStack::Pop(); // pBitmap
  }
  
  // Load icon for colon
  CFbsBitmap* pColonBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsColon));
  CleanupStack::PushL(pColonBitmap);
  m_pColonIcon = CGulIcon::NewL(pColonBitmap, pColonBitmap);
  CleanupStack::Pop(); // pColonBitmap

  // Load icons for volume
  CFbsBitmap* pVolumeBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume));
  CleanupStack::PushL(pVolumeBitmap);
  CFbsBitmap* pVolumeBitmapMask(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_mask));
  CleanupStack::PushL(pVolumeBitmapMask);
  m_pVolumeIcon = CGulIcon::NewL(pVolumeBitmap, pVolumeBitmapMask);
  CleanupStack::Pop(); // pVolumeBitmapMask
  CleanupStack::Pop(); // pVolumeBitmap
  CFbsBitmap* pVolumeMuteBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_mute));
  CleanupStack::PushL(pVolumeMuteBitmap);
  CFbsBitmap* pVolumeMuteBitmapMask(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_mute_mask));
  CleanupStack::PushL(pVolumeMuteBitmapMask);
  m_pVolumeMuteIcon = CGulIcon::NewL(pVolumeMuteBitmap, pVolumeMuteBitmapMask);
  CleanupStack::Pop(); // pVolumeMuteBitmapMask
  CleanupStack::Pop(); // m_pVolumeMuteIcon

  m_pVolumeValueIcons = new (ELeave) CAknIconArray(MAX_VOLUME + 1);
  for (TInt i(0); i <= MAX_VOLUME; ++i)
  {
    CFbsBitmap* pBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_0 + i));
    CleanupStack::PushL(pBitmap);  
    m_pVolumeValueIcons->AppendL(CGulIcon::NewL(pBitmap, pBitmap));
    CleanupStack::Pop(); // pBitmap
  }
  
  // Load icons for repeat
  CFbsBitmap* pRepeatOnceBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsRepeat_once));
  CleanupStack::PushL(pRepeatOnceBitmap);
  m_pRepeatOnceIcon = CGulIcon::NewL(pRepeatOnceBitmap, pRepeatOnceBitmap);
  CleanupStack::Pop(); // pRepeatOnceBitmap
  CFbsBitmap* pRepeatLoopBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsRepeat_loop));
  CleanupStack::PushL(pRepeatLoopBitmap);
  m_pRepeatLoopIcon = CGulIcon::NewL(pRepeatLoopBitmap, pRepeatLoopBitmap);
  CleanupStack::Pop(); // pRepeatLoopBitmap
  CFbsBitmap* pRepeatEveryXBitmap(iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsRepeat_every_x));
  CleanupStack::PushL(pRepeatEveryXBitmap);
  m_pRepeatEveryXIcon = CGulIcon::NewL(pRepeatEveryXBitmap, pRepeatEveryXBitmap);
  CleanupStack::Pop(); // pRepeatEveryXBitmap
  
  // Load settings
  LoadSettingsL();
  
  // Initialize player if required
  if (m_iNotificationFile.Length() > 0)
  {
#ifndef DRM_PLAYER
    m_pFilePlayer = CMdaAudioPlayerUtility::NewFilePlayerL(m_iNotificationFile, *this, EMdaPriorityNormal, EMdaPriorityPreferenceTime);
#else
    m_pFilePlayer = CDrmPlayerUtility::NewFilePlayerL(m_iNotificationFile, *this, EMdaPriorityNormal, EMdaPriorityPreferenceTime);
#endif
    if (m_pFilePlayer)
    {
      m_iAudioState = eAudioIniting;
    }
  }
  
  // Object to detect volume keys
  m_pSelector = CRemConInterfaceSelector::NewL();
  m_pTarget = CRemConCoreApiTarget::NewL( *m_pSelector, *this ); 
  m_pSelector->OpenTargetL();
  
  // Create a window for this application view and set the size
  CreateWindowL();
  SetRect(aRect);

  // Skin support
  m_pSkinBackground = CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain, Rect(), EFalse);

#ifdef  __S60_50__
  // In case of touch screen, activate long touch detector
  if (AknLayoutUtils::PenEnabled())
  {
    m_pLongTapDetector = CAknLongTapDetector::NewL(this);
  }
  
  // Touch feedback
  m_pTouchFeedback = MTouchFeedback::Instance();
  if (m_pTouchFeedback)
  {
    m_pTouchFeedback->SetFeedbackEnabledForThisApp(ETrue);
  }
#endif

  // Activate the control
  ActivateL();
}

CEggClockAppView::CEggClockAppView()
{
  m_iTotalDuration = 0;
  m_pFilePlayer    = NULL;
  m_iAudioState    = eAudioIdle;
  m_iVolume        = MAX_VOLUME;
}

CEggClockAppView::~CEggClockAppView()
{
  if (m_pTimer)
  {
    if (m_pTimer->IsActive())
    {
      m_pTimer->Cancel();
    }
    delete m_pTimer;
    m_pTimer = NULL;
  }
  if (m_pFlashTimer)
  {
    if (m_pFlashTimer->IsActive())
    {
      m_pFlashTimer->Cancel();
    }
    delete m_pFlashTimer;
    m_pFlashTimer = NULL;
  }
  if (m_pDigitIcons)
  {
    m_pDigitIcons->ResetAndDestroy();
    delete m_pDigitIcons;
    m_pDigitIcons = NULL;
  }
  if (m_pColonIcon)
  {
    delete m_pColonIcon;
    m_pColonIcon = NULL;
  }
  if (m_pVolumeIcon)
  {
    delete m_pVolumeIcon;
    m_pVolumeIcon = NULL;
  }
  if (m_pVolumeMuteIcon)
  {
    delete m_pVolumeMuteIcon;
    m_pVolumeMuteIcon = NULL;
  }
  if (m_pVolumeValueIcons)
  {
    m_pVolumeValueIcons->ResetAndDestroy();
    delete m_pVolumeValueIcons;
    m_pVolumeValueIcons = NULL;
  }
  if (m_pRepeatOnceIcon)
  {
    delete m_pRepeatOnceIcon;
    m_pRepeatOnceIcon = NULL;
  }
  if (m_pRepeatLoopIcon)
  {
    delete m_pRepeatLoopIcon;
    m_pRepeatLoopIcon = NULL;
  }
  if (m_pRepeatEveryXIcon)
  {
    delete m_pRepeatEveryXIcon;
    m_pRepeatEveryXIcon = NULL;
  }
  if (m_pFilePlayer)
  {
    delete m_pFilePlayer;
    m_pFilePlayer = NULL;
  }
  if (m_pSkinBackground)
  {
    delete m_pSkinBackground;
    m_pSkinBackground = NULL;
  }
  if (m_pSelector)
  {
    delete m_pSelector;
    m_pSelector = NULL;
  }
  m_pTarget = NULL;
#ifdef  __S60_50__
  if (m_pLongTapDetector)
  {
    delete m_pLongTapDetector;
    m_pLongTapDetector = NULL;
  }
  m_pTouchFeedback = NULL;
#endif
}

TTypeUid::Ptr CEggClockAppView::MopSupplyObject(TTypeUid aId)
{
  if(aId.iUid == MAknsControlContext::ETypeId && m_pSkinBackground)
  {
    return MAknsControlContext::SupplyMopObject(aId, m_pSkinBackground);
  }
  return CCoeControl::MopSupplyObject(aId);
}

void CEggClockAppView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
  if(!AknLayoutUtils::PenEnabled())
  {
    return;
  }

  // Pass the pointer event to Long tap detector component
#ifdef  __S60_50__
  if (m_pLongTapDetector)
  {
    m_pLongTapDetector->PointerEventL(aPointerEvent);
  }
#endif

  // Call base class method
  CCoeControl::HandlePointerEventL(aPointerEvent);
  
  // Check if long tap occurred
#ifdef  __S60_50__
  if (m_bLongTap)
  {
    m_bLongTap = EFalse;
    return;
  }
#endif
  
  // On ButtonUp, start/stop the timer
  if (aPointerEvent.iType == TPointerEvent::EButton1Up)
  {
    if (IsRunning())
    {
      StopTimer();
    }
    else
    {
      StartTimer();
    }
  }
  
  // Touch feedback
#ifdef  __S60_50__
  if (aPointerEvent.iType == TPointerEvent::EButton1Down)
  {
    if (m_pTouchFeedback)
    {
      m_pTouchFeedback->InstantFeedback(ETouchFeedbackBasic);
    }
  }
#endif
}

void CEggClockAppView::HandleLongTapEventL(const TPoint& /*aPenEventLocation*/, const TPoint& /*aPenEventScreenLocation*/)
{
#ifdef  __S60_50__
  m_bLongTap = ETrue;
#endif
  
  ResetTimer();
}

TKeyResponse CEggClockAppView::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/)
{
  switch(aKeyEvent.iCode)
  {
    // Volume up
    case EKeyRightArrow:
    case EKeyIncVolume:
      ChangeVolume(1);
      return EKeyWasConsumed;
    // Volume down
    case EKeyLeftArrow:
    case EKeyDecVolume:
      ChangeVolume(-1);
      return EKeyWasConsumed;
    // Reset
    case EKeyBackspace:
      ResetTimer();
      return EKeyWasConsumed;
    // Start and stop
    case EKeyDevice3:
    case EKeyEnter:
      if (IsRunning())
      {
        StopTimer();
      }
      else
      {
        StartTimer();
      }
      return EKeyWasConsumed;
    // Red key to background if running
    case EKeyNo:
      if (IsRunning())
      {
        CEikonEnv::Static()->RootWin().SetOrdinalPosition(-1);
      }
      return EKeyWasConsumed;
    case '1':
      SetDuration(1 * 60);
      return EKeyWasConsumed;
    case '2':
      SetDuration(2 * 60);
      return EKeyWasConsumed;
    case '3':
      SetDuration(3 * 60);
      return EKeyWasConsumed;
    case '4':
      SetDuration(4 * 60);
      return EKeyWasConsumed;
    case '5':
      SetDuration(5 * 60);
      return EKeyWasConsumed;
    case '6':
      SetDuration(6 * 60);
      return EKeyWasConsumed;
    case '7':
      SetDuration(7 * 60);
      return EKeyWasConsumed;
    case '8':
      SetDuration(8 * 60);
      return EKeyWasConsumed;
    case '9':
      SetDuration(9 * 60);
      return EKeyWasConsumed;
    case '0':
      SetDuration(10 * 60);
      return EKeyWasConsumed;
    case EKeyDownArrow:
    case '*':
    {
      TInt duration(GetDuration() - 60);
      if (duration > 0)
      {
        SetDuration(duration);
      }
      return EKeyWasConsumed;
    }
    case EKeyUpArrow:
    case '#':
    {
      TInt duration(GetDuration() + 60);
      TInt maxDuration((100 * 60) - 1); // 99m:59s
      if (duration < maxDuration)
      {
        SetDuration(duration);
      }
      else
      {
        SetDuration(maxDuration);
      }
      return EKeyWasConsumed;
    }
    default:
      break;
  }
  return EKeyWasNotConsumed;
}

void CEggClockAppView::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction /*aButtonAct*/)
{
  switch(aOperationId)
  {
    case ERemConCoreApiVolumeDown: 
      ChangeVolume(-1);
      break;
    case ERemConCoreApiVolumeUp:
      ChangeVolume(1);
      break;
    default:
      // Nothing to do
      break;
  }
}

void CEggClockAppView::Draw(const TRect& aRect) const
{
  // Get the standard graphics context
  CWindowGc& gc(SystemGc());

  // Clears the screen
  if (m_pFlashTimer && m_pFlashTimer->IsActive() && m_bFlash)
  {
    gc.SetBrushColor(KRgbRed);
    gc.Clear(Rect());
  }
  else
  {
    if (m_bUseSkin)
    {
      // Skin support
      MAknsSkinInstance* skin(AknsUtils::SkinInstance());
      MAknsControlContext* cc(AknsDrawUtils::ControlContext(this));
      AknsDrawUtils::Background(skin, cc, this, gc, aRect);
    }
    else
    {
      // Use white background
      gc.SetBrushColor(KRgbWhite);
      gc.Clear(Rect());
    }
  }

  // Calculate numbers to show
  TInt iLargeNumber((m_iRemainingDuration > 0) ? m_iRemainingDuration : 0);
  TInt iSmallNumber((m_iRemainingDuration < 0) ? (0 - m_iRemainingDuration) : 0);

  // Large numbers
  TRAP_IGNORE(
    DrawLargeNumberL(aRect, iLargeNumber);
    DrawSmallNumberL(aRect, iSmallNumber);
    DrawVolumeL(aRect);
  );
}

void CEggClockAppView::DrawLargeNumberL(const TRect& /*aRect*/, TInt iNumber) const
{
  // Get container size
  TSize iScreenSize(Size());

  // Prepare array with bitmaps
  CFbsBitmap* ppDigitBitmap[5];
  ppDigitBitmap[0] = m_pDigitIcons->At((iNumber / 60) / 10)->Bitmap();
  ppDigitBitmap[1] = m_pDigitIcons->At((iNumber / 60) % 10)->Bitmap();
  ppDigitBitmap[2] = m_pColonIcon->Bitmap();
  ppDigitBitmap[3] = m_pDigitIcons->At((iNumber % 60) / 10)->Bitmap();
  ppDigitBitmap[4] = m_pDigitIcons->At((iNumber % 60) % 10)->Bitmap();
  
  // Calculate zoom factor
  TInt iTotalWidth(ppDigitBitmap[0]->SizeInPixels().iWidth + ppDigitBitmap[1]->SizeInPixels().iWidth +
                   ppDigitBitmap[2]->SizeInPixels().iWidth + ppDigitBitmap[3]->SizeInPixels().iWidth +
                   ppDigitBitmap[4]->SizeInPixels().iWidth + (2 * CLOCK_SIDE_PADDING) + (4 * DIGIT_PADDING));
  TInt iZoomFactor((100 * iScreenSize.iWidth) / iTotalWidth);

  // Calculate X and Y
  TInt iXPosition(CLOCK_SIDE_PADDING * iZoomFactor / 100);
  TInt iYPosition(0);
  for (TInt i(0); i < 5; ++i)
  {
    iYPosition = (ppDigitBitmap[i]->SizeInPixels().iHeight > iYPosition) ? ppDigitBitmap[i]->SizeInPixels().iHeight : iYPosition;
  }
  iYPosition = (iScreenSize.iHeight - (iYPosition * iZoomFactor / 100)) / 2;

  // Draw bitmaps
  CWindowGc& gc(SystemGc());
  for (TInt i(0); i < 5; ++i)
  {
    TPoint iPoint(iXPosition, iYPosition);
    TSize  iSize(ppDigitBitmap[i]->SizeInPixels().iWidth * iZoomFactor / 100,
                 ppDigitBitmap[i]->SizeInPixels().iHeight * iZoomFactor / 100);
    
    gc.DrawBitmapMasked(TRect(iPoint, iSize), ppDigitBitmap[i], TRect(ppDigitBitmap[i]->SizeInPixels()), ppDigitBitmap[i], ETrue);
    
    iXPosition += iSize.iWidth + DIGIT_PADDING;
  }
}

void CEggClockAppView::DrawSmallNumberL(const TRect& /*aRect*/, TInt iNumber) const
{  
  // Small number
  if (iNumber > 0)
  {
    CWindowGc& gc(SystemGc());

    TBuf<128> iOvertimeString;
    iCoeEnv->ReadResourceL(iOvertimeString, R_STRING_OVERTIME);

    iOvertimeString.Append(' ');
    iOvertimeString.AppendNum((iNumber / 60) / 10);
    iOvertimeString.AppendNum((iNumber / 60) % 10);
    iOvertimeString.Append(':');
    iOvertimeString.AppendNum((iNumber % 60) / 10);
    iOvertimeString.AppendNum((iNumber % 60) % 10);
    
    const CFont* iFont(iEikonEnv->DenseFont());
    gc.SetPenColor(KRgbBlack);
    gc.UseFont(iFont);
    
    gc.DrawText(iOvertimeString, TPoint(CLOCK_SIDE_PADDING, Size().iHeight - CLOCK_SIDE_PADDING));
    gc.DiscardFont();
  }
}

void CEggClockAppView::DrawVolumeL(const TRect& /*aRect*/) const
{
  if (m_iVolume >= 0 && m_iVolume <= MAX_VOLUME)
  {
    CWindowGc& gc(SystemGc());
  
    CFbsBitmap* pRepeat(m_iRepeatMinutes == 0 ? m_pRepeatLoopIcon->Bitmap() :
                       (m_iRepeatMinutes >= INFINITE_MINUTES ? m_pRepeatOnceIcon->Bitmap() : m_pRepeatEveryXIcon->Bitmap()));
    CFbsBitmap* pVolume((m_iNotificationFile.Length() > 0 && m_iAudioState != eAudioIdle) ? m_pVolumeIcon->Bitmap() : m_pVolumeMuteIcon->Bitmap());
    CFbsBitmap* pVolumeValue(m_pVolumeValueIcons->At(m_iVolume)->Bitmap());
    
    if (pRepeat && pVolume && pVolumeValue)
    {
      if (m_iNotificationFile.Length() > 0)
      {
        TInt iTotalWidth(pRepeat->SizeInPixels().iWidth + DIGIT_PADDING +
                         pVolume->SizeInPixels().iWidth + DIGIT_PADDING +
                         pVolumeValue->SizeInPixels().iWidth + CLOCK_SIDE_PADDING);

        TPoint iPoint(Size().iWidth - iTotalWidth, CLOCK_SIDE_PADDING);
        gc.DrawBitmapMasked(TRect(iPoint, pRepeat->SizeInPixels()), pRepeat, TRect(pRepeat->SizeInPixels()), pRepeat, ETrue);

        iPoint = iPoint + TPoint(pRepeat->SizeInPixels().iWidth + DIGIT_PADDING, 0);
        gc.DrawBitmapMasked(TRect(iPoint, pVolume->SizeInPixels()), pVolume, TRect(pVolume->SizeInPixels()), pVolume, ETrue);
      
        iPoint = iPoint + TPoint(pVolume->SizeInPixels().iWidth + DIGIT_PADDING, 0);
        gc.DrawBitmapMasked(TRect(iPoint, pVolumeValue->SizeInPixels()), pVolumeValue, TRect(pVolumeValue->SizeInPixels()), pVolumeValue, ETrue);
      }
      else
      {
        TInt iTotalWidth(pVolume->SizeInPixels().iWidth + CLOCK_SIDE_PADDING);
        TPoint iPoint(Size().iWidth - iTotalWidth, CLOCK_SIDE_PADDING);
        gc.DrawBitmapMasked(TRect(iPoint, pVolume->SizeInPixels()), pVolume, TRect(pVolume->SizeInPixels()), pVolume, ETrue);
      }
    }
  }
}

void CEggClockAppView::SizeChanged()
{
  DrawNow();
}

TBool CEggClockAppView::StartTimer()
{
  if (m_pFlashTimer && m_pFlashTimer->IsActive())
  {
    m_pFlashTimer->Cancel();
  }
  if (m_pTimer && !m_pTimer->IsActive())
  {
    m_pTimer->Start(ONE_SECOND, ONE_SECOND, TCallBack(CEggClockAppView::TimerFired, this));
    DrawNow();
    return ETrue;
  }
  return EFalse;
}

TBool CEggClockAppView::StopTimer()
{
  if (m_pFlashTimer && m_pFlashTimer->IsActive())
  {
    m_pFlashTimer->Cancel();
  }
  if (m_pTimer && m_pTimer->IsActive())
  {
    m_pTimer->Cancel();
  }
  if (m_pFilePlayer && m_iAudioState == eAudioPlaying)
  {
    m_pFilePlayer->Stop();
    m_iAudioState = eAudioReady;
  }
  DrawNow();
  return ETrue;
}

TBool CEggClockAppView::ResetTimer()
{
  if (m_pFlashTimer && m_pFlashTimer->IsActive())
  {
    m_pFlashTimer->Cancel();
  }
  if (m_pTimer && m_pTimer->IsActive())
  {
    m_pTimer->Cancel();
  }
  if (m_pFilePlayer && m_iAudioState == eAudioPlaying)
  {
    m_pFilePlayer->Stop();
    m_iAudioState = eAudioReady;
  }

  m_iRemainingDuration = m_iTotalDuration;

  DrawNow();
  return ETrue;
}

TInt CEggClockAppView::GetDuration()
{
  return m_iTotalDuration;
}

void CEggClockAppView::SetDuration(TInt n_iDuration)
{
  if (!IsRunning())
  {
    m_iTotalDuration = n_iDuration;
    m_iRemainingDuration = m_iTotalDuration;
    DrawNow();
  
    // Save settings for future
    TRAP_IGNORE(SaveSettingsL());
  }
}

TBool CEggClockAppView::IsRunning()
{
  if (m_pTimer && m_pTimer->IsActive())
  {
    return ETrue;
  }
  return EFalse;
}

TBool CEggClockAppView::IsReset()
{
  if (m_pTimer && !m_pTimer->IsActive() && m_iTotalDuration == m_iRemainingDuration)
  {
    return ETrue;
  }
  return EFalse;
}

TInt CEggClockAppView::TimerFired(TAny* aPtr)
{
  CEggClockAppView* pThis((CEggClockAppView*)aPtr);
  if (pThis)
  {
    pThis->DoTimerFired();
  }
  return TRUE;
}

void CEggClockAppView::DoTimerFired()
{
  --m_iRemainingDuration;
  if (m_iRemainingDuration == 0)
  {
    PlayNotificationL();
  }
  if (m_iRemainingDuration < 0 && m_iRepeatMinutes > 0 && m_iRepeatMinutes < INFINITE_MINUTES &&
      ((-m_iRemainingDuration) % (60 * m_iRepeatMinutes)) == 0)
  {
    TRAP_IGNORE(PlayNotificationL());
  }
  
  DrawNow();
}

void CEggClockAppView::PlayNotificationL()
{
  // Turn the screensaver off and backlight on
  User::ResetInactivityTime();
  
  // Start flashing (if it is not already)
  if (m_pFlashTimer && !m_pFlashTimer->IsActive())
  {
    m_bFlash = ETrue;
    m_pFlashTimer->Start(FLASH_TIME, FLASH_TIME, TCallBack(CEggClockAppView::FlashTimerFired, this));
  }
  // Start notification
  if (m_pFilePlayer && m_iAudioState == eAudioReady)
  {
    m_pFilePlayer->SetVolume((m_pFilePlayer->MaxVolume() * m_iVolume) / MAX_VOLUME);
    m_pFilePlayer->Play();
    m_iAudioState = eAudioPlaying;
  }
  
  // Bring application to foreground
  CEikonEnv::Static()->RootWin().SetOrdinalPosition(0);
}

TInt CEggClockAppView::FlashTimerFired(TAny* aPtr)
{
  CEggClockAppView* pThis((CEggClockAppView*)aPtr);
  if (pThis)
  {
    pThis->DoFlashTimerFired();
  }
  return TRUE;
}

void CEggClockAppView::DoFlashTimerFired()
{
  m_bFlash = !m_bFlash;
  DrawNow();
}

void CEggClockAppView::SetSkinBackgroundL(TBool bUseSkin)
{
  m_bUseSkin = bUseSkin;

  DrawNow();

  // Save settings for future
  SaveSettingsL();
}

TBool CEggClockAppView::GetSkinBackground()
{
  return m_bUseSkin;
}

void CEggClockAppView::SetNotificationL(const TDesC& aFileName)
{
  m_iNotificationFile.Copy(aFileName);
  
  // Delete old player
  if (m_pFilePlayer)
  {
    if (m_iAudioState == eAudioPlaying)
    {
      m_pFilePlayer->Stop();
      m_pFilePlayer->Close();
    }
    delete m_pFilePlayer;
    m_pFilePlayer = NULL;
  }

  // Create new file player
  m_iAudioState = eAudioIdle;
  if (m_iNotificationFile.Length() > 0)
  {
#ifndef DRM_PLAYER
    m_pFilePlayer = CMdaAudioPlayerUtility::NewFilePlayerL(m_iNotificationFile, *this, EMdaPriorityNormal, EMdaPriorityPreferenceTime);
#else
    m_pFilePlayer = CDrmPlayerUtility::NewFilePlayerL(m_iNotificationFile, *this, EMdaPriorityNormal, EMdaPriorityPreferenceTime);
#endif
    if (m_pFilePlayer)
    {
      m_iAudioState = eAudioIniting;
    }
  }

  // Save settings for future
  SaveSettingsL();
}

void CEggClockAppView::SetRepeatMinutesL(const TInt iMinutes)
{
  m_iRepeatMinutes = iMinutes;
  
  DrawNow();
  
  // Save settings for future
  SaveSettingsL();
}

TInt CEggClockAppView::GetRepeatMinutes()
{
  return m_iRepeatMinutes;
}

void CEggClockAppView::ChangeVolume(TInt iStep)
{
  if (iStep > 0 && m_iVolume < MAX_VOLUME)
  {
    ++m_iVolume;
  }
  if (iStep < 0 && m_iVolume > 0)
  {
    --m_iVolume;
  }
  
  if (m_pFilePlayer && m_iAudioState == eAudioPlaying)
  {
    m_pFilePlayer->SetVolume((m_pFilePlayer->MaxVolume() * m_iVolume) / MAX_VOLUME);
  }
  
  DrawNow();

  TRAP_IGNORE(SaveSettingsL());
}

void CEggClockAppView::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds& /*aDuration*/)
{
  if (KErrNone == aError)
  {
    m_iAudioState = eAudioReady;
  }
  else
  {
    m_iAudioState = eAudioIdle;
    if (m_pFilePlayer)
    {
      delete m_pFilePlayer;
      m_pFilePlayer = NULL;
    }

    TRAP_IGNORE(
      HBufC* pAudioErrorString(CCoeEnv::Static()->AllocReadResourceLC(R_STRING_AUDIO_ERROR));
      CAknErrorNote* pAudioErrorNote(new (ELeave) CAknErrorNote);
      pAudioErrorNote->ExecuteLD(*pAudioErrorString);
      CleanupStack::PopAndDestroy(pAudioErrorString);
    )
  }
}

void CEggClockAppView::MapcPlayComplete(TInt aError)
{
  if (m_iRepeatMinutes == 0 && m_iAudioState == eAudioPlaying && aError == KErrNone)
  {
    m_pFilePlayer->Play();
  }
  else
  {
    m_iAudioState = eAudioReady;
  }
}

void CEggClockAppView::MdapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds& aDuration)
{
  MapcInitComplete(aError, aDuration);
}

void CEggClockAppView::MdapcPlayComplete(TInt aError)
{
  MapcPlayComplete(aError);
}

void CEggClockAppView::SaveSettingsL()
{
  RFs iFileSession;
  if (KErrNone == iFileSession.Connect())
  {
    if (KErrNone == iFileSession.CreatePrivatePath(EDriveC))
    {
      if (KErrNone == iFileSession.SetSessionToPrivate(EDriveC))
      {        
        TFileName iFileName;
        iFileSession.PrivatePath(iFileName);
        iFileName.Append(KSettingsFileName);
        
        RFile iFile;
        if (KErrNone == iFile.Replace(iFileSession, iFileName, EFileWrite))
        {
          SETTINGS_BUF* pSettingsBuf = new SETTINGS_BUF();
          if (pSettingsBuf)
          {
            (*pSettingsBuf)().iNotificationFile.Copy(m_iNotificationFile);
            (*pSettingsBuf)().iTotalTime = m_iTotalDuration;
            (*pSettingsBuf)().iVolume = m_iVolume;
            (*pSettingsBuf)().iRepeatMinutes = m_iRepeatMinutes;
            (*pSettingsBuf)().bUseSkin = m_bUseSkin;
          
            iFile.Write((*pSettingsBuf));
            iFile.Flush();
            
            delete pSettingsBuf;
          }
          iFile.Close();
        }
      }
    }
    iFileSession.Close();
  }
}

void CEggClockAppView::LoadSettingsL()
{
  RFs iFileSession;
  if (KErrNone == iFileSession.Connect())
  {
    if (KErrNone == iFileSession.SetSessionToPrivate(EDriveC))
    {
      TFileName iFileName;
      iFileSession.PrivatePath(iFileName);
      iFileName.Append(KSettingsFileName);
      
      RFile iFile;
      if (KErrNone == iFile.Open(iFileSession, iFileName, EFileRead))
      {
        SETTINGS_BUF* pSettingsBuf = new SETTINGS_BUF();
        if (pSettingsBuf)
        {
          iFile.Read(*pSettingsBuf);

          if ((*pSettingsBuf)().iTotalTime < 0) (*pSettingsBuf)().iTotalTime = 0;
          if ((*pSettingsBuf)().iTotalTime > 5999) (*pSettingsBuf)().iTotalTime = 5999;
          if ((*pSettingsBuf)().iVolume < 0) (*pSettingsBuf)().iVolume = 0;
          if ((*pSettingsBuf)().iVolume > MAX_VOLUME) (*pSettingsBuf)().iVolume = MAX_VOLUME;
          if ((*pSettingsBuf)().iRepeatMinutes < 0) (*pSettingsBuf)().iRepeatMinutes = 0;

          m_iTotalDuration = (*pSettingsBuf)().iTotalTime;
          m_iRemainingDuration = (*pSettingsBuf)().iTotalTime;
          m_iNotificationFile.Copy((*pSettingsBuf)().iNotificationFile);
          m_iVolume = (*pSettingsBuf)().iVolume;
          m_iRepeatMinutes = (*pSettingsBuf)().iRepeatMinutes;
          m_bUseSkin = (*pSettingsBuf)().bUseSkin;
          
          delete pSettingsBuf;

          //iEikonEnv->InfoMsg(m_iNotificationFile);
        }
        iFile.Close();        
      }
    }
    iFileSession.Close();
  }
}

// End of file
