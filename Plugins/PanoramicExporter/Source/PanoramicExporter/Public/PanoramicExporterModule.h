// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#pragma once

class FPanoramicExporterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};