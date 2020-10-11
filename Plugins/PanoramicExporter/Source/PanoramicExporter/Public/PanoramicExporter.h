// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#pragma once

#include "Misc/CoreMisc.h"
#include "UObject/Class.h"
#include "GameFramework/Actor.h"

#ifdef UE_4_20_OR_LATER
#include "Launch/Resources/Version.h"
#else
#include "Resources/Version.h"
#endif

#ifdef UE_4_18_OR_LATER
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#else
#include "Interfaces/IImageWrapperModule.h"
#endif

#include "PanoramicExporter.generated.h"

DECLARE_STATS_GROUP(TEXT("Panoramic"), STATGROUP_PANORAMIC, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("ReadRenderTarget"), STAT_ReadRenderTarget, STATGROUP_PANORAMIC);
DECLARE_CYCLE_STAT(TEXT("ExportPanoramicFrame"), STAT_ExportPanoramicFrame, STATGROUP_PANORAMIC);


/**
 * Image format for exporting capture
 */
UENUM()
enum class EPanoramicExportFormat : uint8
{
	PNG,
	JPEG,
	BMP,
	EXR,
	HDR,
};

inline const FString ToExtension(EPanoramicExportFormat ExportFormat)
{
	switch (ExportFormat)
	{
	case EPanoramicExportFormat::PNG: return TEXT("png");
	case EPanoramicExportFormat::JPEG: return TEXT("jpg");
	case EPanoramicExportFormat::BMP: return TEXT("bmp");
	case EPanoramicExportFormat::EXR: return TEXT("exr");
	case EPanoramicExportFormat::HDR: return TEXT("hdr");
	}

	return TEXT("error");
}


UCLASS()
class PANORAMICEXPORTER_API APanoramicExporter : public AActor
{
	GENERATED_BODY()
	
public:	
	APanoramicExporter();
	APanoramicExporter(FVTableHelper& Helper);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick( float DeltaSeconds ) override;

	virtual void PostActorCreated() override;
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category = "Capturing")
	FString CaptureName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category="Capturing")
	EPanoramicExportFormat ExportFormat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category = "Capturing")
	bool bOneFrameCaptureSpinup;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category="Capturing")
	int32 PreRollFrames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category="Capturing")
	int32 FramesToCapture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category = "Capturing")
	bool bUseOverrideFPSOnStart;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn, EditCondition="bUseOverrideFPSOnStart"), Category = "Capturing")
	float OverrideFrameRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category = "Capturing")
	bool bQuitOnFinish;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn), Category = "Capturing")
	int32 CaptureHeight;

	UPROPERTY(VisibleAnywhere, Category="Capturing")
	int32 FrameNumber;

	UPROPERTY(VisibleAnywhere, Category="Capturing")
	FString Timestamp;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient, Category = "Capturing")
	class UTextureRenderTargetCube* RenderTargetCube;

	UPROPERTY()
	class UStaticMeshComponent* MeshComp;

	UPROPERTY()
	class USceneCaptureComponentCube* CubeCapture;

protected:
	IImageWrapperModule& ImageWrapperModule;

	bool bCaptureSpanUp;
	float OriginalOverrideFPS;
	
};
