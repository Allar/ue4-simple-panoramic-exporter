// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#include "PanoramicExporter.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/CollisionProfile.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Engine/SceneCaptureCube.h"
#include "Engine/StaticMesh.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Components/StaticMeshComponent.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "UObject/Object.h"

#if WITH_EDITOR
#include "CubemapUnwrapUtils.h"
#endif

#ifdef UE_4_20_OR_LATER
#include "Launch/Resources/Version.h"
#else
#include "Resources/Version.h"
#endif

// Sets default values
APanoramicExporter::APanoramicExporter()
	: ExportFormat(EPanoramicExportFormat::HDR)
	, bOneFrameCaptureSpinup(true)
	, PreRollFrames(0)
	, FramesToCapture(0)
	, bUseOverrideFPSOnStart(true)
	, OverrideFrameRate(30.0f)
	, bQuitOnFinish(false)
	, CaptureHeight(256)
	, FrameNumber(-1)
	, RenderTargetCube(nullptr)
	, ImageWrapperModule(FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")))
	
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh"));
	MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComp->bHiddenInGame = true;
	MeshComp->CastShadow = false;
	MeshComp->SetComponentTickEnabled(false);
	RootComponent = MeshComp;

	CubeCapture = CreateDefaultSubobject<USceneCaptureComponentCube>(TEXT("CubeCapture"));
	CubeCapture->bCaptureEveryFrame = false;
	CubeCapture->SetupAttachment(RootComponent);

	CubeCapture->TextureTarget = RenderTargetCube;
}

APanoramicExporter::APanoramicExporter(FVTableHelper& Helper)
	: ExportFormat(EPanoramicExportFormat::HDR)
	, bOneFrameCaptureSpinup(true)
	, PreRollFrames(0)
	, FramesToCapture(0)
	, bUseOverrideFPSOnStart(true)
	, OverrideFrameRate(30.0f)
	, bQuitOnFinish(false)
	, CaptureHeight(256)
	, FrameNumber(-1)
	, RenderTargetCube(nullptr)
	, ImageWrapperModule(FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")))
	, OriginalOverrideFPS(0)
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CamMesh"));
	MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComp->bHiddenInGame = true;
	MeshComp->CastShadow = false;
	MeshComp->SetComponentTickEnabled(false);

	RootComponent = MeshComp;

	CubeCapture = CreateDefaultSubobject<USceneCaptureComponentCube>(TEXT("CubeCapture"));
	CubeCapture->bCaptureEveryFrame = false;
	CubeCapture->SetupAttachment(RootComponent);
	CubeCapture->TextureTarget;
}
// Called when the game starts or when spawned
void APanoramicExporter::BeginPlay()
{
	Super::BeginPlay();
	Timestamp = FDateTime::Now().ToString();
	if (RenderTargetCube == nullptr)
	{
		RenderTargetCube = NewObject<UTextureRenderTargetCube>(this, TEXT("PanoramicRenderTargetCube"), RF_Transient);
		RenderTargetCube->SizeX = CaptureHeight;
		RenderTargetCube->bHDR = ExportFormat == EPanoramicExportFormat::HDR;
		RenderTargetCube->bNeedsTwoCopies = true;
		RenderTargetCube->InitAutoFormat(CaptureHeight);
		RenderTargetCube->UpdateResource();
		CubeCapture->TextureTarget = RenderTargetCube;
	}

	CubeCapture->UpdateContent();
	FlushRenderingCommands();

	if (bUseOverrideFPSOnStart)
	{
		IConsoleManager& ConsoleMan = IConsoleManager::Get();
		IConsoleVariable* OverrideFPS = ConsoleMan.FindConsoleVariable(TEXT("t.overrideFPS"));
		check(OverrideFPS);
		OriginalOverrideFPS = OverrideFPS->GetFloat();
		OverrideFPS->Set(OverrideFrameRate, ECVF_SetByCode);
	}	
}

void APanoramicExporter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RenderTargetCube = nullptr;
	CubeCapture->TextureTarget = nullptr;
	if (bUseOverrideFPSOnStart)
	{
		IConsoleManager& ConsoleMan = IConsoleManager::Get();
		IConsoleVariable* OverrideFPS = ConsoleMan.FindConsoleVariable(TEXT("t.overrideFPS"));
		check(OverrideFPS);
		OverrideFPS->Set(OriginalOverrideFPS, ECVF_SetByCode);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void APanoramicExporter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	++FrameNumber;

#if !WITH_EDITOR
	return
#endif

	if (PreRollFrames > 0 && FrameNumber < PreRollFrames)
	{
		CubeCapture->bCaptureEveryFrame = false;
		return;
	}

	if (FramesToCapture > 0 && FrameNumber >= PreRollFrames + FramesToCapture)
	{
		CubeCapture->bCaptureEveryFrame = false;
		if (bQuitOnFinish)
		{
			UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
		}
		return;
	}

	if (!CubeCapture->bCaptureEveryFrame)
	{
		if (Timestamp.IsEmpty())
		{
			Timestamp = FDateTime::Now().ToString();
		}
		CubeCapture->bCaptureEveryFrame = true;
		CubeCapture->UpdateContent();
		FlushRenderingCommands();
	}

	if (bOneFrameCaptureSpinup && !bCaptureSpanUp)
	{
		FrameNumber--;
		CubeCapture->UpdateContent();
		FlushRenderingCommands();
		bCaptureSpanUp = true;
		return;
	}

	if (RenderTargetCube == nullptr)
	{
		return;
	}
#if ENGINE_MINOR_VERSION < 18
	const FString FramePath = FPaths::Combine(*FPaths::GameSavedDir(), TEXT("Panoramas"), *FString::Printf(TEXT("%s%s%s"), *CaptureName, CaptureName.IsEmpty() ? TEXT("") : TEXT("_"), *Timestamp), *FString::Printf(TEXT("Frame_%05d.%s"), FrameNumber, *ToExtension(ExportFormat)));
#else
	const FString FramePath = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Panoramas"), *FString::Printf(TEXT("%s%s%s"), *CaptureName, CaptureName.IsEmpty() ? TEXT("") : TEXT("_"), *Timestamp), *FString::Printf(TEXT("Frame_%05d.%s"), FrameNumber, *ToExtension(ExportFormat)));
#endif

	if (ExportFormat == EPanoramicExportFormat::HDR)
	{
#if ENGINE_MINOR_VERSION < 20
		TScopedPointer<FArchive> FileWriter(IFileManager::Get().CreateFileWriter(*FramePath));
#else
		TUniquePtr<FArchive> FileWriter(IFileManager::Get().CreateFileWriter(*FramePath));
#endif
		if (FileWriter != nullptr)
		{
			FImageUtils::ExportRenderTargetCubeAsHDR(RenderTargetCube, *FileWriter);
		}
		return;
	}

	// Fun fact: SizeX refers to height
	const int32 Width = CubeCapture->TextureTarget->SizeX*2;
	const int32 Height = CubeCapture->TextureTarget->SizeX;

	TArray64<uint8> RawData;
	
	{
		SCOPE_CYCLE_COUNTER(STAT_ReadRenderTarget);
		FTextureRenderTargetResource* RenderTarget = CubeCapture->TextureTarget->GameThread_GetRenderTargetResource();
		RawData.AddUninitialized(Width * Height);
		
		FIntPoint Size;
		EPixelFormat PixelFormat;
		if (!CubemapHelpers::GenerateLongLatUnwrap(RenderTargetCube, RawData, Size, PixelFormat))
		{
			return;
		}

		// Force Alpha to be 255 as default is a bad value
		for (int32 i = 3; i < RawData.Num(); i += 4)
		{
			RawData[i] = 255;
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_ExportPanoramicFrame);
#if ENGINE_MINOR_VERSION < 18
		IImageWrapperPtr ImageWrapper = nullptr;
#else
		TSharedPtr<IImageWrapper> ImageWrapper;
#endif
		switch (ExportFormat)
		{
			case EPanoramicExportFormat::BMP:
				FFileHelper::CreateBitmap(*FramePath, Width, Height, (FColor*)RawData.GetData());
				break;
			case EPanoramicExportFormat::PNG:

				ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
				if (ImageWrapper->SetRaw(RawData.GetData(), RawData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8))
				{
					FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(100), *FramePath);
				}
				break;
			case EPanoramicExportFormat::JPEG:
				ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
				if (ImageWrapper->SetRaw(RawData.GetData(), RawData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8))
				{
					FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(100), *FramePath);
				}
				break;
			case EPanoramicExportFormat::EXR:
				ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);
				if (ImageWrapper->SetRaw(RawData.GetData(), RawData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8))
				{
#if ENGINE_MINOR_VERSION < 18
					FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(ImageCompression::CompressionQuality::Uncompressed), *FramePath);
#else
					FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed((int32)EImageCompressionQuality::Uncompressed), *FramePath);
#endif
				}
				break;				
		}

		ImageWrapper.Reset();
	}
}

void APanoramicExporter::PostActorCreated()
{
	Super::PostActorCreated();

	// no need load the editor mesh when there is no editor
#if WITH_EDITOR
	if (MeshComp)
	{
		if (!IsRunningCommandlet())
		{
			UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();

			if (StaticMesh == nullptr)
			{
				UStaticMesh* CamMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM"), NULL, LOAD_None, NULL);
				MeshComp->SetStaticMesh(CamMesh);
			}
		}
	}
#endif
}

#if WITH_EDITOR
void APanoramicExporter::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	if (bFinished)
	{
		CubeCapture->UpdateContent();
	}
}
#endif

