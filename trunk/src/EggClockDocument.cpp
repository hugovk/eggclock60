/*
============================================================================
 Name        : EggClockDocument.cpp
 Author      : Michele Berionne
 Version     :
 Copyright   : 
 Description : Application document class (model)
============================================================================
*/

// INCLUDE FILES
#include "EggClockAppUi.h"
#include "EggClockDocument.h"

// ============================ MEMBER FUNCTIONS ===============================
CEggClockDocument* CEggClockDocument::NewL( CEikApplication& aApp )
{
  CEggClockDocument* self = NewLC( aApp );
  CleanupStack::Pop( self );
  return self;
}

CEggClockDocument* CEggClockDocument::NewLC( CEikApplication& aApp )
{
  CEggClockDocument* self = new ( ELeave ) CEggClockDocument( aApp );
  CleanupStack::PushL( self );
  self->ConstructL();
  return self;
}

void CEggClockDocument::ConstructL()
{
  // No implementation required
}

CEggClockDocument::CEggClockDocument( CEikApplication& aApp ) :
  CAknDocument( aApp )
{
  // No implementation required
}

CEggClockDocument::~CEggClockDocument()
{
  // No implementation required
}

CEikAppUi* CEggClockDocument::CreateAppUiL()
{
  // Create the application user interface, and return a pointer to it;
  // the framework takes ownership of this object
  return ( static_cast <CEikAppUi*> ( new ( ELeave ) CEggClockAppUi ) );
}

// End of File
