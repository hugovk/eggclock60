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
#include <aknnotewrappers.h>
#include <avkon.hrh>
#include <caknfileselectiondialog.h>
#include <mgfetch.h>
#include <pathinfo.h>

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
        CAknDurationQueryDialog* dlg(CAknDurationQueryDialog::NewL(iDuration, CAknQueryDialog::ENoTone));
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
      CDesCArray* pSelectedFiles(new (ELeave)CDesCArrayFlat(2));
      CleanupStack::PushL(pSelectedFiles);
      if (MGFetch::RunL(*pSelectedFiles, EAudioFile, EFalse, NULL))
      {
        if (m_pAppContainer)
        {
          TRAP_IGNORE(m_pAppContainer->SetNotificationL((*pSelectedFiles)[0]));
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

      HBufC* pSelectNotification(CCoeEnv::Static()->AllocReadResourceLC(R_STRING_SELECT_NOTIFICATION));
      TBool bSelection(CAknFileSelectionDialog::RunDlgLD(iFilename, PathInfo::PhoneMemoryRootPath(), *pSelectNotification, NULL));
      CleanupStack::PopAndDestroy(pSelectNotification);
      if (bSelection && m_pAppContainer)
      {
        m_pAppContainer->SetNotificationL(iFilename);
      }
*/
      break;
    }
    case EEggClockCmdNotificationRepeat:
    {
      TInt iIndex(0);
      CAknListQueryDialog* dlg(new (ELeave) CAknListQueryDialog(&iIndex));
      dlg->PrepareLC(R_EGGCLOCK_REPETITION_QUERY);

      TInt iSelected(0);
      TInt iMinutes(m_pAppContainer->GetRepeatMinutes());
      TBuf<4> iMinutesDes;
      if (iMinutes >= INFINITE_MINUTES)
      {
        iSelected = 0;
        iMinutesDes.Num(5);
      }
      else if (iMinutes == 0)
      {
        iSelected = 1;
        iMinutesDes.Num(5);
      }
      else
      {
        iSelected = 2;
        iMinutesDes.Num(iMinutes);
      }
      
      CDesCArray* pItemList(new (ELeave) CDesCArrayFlat(3));
      CleanupStack::PushL(pItemList);
      HBufC* pStringOnce(CCoeEnv::Static()->AllocReadResourceLC(R_STRING_REPEAT_ONCE));
      HBufC* pStringLoop(CCoeEnv::Static()->AllocReadResourceLC(R_STRING_REPEAT_LOOP));
      HBufC* pStringEveryX(CCoeEnv::Static()->AllocReadResourceLC(R_STRING_REPEAT_EVERY_X));
      pStringEveryX->Des().Replace(pStringEveryX->Find(_L("XXXX")), 4, iMinutesDes);
      pItemList->AppendL(*pStringOnce);
      pItemList->AppendL(*pStringLoop);
      pItemList->AppendL(*pStringEveryX);
      CleanupStack::PopAndDestroy();  // pStringEveryX
      CleanupStack::PopAndDestroy();  // pStringLoop
      CleanupStack::PopAndDestroy();  // pStringOnce
      CleanupStack::Pop();            // pItemList
      
      dlg->SetItemTextArray(pItemList);
      dlg->SetOwnershipType(ELbmOwnsItemArray);
      dlg->ListBox()->SetCurrentItemIndex(iSelected);

      if (dlg->RunLD())
      {
        if (iIndex == 0)  // Once
        {
          TRAP_IGNORE(m_pAppContainer->SetRepeatMinutesL(INFINITE_MINUTES));
        }
        else if (iIndex == 1)  // Loop
        {
          TRAP_IGNORE(m_pAppContainer->SetRepeatMinutesL(0));
        }
        else if (iIndex == 2)  // Every X minutes
        {
          TInt iNewMinutes(iMinutes);
          if (iMinutes == 0 || iMinutes >= INFINITE_MINUTES)
          {
            iNewMinutes = 5;
          }
          CAknNumberQueryDialog* dlg2(new (ELeave) CAknNumberQueryDialog(iNewMinutes));
          dlg2->PrepareLC(R_EGGCLOCK_REPETITION_MINUTES_QUERY);
          dlg2->SetMinimumAndMaximum(1, 60);
          if (dlg2->RunLD())
          {
            TRAP_IGNORE(m_pAppContainer->SetRepeatMinutesL(iNewMinutes));
          }
        }
      }
      break;
    }
    case EEggClockCmdBackgroundSkin:
      if (m_pAppContainer)
      {
        m_pAppContainer->SetSkinBackgroundL(ETrue);
      }
      break;
    case EEggClockCmdBackgroundWhite:
      if (m_pAppContainer)
      {
        m_pAppContainer->SetSkinBackgroundL(EFalse);
      }
      break;
    case EEggClockCmdAbout:
    {
      HBufC* pVersioString(CCoeEnv::Static()->AllocReadResourceLC(R_STRING_VERSION));
      CAknInformationNote* pAboutNote(new (ELeave) CAknInformationNote);
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
    // Red key to background if running
    case EKeyNo:
      if (m_pAppContainer)
      {
        if (m_pAppContainer->IsRunning())
        {
          CEikonEnv::Static()->RootWin().SetOrdinalPosition(-1);
        }
      }
      return EKeyWasConsumed;    
    default:
      break;
  }
  return EKeyWasNotConsumed;
}

void CEggClockAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
{
  switch (aEvent.Type())
  { 
    case KAknUidValueEndKeyCloseEvent:
      // Nothing to do
      break;
    default:
      // other events to be handled by CAknAppUi::HandleWsEventL
      CAknAppUi::HandleWsEventL(aEvent, aDestination);
      break;
  }
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
  CDesCArrayFlat* pNamesArray(new (ELeave) CDesCArrayFlat(5));
  CleanupStack::PushL(pNamesArray);
  for (TInt i(0); i < 5; ++i) 
  {
    TBuf<20> iName(_L("Schema: "));
    iName.AppendNum(i);
    pNamesArray->AppendL(iName); 
  }

  CleanupStack::Pop(pNamesArray);
  return pNamesArray; 
}

// End of file
