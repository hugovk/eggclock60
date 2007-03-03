/*
============================================================================
 Name        : EggClockView.cpp
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application container
============================================================================
*/

// INCLUDE FILES
#include <coemain.h>
#include <eikenv.h>
#include <akniconarray.h>     // Icon array
#include <gulicon.h>          // Icons
#include <aknnotewrappers.h>  // Error note
#include <AknsDrawUtils.h>    // Skin support
#include <AknsBasicBackgroundControlContext.h>

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
};
typedef TPckgBuf<stSettings>  SETTINGS_BUF;


// ============================ MEMBER FUNCTIONS ===============================
CEggClockAppView* CEggClockAppView::NewL( const TRect& aRect )
{
  CEggClockAppView* self = CEggClockAppView::NewLC( aRect );
  CleanupStack::Pop( self );
  return self;
}

CEggClockAppView* CEggClockAppView::NewLC( const TRect& aRect )
{
  CEggClockAppView* self = new (ELeave) CEggClockAppView;
  CleanupStack::PushL( self );
  self->ConstructL( aRect );
  return self;
}

void CEggClockAppView::ConstructL( const TRect& aRect )
{
  // Create the timers
  m_pTimer = CPeriodic::NewL(CActive::EPriorityIdle);
  m_pFlashTimer = CPeriodic::NewL(CActive::EPriorityIdle);

  // Load icons with numbers
  _LIT(KEggClockIconsFilename, "\\resource\\apps\\EggClock_numbers_icons.mbm");
  m_pDigitIcons = new (ELeave) CAknIconArray(10);
  for (TInt i = 0; i < 10; i++)
  {
    CFbsBitmap* pBitmap = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_icons0 + i);
    CleanupStack::PushL(pBitmap);  
    m_pDigitIcons->AppendL(CGulIcon::NewL(pBitmap, pBitmap));
    CleanupStack::Pop(); // pBitmap
  }
  
  // Load icon for colon
  CFbsBitmap* pColonBitmap = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsColon);
  CleanupStack::PushL(pColonBitmap);
  m_pColonIcon = CGulIcon::NewL(pColonBitmap, pColonBitmap);
  CleanupStack::Pop(); // pColonBitmap

  // Load icons for volume
  CFbsBitmap* pVolumeBitmap = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume);
  CleanupStack::PushL(pVolumeBitmap);
  CFbsBitmap* pVolumeBitmapMask = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_mask);
  CleanupStack::PushL(pVolumeBitmapMask);
  m_pVolumeIcon = CGulIcon::NewL(pVolumeBitmap, pVolumeBitmapMask);
  CleanupStack::Pop(); // pVolumeBitmapMask
  CleanupStack::Pop(); // pVolumeBitmap
  CFbsBitmap* pVolumeMuteBitmap = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_mute);
  CleanupStack::PushL(pVolumeMuteBitmap);
  CFbsBitmap* pVolumeMuteBitmapMask = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_mute_mask);
  CleanupStack::PushL(pVolumeMuteBitmapMask);
  m_pVolumeMuteIcon = CGulIcon::NewL(pVolumeMuteBitmap, pVolumeMuteBitmapMask);
  CleanupStack::Pop(); // pVolumeMuteBitmapMask
  CleanupStack::Pop(); // m_pVolumeMuteIcon

  m_pVolumeValueIcons = new (ELeave) CAknIconArray(MAX_VOLUME + 1);
  for (TInt i = 0; i <= MAX_VOLUME; i++)
  {
    CFbsBitmap* pBitmap = iEikonEnv->CreateBitmapL(KEggClockIconsFilename, EMbmEggclock_numbers_iconsVolume_0 + i);
    CleanupStack::PushL(pBitmap);  
    m_pVolumeValueIcons->AppendL(CGulIcon::NewL(pBitmap, pBitmap));
    CleanupStack::Pop(); // pBitmap
  }
  
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
  
  
  // Create a window for this application view and set the size
  CreateWindowL();
  SetRect(aRect);

  // Skin support
  m_pSkinBackground = CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain, Rect(), EFalse);

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
}

TTypeUid::Ptr CEggClockAppView::MopSupplyObject(TTypeUid aId)
{
  if(aId.iUid == MAknsControlContext::ETypeId && m_pSkinBackground)
  {
    return MAknsControlContext::SupplyMopObject(aId, m_pSkinBackground);
  }
  return CCoeControl::MopSupplyObject(aId);
}

void CEggClockAppView::Draw(const TRect& aRect) const
{
  // Get the standard graphics context
  CWindowGc& gc = SystemGc();

  // Clears the screen
  if (m_pFlashTimer && m_pFlashTimer->IsActive() && m_bFlash)
  {
    gc.SetBrushColor(TRgb(255, 0, 0));
    gc.Clear(Rect());
  }
  else
  {
    // Skin support
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
    AknsDrawUtils::Background(skin, cc, this, gc, aRect);
  }

  // Calculate numbers to show
  TInt iLargeNumber = (m_iRemainingDuration > 0) ? m_iRemainingDuration : 0;
  TInt iSmallNumber = (m_iRemainingDuration < 0) ? (0 - m_iRemainingDuration) : 0;

  // Large numbers
  TRAPD(r,
    DrawLargeNumberL(aRect, iLargeNumber);
    DrawSmallNumberL(aRect, iSmallNumber);
    DrawVolumeL(aRect);
  );
}

void CEggClockAppView::DrawLargeNumberL(const TRect& /*aRect*/, TInt iNumber) const
{
  TInt i;
  TInt iXPosition, iYPosition;
  
  // Get container size
  TSize iScreenSize = Size();

  // Prepare array with bitmaps
  CFbsBitmap* ppDigitBitmap[5];
  ppDigitBitmap[0] = m_pDigitIcons->At((iNumber / 60) / 10)->Bitmap();
  ppDigitBitmap[1] = m_pDigitIcons->At((iNumber / 60) % 10)->Bitmap();
  ppDigitBitmap[2] = m_pColonIcon->Bitmap();
  ppDigitBitmap[3] = m_pDigitIcons->At((iNumber % 60) / 10)->Bitmap();
  ppDigitBitmap[4] = m_pDigitIcons->At((iNumber % 60) % 10)->Bitmap();
  
  // Calculate zoom factor
  TInt iTotalWidth = ppDigitBitmap[0]->SizeInPixels().iWidth + ppDigitBitmap[1]->SizeInPixels().iWidth +
                     ppDigitBitmap[2]->SizeInPixels().iWidth + ppDigitBitmap[3]->SizeInPixels().iWidth +
                     ppDigitBitmap[4]->SizeInPixels().iWidth + (2 * CLOCK_SIDE_PADDING) + (4 * DIGIT_PADDING);
  TInt iZoomFactor = (100 * iScreenSize.iWidth) / iTotalWidth;

  // Calculate X and Y
  iXPosition = CLOCK_SIDE_PADDING * iZoomFactor / 100;
  iYPosition = 0;
  for (i = 0; i < 5; i++)
  {
    iYPosition = (ppDigitBitmap[0]->SizeInPixels().iHeight > iYPosition) ? ppDigitBitmap[0]->SizeInPixels().iHeight : iYPosition;
  }
  iYPosition = (iScreenSize.iHeight - (iYPosition * iZoomFactor / 100)) / 2;

  // Draw bitmaps
  CWindowGc& gc = SystemGc();
  for (i = 0; i < 5; i++)
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
    CWindowGc& gc = SystemGc();

    TBuf<128> iOvertimeString;
    iCoeEnv->ReadResourceL(iOvertimeString, R_STRING_OVERTIME);

    iOvertimeString.Append(' ');
    iOvertimeString.AppendNum((iNumber / 60) / 10);
    iOvertimeString.AppendNum((iNumber / 60) % 10);
    iOvertimeString.Append(':');
    iOvertimeString.AppendNum((iNumber % 60) / 10);
    iOvertimeString.AppendNum((iNumber % 60) % 10);
    
    TRgb iFontColor(0, 0, 0);
    const CFont* iFont = iEikonEnv->DenseFont();
    gc.SetPenColor(iFontColor);
    gc.UseFont(iFont);
    
    gc.DrawText(iOvertimeString, TPoint(CLOCK_SIDE_PADDING, Size().iHeight - CLOCK_SIDE_PADDING));
    gc.DiscardFont();
  }
}

void CEggClockAppView::DrawVolumeL(const TRect& /*aRect*/) const
{
  if (m_iVolume >= 0 && m_iVolume <= MAX_VOLUME)
  {
    CWindowGc& gc = SystemGc();
  
    CFbsBitmap* pVolume = (m_iNotificationFile.Length() > 0 && m_iAudioState != eAudioIdle) ? m_pVolumeIcon->Bitmap() : m_pVolumeMuteIcon->Bitmap();
    CFbsBitmap* pVolumeValue = m_pVolumeValueIcons->At(m_iVolume)->Bitmap();
    
    if (pVolume && pVolumeValue)
    {
      TInt iTotalWidth = pVolume->SizeInPixels().iWidth + DIGIT_PADDING +
                         pVolumeValue->SizeInPixels().iWidth + CLOCK_SIDE_PADDING;
      TPoint iPoint(Size().iWidth - iTotalWidth, CLOCK_SIDE_PADDING);
  
      gc.DrawBitmapMasked(TRect(iPoint, pVolume->SizeInPixels()), pVolume, TRect(pVolume->SizeInPixels()), pVolume, ETrue);
      gc.DrawBitmapMasked(TRect(iPoint + TPoint(pVolume->SizeInPixels().iWidth + DIGIT_PADDING, 0),
                          pVolumeValue->SizeInPixels()), pVolumeValue, TRect(pVolumeValue->SizeInPixels()), pVolumeValue, ETrue);
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
  m_iTotalDuration = n_iDuration;
  if (!IsRunning())
  {
    m_iRemainingDuration = m_iTotalDuration;
    DrawNow();
  }
  
  // Save settings for future
  TRAPD(r, SaveSettingsL(); );
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
  CEggClockAppView* pThis = (CEggClockAppView*)aPtr;
  if (pThis)
  {
    pThis->DoTimerFired();
  }
  return TRUE;
}

void CEggClockAppView::DoTimerFired()
{
  m_iRemainingDuration--;
  if (m_iRemainingDuration == 0)
  {
    PlayNotificationL();
  }
  
  DrawNow();
}

void CEggClockAppView::PlayNotificationL()
{
  if (m_pFlashTimer && !m_pFlashTimer->IsActive())
  {
    m_bFlash = ETrue;
    m_pFlashTimer->Start(FLASH_TIME, FLASH_TIME, TCallBack(CEggClockAppView::FlashTimerFired, this));
  }
  if (m_pFilePlayer && m_iAudioState == eAudioReady)
  {
    m_pFilePlayer->SetVolume((m_pFilePlayer->MaxVolume() * m_iVolume) / MAX_VOLUME);
    m_pFilePlayer->Play();
    m_iAudioState = eAudioPlaying;
  }
}

TInt CEggClockAppView::FlashTimerFired(TAny* aPtr)
{
  CEggClockAppView* pThis = (CEggClockAppView*)aPtr;
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

void CEggClockAppView::ChangeVolume(TInt iStep)
{
  if (iStep > 0 && m_iVolume < MAX_VOLUME)
  {
    m_iVolume++;
  }
  if (iStep < 0 && m_iVolume > 0)
  {
    m_iVolume--;
  }
  
  if (m_pFilePlayer && m_iAudioState == eAudioPlaying)
  {
    m_pFilePlayer->SetVolume((m_pFilePlayer->MaxVolume() * m_iVolume) / MAX_VOLUME);
  }
  
  DrawNow();

  TRAPD(r, SaveSettingsL(); );
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

    TRAPD(r,
      HBufC* pAudioErrorString = CCoeEnv::Static()->AllocReadResourceLC(R_STRING_AUDIO_ERROR);
      CAknErrorNote* pAudioErrorNote = new (ELeave) CAknErrorNote;
      pAudioErrorNote->ExecuteLD(*pAudioErrorString);
      CleanupStack::PopAndDestroy(pAudioErrorString);
    )    
  }
}

void CEggClockAppView::MapcPlayComplete(TInt /*aError*/)
{
  m_iAudioState = eAudioReady;
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

          m_iTotalDuration = (*pSettingsBuf)().iTotalTime;
          m_iRemainingDuration = (*pSettingsBuf)().iTotalTime;
          m_iNotificationFile.Copy((*pSettingsBuf)().iNotificationFile);
          m_iVolume = (*pSettingsBuf)().iVolume;
                    
          delete pSettingsBuf;

          //iEikonEnv->InfoMsg(m_iNotificationFile);
        }
        iFile.Close();        
      }
    }
    iFileSession.Close();
  }
}