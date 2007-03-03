/*
============================================================================
 Name        : EggClockAppUi.cpp
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Main application UI class (controller)
============================================================================
*/

// INCLUDE FILES
#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <pathinfo.h>
#include <caknfileselectiondialog.h>
#include <mgfetch.h>

#include <EggClock.rsg>
#include "EggClock.pan"
#include "EggClock.hrh"
#include "EggClockAppUi.h"
#include "EggClockAppView.h"


// ============================ MEMBER FUNCTIONS ===============================
void CEggClockAppUi::ConstructL()
{
  // Initialise app UI with standard value.
  BaseConstructL(EAknEnableSkin);

  // Create view object
  m_pAppContainer = CEggClockAppView::NewL(ClientRect());
}

CEggClockAppUi::CEggClockAppUi()
{
  // No implementation required
}

CEggClockAppUi::~CEggClockAppUi()
{
  if (m_pAppContainer)
  {
    delete m_pAppContainer;
    m_pAppContainer = NULL;
  }
}

void CEggClockAppUi::HandleCommandL(TInt aCommand)
{
  switch( aCommand )
  {
    case EEikCmdExit:
    case EAknSoftkeyExit:
      Exit();
      break;
    case EEggClockCmdStart:
      if (m_pAppContainer)
      {
        m_pAppContainer->StartTimer();
      }
      break;
    case EEggClockCmdStop:
      if (m_pAppContainer)
      {
        m_pAppContainer->StopTimer();
      }
      break;
    case EEggClockCmdDuration:
      if (m_pAppContainer)
      {
        TTimeIntervalSeconds iDuration(m_pAppContainer->GetDuration());
        CAknDurationQueryDialog* dlg = CAknDurationQueryDialog::NewL(iDuration, CAknQueryDialog::ENoTone);
        if (dlg->ExecuteLD(R_EGGCLOCK_DURATION_QUERY_DIALOG))
        {
          m_pAppContainer->SetDuration(iDuration.Int()); 
        }
      }
      break; 
    case EEggClockCmdReset:
      if (m_pAppContainer)
      {
        m_pAppContainer->ResetTimer();
      }
      break;
    case EEggClockCmdNotificationSchemeSilent:
      if (m_pAppContainer)
      {
        _LIT(KEmpty, "");
        m_pAppContainer->SetNotificationL(KEmpty);
      }
      break;
    case EEggClockCmdNotificationSchemeSelect:
    {
      TFileName iFilename;
      CDesCArray* pSelectedFiles = new (ELeave)CDesCArrayFlat(2);
      CleanupStack::PushL(pSelectedFiles);
      if (MGFetch::RunL(*pSelectedFiles, EAudioFile, EFalse, NULL))
      {
        if (m_pAppContainer)
        {
          TRAPD(r, m_pAppContainer->SetNotificationL((*pSelectedFiles)[0]); );
        }
      }
      CleanupStack::PopAndDestroy(pSelectedFiles);
/*
      TVolumeInfo aVol;
      TFileName iFilename;
      if (KErrNone == CEikonEnv::Static()->FsSession().Volume(aVol, EDriveE))
      {
        iFilename.Copy(PathInfo::MemoryCardRootPath());
      }
      else
      {
        iFilename.Copy(PathInfo::PhoneMemoryRootPath());
      }
      iFilename.Append(PathInfo::SoundsPath());

      HBufC* pSelectNotification = CCoeEnv::Static()->AllocReadResourceLC(R_STRING_SELECT_NOTIFICATION);
      TBool bSelection = CAknFileSelectionDialog::RunDlgLD(iFilename, PathInfo::PhoneMemoryRootPath(), *pSelectNotification, NULL);
      CleanupStack::PopAndDestroy(pSelectNotification);
      if (bSelection && m_pAppContainer)
      {
        m_pAppContainer->SetNotificationL(iFilename);
      }
*/
      break;
    }
    case EEggClockCmdAbout:
    {
      HBufC* pVersioString = CCoeEnv::Static()->AllocReadResourceLC(R_STRING_VERSION);
      CAknInformationNote* pAboutNote = new (ELeave) CAknInformationNote;
      pAboutNote->SetTimeout(CAknNoteDialog::ENoTimeout);
      pAboutNote->ExecuteLD(*pVersioString);
      CleanupStack::PopAndDestroy(pVersioString);
      break;
    }
    default:
      Panic(EEggClockUi);
      break;
    }
}

TKeyResponse CEggClockAppUi::HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
{
  switch(aKeyEvent.iCode)
  {
    // Volume up
    case EKeyRightArrow:
    case EKeyUpArrow:
      if (m_pAppContainer)
      {
        m_pAppContainer->ChangeVolume(1);
      }
      return EKeyWasConsumed;
    // Volume down
    case EKeyLeftArrow:
    case EKeyDownArrow:
      if (m_pAppContainer)
      {
        m_pAppContainer->ChangeVolume(-1);
      }
      return EKeyWasConsumed;
    // Reset
    case EKeyBackspace:
      if (m_pAppContainer)
      {
        m_pAppContainer->ResetTimer();
      }
      return EKeyWasConsumed;
    // Start and stop
    case EKeyDevice3:
    case EKeyEnter:
      if (m_pAppContainer)
      {
        if (m_pAppContainer->IsRunning())
        {
          m_pAppContainer->StopTimer();
        }
        else
        {
          m_pAppContainer->StartTimer();
        }
      }
      return EKeyWasConsumed;
    default:
      break;
  }
  return EKeyWasNotConsumed;
}

void CEggClockAppUi::HandleStatusPaneSizeChange()
{
  if (m_pAppContainer)
  {
    m_pAppContainer->SetRect(ClientRect());
  }
}

void CEggClockAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
{
  if (aResourceId == R_EGGCLOCK_MENU)
  {
    if (m_pAppContainer)
    {
      if (m_pAppContainer->IsRunning())
      {
        aMenuPane->SetItemDimmed(EEggClockCmdStart, ETrue);
        aMenuPane->SetItemDimmed(EEggClockCmdDuration, ETrue);
      }
      else
      {
        aMenuPane->SetItemDimmed(EEggClockCmdStop, ETrue);
        if (m_pAppContainer->IsReset())
        {
          aMenuPane->SetItemDimmed(EEggClockCmdReset, ETrue);
        }
      }
    }
  }
}

MDesCArray* CEggClockAppUi::CreateNotificationArrayL() const
{
  CDesCArrayFlat* pNamesArray = new (ELeave) CDesCArrayFlat(5);
  CleanupStack::PushL(pNamesArray);
  for (TInt i = 0; i < 5; i++) 
  {
    TBuf<20> iName;
    iName.Copy(_L("Schema: "));
    iName.AppendNum(i);
    pNamesArray->AppendL(iName); 
  }

  CleanupStack::Pop(pNamesArray);
  return pNamesArray; 
}
