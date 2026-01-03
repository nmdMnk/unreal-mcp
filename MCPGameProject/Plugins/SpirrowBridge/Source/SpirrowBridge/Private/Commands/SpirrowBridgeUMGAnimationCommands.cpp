#include "Commands/SpirrowBridgeUMGAnimationCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Blueprint/UserWidget.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
// Animation support
#include "Animation/WidgetAnimation.h"
#include "MovieScene.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneColorTrack.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieSceneColorSection.h"
#include "Channels/MovieSceneFloatChannel.h"

FSpirrowBridgeUMGAnimationCommands::FSpirrowBridgeUMGAnimationCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGAnimationCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("create_widget_animation"))
	{
		return HandleCreateWidgetAnimation(Params);
	}
	else if (CommandName == TEXT("add_animation_track"))
	{
		return HandleAddAnimationTrack(Params);
	}
	else if (CommandName == TEXT("add_animation_keyframe"))
	{
		return HandleAddAnimationKeyframe(Params);
	}
	else if (CommandName == TEXT("get_widget_animations"))
	{
		return HandleGetWidgetAnimations(Params);
	}

	return nullptr;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGAnimationCommands::HandleCreateWidgetAnimation(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, AnimationName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("animation_name"), AnimationName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));

	double LengthDouble;
	FSpirrowBridgeCommonUtils::GetOptionalNumber(Params, TEXT("length"), LengthDouble, 1.0);
	float Length = static_cast<float>(LengthDouble);

	bool bLoop;
	FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("loop"), bLoop, false);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Check if animation already exists
	for (UWidgetAnimation* ExistingAnim : WidgetBP->Animations)
	{
		if (ExistingAnim && ExistingAnim->GetFName() == FName(*AnimationName))
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::AssetAlreadyExists,
				FString::Printf(TEXT("Animation '%s' already exists in Widget Blueprint"), *AnimationName));
		}
	}

	// Create new Widget Animation
	UWidgetAnimation* NewAnimation = NewObject<UWidgetAnimation>(
		WidgetBP,
		UWidgetAnimation::StaticClass(),
		FName(*AnimationName),
		RF_Transactional
	);

	if (!NewAnimation)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::AssetCreationFailed,
			TEXT("Failed to create Widget Animation"));
	}

	// Create MovieScene for the animation
	UMovieScene* MovieScene = NewObject<UMovieScene>(
		NewAnimation,
		UMovieScene::StaticClass(),
		NAME_None,
		RF_Transactional
	);

	if (!MovieScene)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::AssetCreationFailed,
			TEXT("Failed to create MovieScene"));
	}

	NewAnimation->MovieScene = MovieScene;

	// Set playback range
	FFrameRate TickResolution = MovieScene->GetTickResolution();
	FFrameNumber StartFrame = 0;
	FFrameNumber EndFrame = FFrameNumber(static_cast<int32>(Length * TickResolution.AsDecimal()));

	MovieScene->SetPlaybackRange(TRange<FFrameNumber>(StartFrame, EndFrame));
	MovieScene->SetWorkingRange(StartFrame.Value / TickResolution.AsDecimal(), EndFrame.Value / TickResolution.AsDecimal());
	MovieScene->SetViewRange(StartFrame.Value / TickResolution.AsDecimal(), EndFrame.Value / TickResolution.AsDecimal());

	// Set loop mode
	if (bLoop)
	{
		MovieScene->SetEvaluationType(EMovieSceneEvaluationType::WithSubFrames);
	}

	// Add animation to Widget Blueprint
	WidgetBP->Animations.Add(NewAnimation);

	// Mark Blueprint as modified and compile
	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("animation_name"), AnimationName);
	ResultObj->SetStringField(TEXT("animation_id"), NewAnimation->GetPathName());
	ResultObj->SetNumberField(TEXT("length"), Length);
	ResultObj->SetBoolField(TEXT("loop"), bLoop);

	return ResultObj;
}

// Helper: Find Float Track by Property Name
static UMovieSceneFloatTrack* FindFloatTrackByPropertyName(UMovieScene* MovieScene, const FGuid& BindingGuid, const FString& PropertyName)
{
	if (!MovieScene)
	{
		return nullptr;
	}

	const FMovieSceneBinding* Binding = MovieScene->FindBinding(BindingGuid);
	if (!Binding)
	{
		return nullptr;
	}

	for (UMovieSceneTrack* Track : Binding->GetTracks())
	{
		if (UMovieSceneFloatTrack* FloatTrack = Cast<UMovieSceneFloatTrack>(Track))
		{
			if (FloatTrack->GetTrackName().ToString() == PropertyName ||
				FloatTrack->GetPropertyPath().ToString() == PropertyName)
			{
				return FloatTrack;
			}
		}
	}

	return nullptr;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGAnimationCommands::HandleAddAnimationTrack(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, AnimationName, TargetWidgetName, PropertyName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("animation_name"), AnimationName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("target_widget"), TargetWidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("property_name"), PropertyName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Find the animation
	UWidgetAnimation* Animation = nullptr;
	for (UWidgetAnimation* Anim : WidgetBP->Animations)
	{
		if (Anim && Anim->GetName() == AnimationName)
		{
			Animation = Anim;
			break;
		}
	}

	if (!Animation)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::AssetNotFound,
			FString::Printf(TEXT("Animation not found: %s"), *AnimationName));
	}

	UMovieScene* MovieScene = Animation->GetMovieScene();
	if (!MovieScene)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidOperation,
			TEXT("MovieScene not found in animation"));
	}

	// Find target widget
	UWidget* TargetWidget = nullptr;
	if (WidgetBP->WidgetTree)
	{
		TargetWidget = WidgetBP->WidgetTree->FindWidget(FName(*TargetWidgetName));
	}

	if (!TargetWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetElementNotFound,
			FString::Printf(TEXT("Target widget not found: %s"), *TargetWidgetName));
	}

	// Find or create Possessable (binding target)
	FGuid BindingGuid;
	bool bFoundBinding = false;

	for (int32 i = 0; i < MovieScene->GetPossessableCount(); ++i)
	{
		const FMovieScenePossessable& Possessable = MovieScene->GetPossessable(i);
		if (Possessable.GetName() == TargetWidgetName)
		{
			BindingGuid = Possessable.GetGuid();
			bFoundBinding = true;
			break;
		}
	}

	if (!bFoundBinding)
	{
		// Create new Possessable
		BindingGuid = MovieScene->AddPossessable(TargetWidgetName, TargetWidget->GetClass());

		// Add binding to Animation's AnimationBindings
		FWidgetAnimationBinding Binding;
		Binding.WidgetName = FName(*TargetWidgetName);
		Binding.SlotWidgetName = NAME_None;
		Binding.AnimationGuid = BindingGuid;
		Binding.bIsRootWidget = false;
		Animation->AnimationBindings.Add(Binding);
	}

	// Create track based on property
	FString TrackDisplayName;

	if (PropertyName == TEXT("Opacity") || PropertyName == TEXT("RenderOpacity"))
	{
		UMovieSceneFloatTrack* Track = MovieScene->AddTrack<UMovieSceneFloatTrack>(BindingGuid);
		if (Track)
		{
			Track->SetPropertyNameAndPath(FName("RenderOpacity"), "RenderOpacity");

			UMovieSceneFloatSection* Section = Cast<UMovieSceneFloatSection>(Track->CreateNewSection());
			if (Section)
			{
				Section->SetRange(TRange<FFrameNumber>::All());
				Track->AddSection(*Section);
			}

			TrackDisplayName = TEXT("RenderOpacity");
		}
	}
	else if (PropertyName == TEXT("ColorAndOpacity"))
	{
		UMovieSceneColorTrack* Track = MovieScene->AddTrack<UMovieSceneColorTrack>(BindingGuid);
		if (Track)
		{
			Track->SetPropertyNameAndPath(FName("ColorAndOpacity"), "ColorAndOpacity");

			UMovieSceneColorSection* Section = Cast<UMovieSceneColorSection>(Track->CreateNewSection());
			if (Section)
			{
				Section->SetRange(TRange<FFrameNumber>::All());
				Track->AddSection(*Section);
			}

			TrackDisplayName = TEXT("ColorAndOpacity");
		}
	}
	else if (PropertyName.StartsWith(TEXT("RenderTransform.")))
	{
		UMovieSceneFloatTrack* Track = MovieScene->AddTrack<UMovieSceneFloatTrack>(BindingGuid);
		if (!Track)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::OperationFailed,
				TEXT("Failed to create float track for RenderTransform"));
		}

		Track->SetPropertyNameAndPath(FName(*PropertyName), PropertyName);

		UMovieSceneFloatSection* Section = Cast<UMovieSceneFloatSection>(Track->CreateNewSection());
		if (Section)
		{
			Section->SetRange(TRange<FFrameNumber>::All());
			Track->AddSection(*Section);
		}

		TrackDisplayName = PropertyName;
	}
	else
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidParameter,
			FString::Printf(TEXT("Unsupported property: %s. Supported: Opacity, RenderOpacity, ColorAndOpacity, RenderTransform.*"), *PropertyName));
	}

	// Save
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	// Create response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetStringField(TEXT("animation_name"), AnimationName);
	Response->SetStringField(TEXT("target_widget"), TargetWidgetName);
	Response->SetStringField(TEXT("property_name"), TrackDisplayName);
	Response->SetStringField(TEXT("binding_guid"), BindingGuid.ToString());

	return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGAnimationCommands::HandleAddAnimationKeyframe(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, AnimationName, TargetWidgetName, PropertyName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("animation_name"), AnimationName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("target_widget"), TargetWidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("property_name"), PropertyName))
	{
		return Error;
	}
	if (!Params->HasField(TEXT("time")) || !Params->HasField(TEXT("value")))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::MissingRequiredParam,
			TEXT("Missing 'time' or 'value' parameter"));
	}

	double Time = Params->GetNumberField(TEXT("time"));

	// Get optional parameters
	FString Interpolation, Path;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("interpolation"), Interpolation, TEXT("Linear"));
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Find the animation
	UWidgetAnimation* Animation = nullptr;
	for (UWidgetAnimation* Anim : WidgetBP->Animations)
	{
		if (Anim && Anim->GetName() == AnimationName)
		{
			Animation = Anim;
			break;
		}
	}

	if (!Animation)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::AssetNotFound,
			FString::Printf(TEXT("Animation not found: %s"), *AnimationName));
	}

	UMovieScene* MovieScene = Animation->GetMovieScene();
	if (!MovieScene)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidOperation,
			TEXT("MovieScene not found"));
	}

	// Find Possessable binding GUID
	FGuid BindingGuid;
	bool bFoundBinding = false;

	for (int32 i = 0; i < MovieScene->GetPossessableCount(); ++i)
	{
		const FMovieScenePossessable& Possessable = MovieScene->GetPossessable(i);
		if (Possessable.GetName() == TargetWidgetName)
		{
			BindingGuid = Possessable.GetGuid();
			bFoundBinding = true;
			break;
		}
	}

	if (!bFoundBinding)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidOperation,
			FString::Printf(TEXT("No binding found for widget: %s. Add a track first."), *TargetWidgetName));
	}

	// Calculate frame number
	FFrameRate TickResolution = MovieScene->GetTickResolution();
	FFrameNumber FrameNumber = (Time * TickResolution).FloorToFrame();

	// Determine interpolation type
	ERichCurveInterpMode InterpMode = RCIM_Linear;
	if (Interpolation == TEXT("Cubic"))
	{
		InterpMode = RCIM_Cubic;
	}
	else if (Interpolation == TEXT("Constant"))
	{
		InterpMode = RCIM_Constant;
	}

	// Add keyframe based on property
	if (PropertyName == TEXT("Opacity") || PropertyName == TEXT("RenderOpacity"))
	{
		double Value = Params->GetNumberField(TEXT("value"));

		UMovieSceneFloatTrack* Track = MovieScene->FindTrack<UMovieSceneFloatTrack>(BindingGuid);
		if (!Track)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("Float track not found. Add a track first using add_animation_track."));
		}

		if (Track->GetAllSections().Num() == 0)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("No section found in track"));
		}

		UMovieSceneFloatSection* Section = Cast<UMovieSceneFloatSection>(Track->GetAllSections()[0]);
		if (!Section)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("Invalid section"));
		}

		FMovieSceneFloatChannel* Channel = Section->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);
		if (Channel)
		{
			FMovieSceneFloatValue KeyValue(Value);
			KeyValue.InterpMode = InterpMode;
			Channel->AddKeys({FrameNumber}, {KeyValue});
		}
	}
	else if (PropertyName == TEXT("ColorAndOpacity"))
	{
		const TArray<TSharedPtr<FJsonValue>>* ColorArray;
		if (!Params->TryGetArrayField(TEXT("value"), ColorArray) || ColorArray->Num() < 4)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidParameter,
				TEXT("ColorAndOpacity requires [R, G, B, A] array"));
		}

		float R = (*ColorArray)[0]->AsNumber();
		float G = (*ColorArray)[1]->AsNumber();
		float B = (*ColorArray)[2]->AsNumber();
		float A = (*ColorArray)[3]->AsNumber();

		UMovieSceneColorTrack* Track = MovieScene->FindTrack<UMovieSceneColorTrack>(BindingGuid);
		if (!Track)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("Color track not found. Add a track first."));
		}

		if (Track->GetAllSections().Num() == 0)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("No section found in color track"));
		}

		UMovieSceneColorSection* Section = Cast<UMovieSceneColorSection>(Track->GetAllSections()[0]);
		if (!Section)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("Invalid color section"));
		}

		TArrayView<FMovieSceneFloatChannel*> Channels = Section->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
		if (Channels.Num() >= 4)
		{
			FMovieSceneFloatValue RValue(R); RValue.InterpMode = InterpMode;
			FMovieSceneFloatValue GValue(G); GValue.InterpMode = InterpMode;
			FMovieSceneFloatValue BValue(B); BValue.InterpMode = InterpMode;
			FMovieSceneFloatValue AValue(A); AValue.InterpMode = InterpMode;

			Channels[0]->AddKeys({FrameNumber}, {RValue});
			Channels[1]->AddKeys({FrameNumber}, {GValue});
			Channels[2]->AddKeys({FrameNumber}, {BValue});
			Channels[3]->AddKeys({FrameNumber}, {AValue});
		}
	}
	else if (PropertyName.StartsWith(TEXT("RenderTransform.")))
	{
		double Value = Params->GetNumberField(TEXT("value"));

		UMovieSceneFloatTrack* Track = FindFloatTrackByPropertyName(MovieScene, BindingGuid, PropertyName);
		if (!Track)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				FString::Printf(TEXT("Float track not found for property: %s. Add a track first."), *PropertyName));
		}

		if (Track->GetAllSections().Num() == 0)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("No section found in track"));
		}

		UMovieSceneFloatSection* Section = Cast<UMovieSceneFloatSection>(Track->GetAllSections()[0]);
		if (!Section)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::InvalidOperation,
				TEXT("Invalid section"));
		}

		FMovieSceneFloatChannel* Channel = Section->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);
		if (Channel)
		{
			FMovieSceneFloatValue KeyValue(Value);
			KeyValue.InterpMode = InterpMode;
			Channel->AddKeys({FrameNumber}, {KeyValue});
		}
	}
	else
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidParameter,
			FString::Printf(TEXT("Unsupported property for keyframe: %s"), *PropertyName));
	}

	// Save
	WidgetBP->MarkPackageDirty();

	// Response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetStringField(TEXT("animation_name"), AnimationName);
	Response->SetStringField(TEXT("target_widget"), TargetWidgetName);
	Response->SetStringField(TEXT("property_name"), PropertyName);
	Response->SetNumberField(TEXT("time"), Time);
	Response->SetNumberField(TEXT("frame"), FrameNumber.Value);
	Response->SetStringField(TEXT("interpolation"), Interpolation);

	return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGAnimationCommands::HandleGetWidgetAnimations(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Build animations array
	TArray<TSharedPtr<FJsonValue>> AnimationsArray;

	for (UWidgetAnimation* Animation : WidgetBP->Animations)
	{
		if (!Animation) continue;

		TSharedPtr<FJsonObject> AnimObj = MakeShareable(new FJsonObject());
		AnimObj->SetStringField(TEXT("name"), Animation->GetName());

		UMovieScene* MovieScene = Animation->GetMovieScene();
		if (MovieScene)
		{
			FFrameRate TickResolution = MovieScene->GetTickResolution();
			TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();

			double StartTime = TickResolution.AsSeconds(PlaybackRange.GetLowerBoundValue());
			double EndTime = TickResolution.AsSeconds(PlaybackRange.GetUpperBoundValue());
			double Length = EndTime - StartTime;

			AnimObj->SetNumberField(TEXT("length"), Length);
			AnimObj->SetBoolField(TEXT("is_looping"), false);

			TArray<TSharedPtr<FJsonValue>> TracksArray;

			for (int32 i = 0; i < MovieScene->GetPossessableCount(); ++i)
			{
				const FMovieScenePossessable& Possessable = MovieScene->GetPossessable(i);
				FGuid BindingGuid = Possessable.GetGuid();
				FString TargetName = Possessable.GetName();

				if (UMovieSceneFloatTrack* FloatTrack = MovieScene->FindTrack<UMovieSceneFloatTrack>(BindingGuid))
				{
					TSharedPtr<FJsonObject> TrackObj = MakeShareable(new FJsonObject());
					TrackObj->SetStringField(TEXT("target_widget"), TargetName);
					TrackObj->SetStringField(TEXT("property"), FloatTrack->GetTrackName().ToString());
					TrackObj->SetStringField(TEXT("type"), TEXT("Float"));

					int32 KeyframeCount = 0;
					if (FloatTrack->GetAllSections().Num() > 0)
					{
						UMovieSceneFloatSection* Section = Cast<UMovieSceneFloatSection>(FloatTrack->GetAllSections()[0]);
						if (Section)
						{
							FMovieSceneFloatChannel* Channel = Section->GetChannelProxy().GetChannel<FMovieSceneFloatChannel>(0);
							if (Channel)
							{
								KeyframeCount = Channel->GetNumKeys();
							}
						}
					}
					TrackObj->SetNumberField(TEXT("keyframe_count"), KeyframeCount);

					TracksArray.Add(MakeShareable(new FJsonValueObject(TrackObj)));
				}

				if (UMovieSceneColorTrack* ColorTrack = MovieScene->FindTrack<UMovieSceneColorTrack>(BindingGuid))
				{
					TSharedPtr<FJsonObject> TrackObj = MakeShareable(new FJsonObject());
					TrackObj->SetStringField(TEXT("target_widget"), TargetName);
					TrackObj->SetStringField(TEXT("property"), ColorTrack->GetTrackName().ToString());
					TrackObj->SetStringField(TEXT("type"), TEXT("Color"));

					int32 KeyframeCount = 0;
					if (ColorTrack->GetAllSections().Num() > 0)
					{
						UMovieSceneColorSection* Section = Cast<UMovieSceneColorSection>(ColorTrack->GetAllSections()[0]);
						if (Section)
						{
							TArrayView<FMovieSceneFloatChannel*> Channels = Section->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
							if (Channels.Num() > 0)
							{
								KeyframeCount = Channels[0]->GetNumKeys();
							}
						}
					}
					TrackObj->SetNumberField(TEXT("keyframe_count"), KeyframeCount);

					TracksArray.Add(MakeShareable(new FJsonValueObject(TrackObj)));
				}
			}

			AnimObj->SetNumberField(TEXT("track_count"), TracksArray.Num());
			AnimObj->SetArrayField(TEXT("tracks"), TracksArray);
		}
		else
		{
			AnimObj->SetNumberField(TEXT("length"), 0.0);
			AnimObj->SetBoolField(TEXT("is_looping"), false);
			AnimObj->SetNumberField(TEXT("track_count"), 0);
			AnimObj->SetArrayField(TEXT("tracks"), TArray<TSharedPtr<FJsonValue>>());
		}

		AnimationsArray.Add(MakeShareable(new FJsonValueObject(AnimObj)));
	}

	// Response
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetNumberField(TEXT("animation_count"), AnimationsArray.Num());
	Response->SetArrayField(TEXT("animations"), AnimationsArray);

	return Response;
}
