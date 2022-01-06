// Copyright Epic Games, Inc. All Rights Reserved.

#include "ART.h"

#if WITH_EDITOR
DEFINE_LOG_CATEGORY(LogOrder)
#endif

#define LOCTEXT_NAMESPACE "FARTModule"

void FARTModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FARTModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FARTModule, ART)