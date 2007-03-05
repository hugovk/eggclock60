/*
============================================================================
 Name        : EggClockAppUi.h
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Main application UI class (controller)
============================================================================
*/

#ifndef __EGGCLOCKAPPUI_H__
#define __EGGCLOCKAPPUI_H__

// INCLUDES
#include <aknappui.h>

// FORWARD DECLARATIONS
class CEggClockAppView;

class CEggClockAppUi : public CAknAppUi
{
  public: // Constructors and destructor
    void ConstructL();
    CEggClockAppUi();
    virtual ~CEggClockAppUi();

  private:  // Functions from base classes
    void HandleCommandL( TInt aCommand );
    void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);
    TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
    void HandleStatusPaneSizeChange();
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
  
  private: // New methods
    MDesCArray* CreateNotificationArrayL() const;    

  private: // Data
    CEggClockAppView* m_pAppContainer;
};

#endif // __EGGCLOCKAPPUI_H__

// End of File
