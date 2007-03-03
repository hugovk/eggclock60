/*
============================================================================
 Name        : EggClockApplication.h
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Main application class
============================================================================
*/

#ifndef __EGGCLOCKAPPLICATION_H__
#define __EGGCLOCKAPPLICATION_H__

// INCLUDES
#include <aknapp.h>

// CLASS DECLARATION

class CEggClockApplication : public CAknApplication
{
  public: // Functions from base classes
    TUid AppDllUid() const;

  protected: // Functions from base classes
    CApaDocument* CreateDocumentL();
};

#endif // __EGGCLOCKAPPLICATION_H__

// End of File
