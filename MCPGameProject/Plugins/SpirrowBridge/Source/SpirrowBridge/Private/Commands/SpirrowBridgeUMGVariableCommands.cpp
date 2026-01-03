#include "Commands/SpirrowBridgeUMGVariableCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_ComponentBoundEvent.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"

FSpirrowBridgeUMGVariableCommands::FSpirrowBridgeUMGVariableCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("add_widget_variable"))
	{
		return HandleAddWidgetVariable(Params);
	}
	else if (CommandName == TEXT("add_widget_array_variable"))
	{
		return HandleAddWidgetArrayVariable(Params);
	}
	else if (CommandName == TEXT("set_widget_variable_default"))
	{
		return HandleSetWidgetVariableDefault(Params);
	}
	else if (CommandName == TEXT("add_widget_function"))
	{
		return HandleAddWidgetFunction(Params);
	}
	else if (CommandName == TEXT("add_widget_event"))
	{
		return HandleAddWidgetEvent(Params);
	}
	else if (CommandName == TEXT("bind_widget_to_variable"))
	{
		return HandleBindWidgetToVariable(Params);
	}
	else if (CommandName == TEXT("bind_widget_event"))
	{
		return HandleBindWidgetEvent(Params);
	}
	else if (CommandName == TEXT("set_text_block_binding"))
	{
		return HandleSetTextBlockBinding(Params);
	}
	else if (CommandName == TEXT("bind_widget_component_event"))
	{
		return HandleBindWidgetComponentEvent(Params);
	}

	return nullptr;
}

bool FSpirrowBridgeUMGVariableCommands::SetupPinType(const FString& TypeName, FEdGraphPinType& OutPinType)
{
	OutPinType.PinCategory = NAME_None;
	OutPinType.PinSubCategory = NAME_None;
	OutPinType.PinSubCategoryObject = nullptr;
	OutPinType.ContainerType = EPinContainerType::None;
	OutPinType.bIsReference = false;

	if (TypeName == TEXT("Boolean") || TypeName == TEXT("Bool"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
	}
	else if (TypeName == TEXT("Integer") || TypeName == TEXT("Int"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Int;
	}
	else if (TypeName == TEXT("Float") || TypeName == TEXT("Double"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Real;
		OutPinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
	}
	else if (TypeName == TEXT("String"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_String;
	}
	else if (TypeName == TEXT("Name"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Name;
	}
	else if (TypeName == TEXT("Text"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Text;
	}
	else if (TypeName == TEXT("Vector"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
	}
	else if (TypeName == TEXT("Vector2D"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = TBaseStructure<FVector2D>::Get();
	}
	else if (TypeName == TEXT("Rotator"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
	}
	else if (TypeName == TEXT("Transform"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
	}
	else if (TypeName == TEXT("LinearColor") || TypeName == TEXT("Color"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
	}
	else if (TypeName == TEXT("TimerHandle"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = FTimerHandle::StaticStruct();
	}
	else if (TypeName == TEXT("Texture2D"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		OutPinType.PinSubCategoryObject = UTexture2D::StaticClass();
	}
	else if (TypeName == TEXT("Object"))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		OutPinType.PinSubCategoryObject = UObject::StaticClass();
	}
	else
	{
		UClass* FoundClass = FindObject<UClass>(nullptr, *TypeName);
		if (FoundClass)
		{
			OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
			OutPinType.PinSubCategoryObject = FoundClass;
		}
		else
		{
			UScriptStruct* FoundStruct = FindObject<UScriptStruct>(nullptr, *TypeName);
			if (FoundStruct)
			{
				OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
				OutPinType.PinSubCategoryObject = FoundStruct;
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleAddWidgetVariable(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, VariableName, VariableType;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("variable_name"), VariableName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("variable_type"), VariableType))
	{
		return Error;
	}

	// Get optional parameters
	FString Path, DefaultValue, Category;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	bool bHasDefault = Params->TryGetStringField(TEXT("default_value"), DefaultValue);
	bool bIsExposed;
	FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("is_exposed"), bIsExposed, false);
	Params->TryGetStringField(TEXT("category"), Category);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Setup Pin Type
	FEdGraphPinType PinType;
	if (!SetupPinType(VariableType, PinType))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidParameter,
			FString::Printf(TEXT("Unsupported variable type: %s"), *VariableType));
	}

	// Add variable to Blueprint
	FBlueprintEditorUtils::AddMemberVariable(WidgetBP, FName(*VariableName), PinType);

	// Find and configure the newly created variable
	FBPVariableDescription* NewVar = nullptr;
	for (FBPVariableDescription& Variable : WidgetBP->NewVariables)
	{
		if (Variable.VarName == FName(*VariableName))
		{
			NewVar = &Variable;
			break;
		}
	}

	if (NewVar)
	{
		if (bIsExposed)
		{
			NewVar->PropertyFlags |= CPF_Edit | CPF_BlueprintVisible;
		}
		if (!Category.IsEmpty())
		{
			NewVar->Category = FText::FromString(Category);
		}
		if (bHasDefault)
		{
			NewVar->DefaultValue = DefaultValue;
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("variable_name"), VariableName);
	ResultObj->SetStringField(TEXT("variable_type"), VariableType);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleSetWidgetVariableDefault(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, VariableName, DefaultValue;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("variable_name"), VariableName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("default_value"), DefaultValue))
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

	// Find variable
	FBPVariableDescription* Variable = nullptr;
	for (FBPVariableDescription& Var : WidgetBP->NewVariables)
	{
		if (Var.VarName == FName(*VariableName))
		{
			Variable = &Var;
			break;
		}
	}

	if (!Variable)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found in Widget Blueprint"), *VariableName));
	}

	Variable->DefaultValue = DefaultValue;

	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("variable_name"), VariableName);
	ResultObj->SetStringField(TEXT("default_value"), DefaultValue);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleAddWidgetFunction(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, FunctionName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("function_name"), FunctionName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	bool bIsPure;
	FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("is_pure"), bIsPure, false);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Check if function already exists
	for (UEdGraph* Graph : WidgetBP->FunctionGraphs)
	{
		if (Graph->GetFName() == FName(*FunctionName))
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::AssetAlreadyExists,
				FString::Printf(TEXT("Function '%s' already exists"), *FunctionName));
		}
	}

	// Create function graph
	UEdGraph* FuncGraph = FBlueprintEditorUtils::CreateNewGraph(
		WidgetBP,
		FName(*FunctionName),
		UEdGraph::StaticClass(),
		UEdGraphSchema_K2::StaticClass()
	);

	if (!FuncGraph)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::OperationFailed,
			TEXT("Failed to create function graph"));
	}

	FBlueprintEditorUtils::AddFunctionGraph<UClass>(WidgetBP, FuncGraph, false, nullptr);

	// Find the existing Entry node
	UK2Node_FunctionEntry* EntryNode = nullptr;
	for (UEdGraphNode* Node : FuncGraph->Nodes)
	{
		EntryNode = Cast<UK2Node_FunctionEntry>(Node);
		if (EntryNode) break;
	}

	if (!EntryNode)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::NodeNotFound,
			TEXT("Failed to find function entry node"));
	}

	// Add input parameters
	const TArray<TSharedPtr<FJsonValue>>* InputsArray;
	if (Params->TryGetArrayField(TEXT("inputs"), InputsArray))
	{
		for (const TSharedPtr<FJsonValue>& InputValue : *InputsArray)
		{
			const TSharedPtr<FJsonObject>* InputObj;
			if (InputValue->TryGetObject(InputObj))
			{
				FString ParamName, ParamType;
				(*InputObj)->TryGetStringField(TEXT("name"), ParamName);
				(*InputObj)->TryGetStringField(TEXT("type"), ParamType);

				if (!ParamName.IsEmpty() && !ParamType.IsEmpty())
				{
					FEdGraphPinType PinType;
					if (SetupPinType(ParamType, PinType))
					{
						EntryNode->CreateUserDefinedPin(FName(*ParamName), PinType, EGPD_Output);
					}
				}
			}
		}
	}

	// Add output parameters if provided
	const TArray<TSharedPtr<FJsonValue>>* OutputsArray;
	if (Params->TryGetArrayField(TEXT("outputs"), OutputsArray) && OutputsArray->Num() > 0)
	{
		UK2Node_FunctionResult* ResultNode = nullptr;
		for (UEdGraphNode* Node : FuncGraph->Nodes)
		{
			ResultNode = Cast<UK2Node_FunctionResult>(Node);
			if (ResultNode) break;
		}

		if (!ResultNode)
		{
			ResultNode = NewObject<UK2Node_FunctionResult>(FuncGraph);
			FuncGraph->AddNode(ResultNode, false, false);
			ResultNode->NodePosX = 400;
			ResultNode->NodePosY = 0;
			ResultNode->AllocateDefaultPins();
		}

		for (const TSharedPtr<FJsonValue>& OutputValue : *OutputsArray)
		{
			const TSharedPtr<FJsonObject>* OutputObj;
			if (OutputValue->TryGetObject(OutputObj))
			{
				FString ParamName, ParamType;
				(*OutputObj)->TryGetStringField(TEXT("name"), ParamName);
				(*OutputObj)->TryGetStringField(TEXT("type"), ParamType);

				if (!ParamName.IsEmpty() && !ParamType.IsEmpty())
				{
					FEdGraphPinType PinType;
					if (SetupPinType(ParamType, PinType))
					{
						ResultNode->CreateUserDefinedPin(FName(*ParamName), PinType, EGPD_Input);
					}
				}
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("function_name"), FunctionName);
	ResultObj->SetStringField(TEXT("graph_id"), FuncGraph->GraphGuid.ToString());
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleAddWidgetEvent(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, EventName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("event_name"), EventName))
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

	// Find Event Graph
	UEdGraph* EventGraph = nullptr;
	for (UEdGraph* Graph : WidgetBP->UbergraphPages)
	{
		if (Graph->GetFName() == TEXT("EventGraph"))
		{
			EventGraph = Graph;
			break;
		}
	}

	if (!EventGraph)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::GraphNotFound,
			TEXT("Event Graph not found"));
	}

	// Create Custom Event node
	UK2Node_Event* EventNode = NewObject<UK2Node_Event>(EventGraph);
	EventGraph->AddNode(EventNode, false, false);
	EventNode->CustomFunctionName = FName(*EventName);
	EventNode->bIsEditable = true;
	EventNode->NodePosX = 0;
	EventNode->NodePosY = 0;
	EventNode->AllocateDefaultPins();

	// Add input parameters
	const TArray<TSharedPtr<FJsonValue>>* InputsArray;
	if (Params->TryGetArrayField(TEXT("inputs"), InputsArray))
	{
		for (const TSharedPtr<FJsonValue>& InputValue : *InputsArray)
		{
			const TSharedPtr<FJsonObject>* InputObj;
			if (InputValue->TryGetObject(InputObj))
			{
				FString ParamName, ParamType;
				(*InputObj)->TryGetStringField(TEXT("name"), ParamName);
				(*InputObj)->TryGetStringField(TEXT("type"), ParamType);

				FEdGraphPinType PinType;
				if (SetupPinType(ParamType, PinType))
				{
					EventNode->CreateUserDefinedPin(FName(*ParamName), PinType, EGPD_Output);
				}
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("event_name"), EventName);
	ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleBindWidgetToVariable(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, ElementName, PropertyName, VariableName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("element_name"), ElementName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("property_name"), PropertyName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("variable_name"), VariableName))
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

	// Find widget element
	UWidget* Element = WidgetBP->WidgetTree->FindWidget(FName(*ElementName));
	if (!Element)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetElementNotFound,
			FString::Printf(TEXT("Widget element '%s' not found"), *ElementName));
	}

	// Generate binding function name
	FString FunctionName = FString::Printf(TEXT("Get%s%s"), *VariableName, *PropertyName);

	// Check if binding function already exists
	UEdGraph* FuncGraph = nullptr;
	for (UEdGraph* Graph : WidgetBP->FunctionGraphs)
	{
		if (Graph->GetFName() == FName(*FunctionName))
		{
			FuncGraph = Graph;
			break;
		}
	}

	// Create binding function if it doesn't exist
	if (!FuncGraph)
	{
		// Find the variable
		FBPVariableDescription* Variable = nullptr;
		for (FBPVariableDescription& Var : WidgetBP->NewVariables)
		{
			if (Var.VarName == FName(*VariableName))
			{
				Variable = &Var;
				break;
			}
		}

		if (!Variable)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::VariableNotFound,
				FString::Printf(TEXT("Variable '%s' not found in Widget Blueprint"), *VariableName));
		}

		FuncGraph = FBlueprintEditorUtils::CreateNewGraph(
			WidgetBP,
			FName(*FunctionName),
			UEdGraph::StaticClass(),
			UEdGraphSchema_K2::StaticClass()
		);

		if (!FuncGraph)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::OperationFailed,
				TEXT("Failed to create binding function"));
		}

		FBlueprintEditorUtils::AddFunctionGraph<UClass>(WidgetBP, FuncGraph, false, nullptr);

		// Find Entry node
		UK2Node_FunctionEntry* EntryNode = nullptr;
		for (UEdGraphNode* Node : FuncGraph->Nodes)
		{
			EntryNode = Cast<UK2Node_FunctionEntry>(Node);
			if (EntryNode) break;
		}

		if (!EntryNode)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::NodeNotFound,
				TEXT("Failed to find function entry node"));
		}

		// Create Variable Get node
		UK2Node_VariableGet* GetVarNode = NewObject<UK2Node_VariableGet>(FuncGraph);
		GetVarNode->VariableReference.SetSelfMember(FName(*VariableName));
		FuncGraph->AddNode(GetVarNode, false, false);
		GetVarNode->NodePosX = 200;
		GetVarNode->NodePosY = 0;
		GetVarNode->AllocateDefaultPins();

		// Create Result node
		UK2Node_FunctionResult* ResultNode = nullptr;
		for (UEdGraphNode* Node : FuncGraph->Nodes)
		{
			ResultNode = Cast<UK2Node_FunctionResult>(Node);
			if (ResultNode) break;
		}

		if (!ResultNode)
		{
			ResultNode = NewObject<UK2Node_FunctionResult>(FuncGraph);
			FuncGraph->AddNode(ResultNode, false, false);
			ResultNode->NodePosX = 400;
			ResultNode->NodePosY = 0;
			ResultNode->AllocateDefaultPins();

			FEdGraphPinType ReturnPinType = Variable->VarType;
			ResultNode->CreateUserDefinedPin(TEXT("ReturnValue"), ReturnPinType, EGPD_Input);
		}

		// Connect nodes
		UEdGraphPin* GetVarOutputPin = GetVarNode->FindPin(FName(*VariableName), EGPD_Output);
		UEdGraphPin* ResultInputPin = ResultNode->FindPin(TEXT("ReturnValue"), EGPD_Input);

		if (GetVarOutputPin && ResultInputPin)
		{
			GetVarOutputPin->MakeLinkTo(ResultInputPin);
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("binding_function"), FunctionName);
	ResultObj->SetStringField(TEXT("note"), TEXT("Binding function created. Manual binding in UMG editor may be required."));
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleBindWidgetEvent(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString BlueprintName, WidgetName, EventName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("event_name"), EventName))
	{
		return Error;
	}

	// Load the Widget Blueprint
	const FString BlueprintPath = FString::Printf(TEXT("/Game/Widgets/%s.%s"), *BlueprintName, *BlueprintName);
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
	if (!WidgetBlueprint)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::BlueprintNotFound,
			FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
	}

	// Find event graph
	UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(WidgetBlueprint);
	if (!EventGraph)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::GraphNotFound,
			TEXT("Failed to find event graph"));
	}

	// Find widget
	UWidget* Widget = WidgetBlueprint->WidgetTree->FindWidget(*WidgetName);
	if (!Widget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetElementNotFound,
			FString::Printf(TEXT("Failed to find widget: %s"), *WidgetName));
	}

	// Find or create event node
	UK2Node_Event* EventNode = nullptr;
	TArray<UK2Node_Event*> AllEventNodes;
	FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(WidgetBlueprint, AllEventNodes);

	for (UK2Node_Event* Node : AllEventNodes)
	{
		if (Node->CustomFunctionName == FName(*EventName) && Node->EventReference.GetMemberParentClass() == Widget->GetClass())
		{
			EventNode = Node;
			break;
		}
	}

	if (!EventNode)
	{
		float MaxHeight = 0.0f;
		for (UEdGraphNode* Node : EventGraph->Nodes)
		{
			MaxHeight = FMath::Max(MaxHeight, Node->NodePosY);
		}

		FKismetEditorUtilities::CreateNewBoundEventForClass(
			Widget->GetClass(),
			FName(*EventName),
			WidgetBlueprint,
			nullptr
		);

		TArray<UK2Node_Event*> UpdatedEventNodes;
		FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(WidgetBlueprint, UpdatedEventNodes);

		for (UK2Node_Event* Node : UpdatedEventNodes)
		{
			if (Node->CustomFunctionName == FName(*EventName) && Node->EventReference.GetMemberParentClass() == Widget->GetClass())
			{
				EventNode = Node;
				EventNode->NodePosX = 200;
				EventNode->NodePosY = MaxHeight + 200;
				break;
			}
		}
	}

	if (!EventNode)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::NodeCreationFailed,
			TEXT("Failed to create event node"));
	}

	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(BlueprintPath, false);

	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("event_name"), EventName);
	return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleSetTextBlockBinding(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString BlueprintName, WidgetName, BindingName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("binding_name"), BindingName))
	{
		return Error;
	}

	// Load the Widget Blueprint
	const FString BlueprintPath = FString::Printf(TEXT("/Game/Widgets/%s.%s"), *BlueprintName, *BlueprintName);
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
	if (!WidgetBlueprint)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::BlueprintNotFound,
			FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
	}

	// Add binding variable
	FBlueprintEditorUtils::AddMemberVariable(
		WidgetBlueprint,
		FName(*BindingName),
		FEdGraphPinType(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType())
	);

	// Find the TextBlock widget
	UTextBlock* TextBlock = Cast<UTextBlock>(WidgetBlueprint->WidgetTree->FindWidget(FName(*WidgetName)));
	if (!TextBlock)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetElementNotFound,
			FString::Printf(TEXT("Failed to find TextBlock widget: %s"), *WidgetName));
	}

	// Create binding function
	const FString FunctionName = FString::Printf(TEXT("Get%s"), *BindingName);
	UEdGraph* FuncGraph = FBlueprintEditorUtils::CreateNewGraph(
		WidgetBlueprint,
		FName(*FunctionName),
		UEdGraph::StaticClass(),
		UEdGraphSchema_K2::StaticClass()
	);

	if (FuncGraph)
	{
		FBlueprintEditorUtils::AddFunctionGraph<UClass>(WidgetBlueprint, FuncGraph, false, nullptr);

		UK2Node_FunctionEntry* EntryNode = NewObject<UK2Node_FunctionEntry>(FuncGraph);
		FuncGraph->AddNode(EntryNode, false, false);
		EntryNode->NodePosX = 0;
		EntryNode->NodePosY = 0;
		EntryNode->FunctionReference.SetExternalMember(FName(*FunctionName), WidgetBlueprint->GeneratedClass);
		EntryNode->AllocateDefaultPins();

		UK2Node_VariableGet* GetVarNode = NewObject<UK2Node_VariableGet>(FuncGraph);
		GetVarNode->VariableReference.SetSelfMember(FName(*BindingName));
		FuncGraph->AddNode(GetVarNode, false, false);
		GetVarNode->NodePosX = 200;
		GetVarNode->NodePosY = 0;
		GetVarNode->AllocateDefaultPins();

		UEdGraphPin* EntryThenPin = EntryNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* GetVarOutPin = GetVarNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		if (EntryThenPin && GetVarOutPin)
		{
			EntryThenPin->MakeLinkTo(GetVarOutPin);
		}
	}

	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(BlueprintPath, false);

	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("binding_name"), BindingName);
	return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleAddWidgetArrayVariable(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, VariableName, ElementType;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("variable_name"), VariableName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("element_type"), ElementType))
	{
		return Error;
	}

	// Get optional parameters
	FString Path, Category;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	Params->TryGetStringField(TEXT("category"), Category);
	bool bIsExposed;
	FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("is_exposed"), bIsExposed, false);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Check if variable already exists
	for (const FBPVariableDescription& Var : WidgetBP->NewVariables)
	{
		if (Var.VarName == *VariableName)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(
				ESpirrowErrorCode::AssetAlreadyExists,
				FString::Printf(TEXT("Variable '%s' already exists"), *VariableName));
		}
	}

	// Set up the element pin type
	FEdGraphPinType ElementPinType;
	if (!SetupPinType(ElementType, ElementPinType))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidParameter,
			FString::Printf(TEXT("Unsupported element type: %s"), *ElementType));
	}

	// Create array pin type
	FEdGraphPinType ArrayPinType;
	ArrayPinType.ContainerType = EPinContainerType::Array;
	ArrayPinType.PinCategory = ElementPinType.PinCategory;
	ArrayPinType.PinSubCategory = ElementPinType.PinSubCategory;
	ArrayPinType.PinSubCategoryObject = ElementPinType.PinSubCategoryObject;
	ArrayPinType.PinSubCategoryMemberReference = ElementPinType.PinSubCategoryMemberReference;

	// Create the variable description
	FBPVariableDescription NewVar;
	NewVar.VarName = *VariableName;
	NewVar.VarGuid = FGuid::NewGuid();
	NewVar.VarType = ArrayPinType;
	NewVar.PropertyFlags = CPF_Edit | CPF_BlueprintVisible | CPF_DisableEditOnInstance;

	if (bIsExposed)
	{
		NewVar.PropertyFlags |= CPF_ExposeOnSpawn;
	}

	if (!Category.IsEmpty())
	{
		NewVar.Category = FText::FromString(Category);
	}

	WidgetBP->NewVariables.Add(NewVar);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);
	WidgetBP->MarkPackageDirty();

	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetStringField(TEXT("variable_name"), VariableName);
	Response->SetStringField(TEXT("variable_type"), FString::Printf(TEXT("TArray<%s>"), *ElementType));
	Response->SetStringField(TEXT("element_type"), ElementType);
	Response->SetBoolField(TEXT("is_exposed"), bIsExposed);

	return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGVariableCommands::HandleBindWidgetComponentEvent(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, ComponentName, EventType;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("component_name"), ComponentName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("event_type"), EventType))
	{
		return Error;
	}

	// Get optional parameters
	FString Path, FunctionName;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
	{
		FunctionName = ComponentName + TEXT("_") + EventType;
	}
	bool bCreateFunction;
	FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("create_function"), bCreateFunction, true);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	// Find widget component
	UWidget* WidgetComponent = WidgetBP->WidgetTree->FindWidget(FName(*ComponentName));
	if (!WidgetComponent)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetElementNotFound,
			FString::Printf(TEXT("Widget component '%s' not found"), *ComponentName));
	}

	// Validate event type for component
	UButton* ButtonWidget = Cast<UButton>(WidgetComponent);
	USlider* SliderWidget = Cast<USlider>(WidgetComponent);
	UCheckBox* CheckBoxWidget = Cast<UCheckBox>(WidgetComponent);

	bool bValidEvent = false;
	FName EventPropertyName;

	if (ButtonWidget)
	{
		if (EventType == TEXT("OnClicked") || EventType == TEXT("OnPressed") || EventType == TEXT("OnReleased") ||
			EventType == TEXT("OnHovered") || EventType == TEXT("OnUnhovered"))
		{
			EventPropertyName = FName(*EventType);
			bValidEvent = true;
		}
	}
	else if (SliderWidget || CheckBoxWidget)
	{
		if (EventType == TEXT("OnValueChanged"))
		{
			EventPropertyName = FName("OnValueChanged");
			bValidEvent = true;
		}
	}

	if (!bValidEvent)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidParameter,
			FString::Printf(TEXT("Invalid event type '%s' for component type"), *EventType));
	}

	// Find Event Graph
	UEdGraph* EventGraph = nullptr;
	for (UEdGraph* Graph : WidgetBP->UbergraphPages)
	{
		if (Graph->GetFName() == TEXT("EventGraph"))
		{
			EventGraph = Graph;
			break;
		}
	}

	if (!EventGraph)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::GraphNotFound,
			TEXT("Event Graph not found"));
	}

	// Create the function if requested
	UEdGraph* FuncGraph = nullptr;
	if (bCreateFunction)
	{
		for (UEdGraph* Graph : WidgetBP->FunctionGraphs)
		{
			if (Graph->GetFName() == FName(*FunctionName))
			{
				FuncGraph = Graph;
				break;
			}
		}

		if (!FuncGraph)
		{
			FuncGraph = FBlueprintEditorUtils::CreateNewGraph(
				WidgetBP,
				FName(*FunctionName),
				UEdGraph::StaticClass(),
				UEdGraphSchema_K2::StaticClass()
			);

			if (FuncGraph)
			{
				FBlueprintEditorUtils::AddFunctionGraph<UClass>(WidgetBP, FuncGraph, false, nullptr);
			}
		}
	}

	// Ensure widget is bound as a variable
	FObjectProperty* WidgetProperty = FindFProperty<FObjectProperty>(WidgetBP->SkeletonGeneratedClass, FName(*ComponentName));
	if (!WidgetProperty)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::InvalidOperation,
			FString::Printf(TEXT("Widget '%s' must be bound as a variable first."), *ComponentName));
	}

	// Create component bound event
	FKismetEditorUtilities::CreateNewBoundEventForComponent(
		WidgetComponent,
		EventPropertyName,
		WidgetBP,
		WidgetProperty
	);

	// Find the created event node
	UEdGraphNode* EventNode = nullptr;
	for (UEdGraphNode* Node : EventGraph->Nodes)
	{
		if (UK2Node_ComponentBoundEvent* BoundEventNode = Cast<UK2Node_ComponentBoundEvent>(Node))
		{
			if (BoundEventNode->DelegatePropertyName == EventPropertyName &&
				BoundEventNode->ComponentPropertyName == FName(*ComponentName))
			{
				EventNode = BoundEventNode;
				break;
			}
		}
	}

	if (!EventNode)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::NodeNotFound,
			TEXT("Failed to find created event node"));
	}

	// Connect event to function
	if (FuncGraph && EventNode)
	{
		UEdGraphPin* EventExecPin = nullptr;
		for (UEdGraphPin* Pin : EventNode->Pins)
		{
			if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec && Pin->Direction == EGPD_Output)
			{
				EventExecPin = Pin;
				break;
			}
		}

		if (EventExecPin)
		{
			UK2Node_CallFunction* CallFuncNode = NewObject<UK2Node_CallFunction>(EventGraph);
			UFunction* TargetFunction = WidgetBP->GeneratedClass->FindFunctionByName(FName(*FunctionName));
			if (TargetFunction)
			{
				CallFuncNode->SetFromFunction(TargetFunction);
				EventGraph->AddNode(CallFuncNode, false, false);
				CallFuncNode->NodePosX = EventNode->NodePosX + 300;
				CallFuncNode->NodePosY = EventNode->NodePosY;
				CallFuncNode->AllocateDefaultPins();

				UEdGraphPin* FuncExecPin = CallFuncNode->FindPin(UEdGraphSchema_K2::PN_Execute);
				if (FuncExecPin)
				{
					EventExecPin->MakeLinkTo(FuncExecPin);
				}
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("component_name"), ComponentName);
	ResultObj->SetStringField(TEXT("event_type"), EventType);
	ResultObj->SetStringField(TEXT("function_name"), FunctionName);
	if (EventNode)
	{
		ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
	}
	return ResultObj;
}
