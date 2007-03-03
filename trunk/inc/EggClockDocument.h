/*
============================================================================
 Name        : EggClockDocument.h
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application document class (model)
============================================================================
*/

#ifndef __EGGCLOCKDOCUMENT_H__
#define __EGGCLOCKDOCUMENT_H__

// INCLUDES
#include <akndoc.h>

// FORWARD DECLARATIONS
class CEggClockAppUi;
class CEikApplication;


// CLASS DECLARATION

class CEggClockDocument : public CAknDocument
{
  public: // Constructors and destructor
    static CEggClockDocument* NewL( CEikApplication& aApp );
    static CEggClockDocument* NewLC( CEikApplication& aApp );
    virtual ~CEggClockDocument();

  public: // Functions from base classes
    CEikAppUi* CreateAppUiL();

  private: // Constructors
    void ConstructL();
    CEggClockDocument( CEikApplication& aApp );
};

#endif // __EGGCLOCKDOCUMENT_H__

// End of File
