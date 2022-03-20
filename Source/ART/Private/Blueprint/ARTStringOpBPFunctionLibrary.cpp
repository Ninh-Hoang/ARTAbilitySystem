#include "Blueprint/ARTStringOpBPFunctionLibrary.h"

bool UARTStringOpBPFunctionLibrary::LoadStringArrayFromFile(TArray<FString>& StringArray, int32& ArraySize, FString FullFilePath, bool ExcludeEmptyLines)
{
	ArraySize = 0;
	
	if(FullFilePath == "" || FullFilePath == " ") return false;
	
	//Empty any previous contents!
	StringArray.Empty();
	
	TArray<FString> FileArray;
	 
	if( ! FFileHelper::LoadANSITextFileToStrings(*FullFilePath, NULL, FileArray))
	{
		return false;
	}

	if(ExcludeEmptyLines)
	{
		for(const FString& Each : FileArray )
		{
			if(Each == "") continue;
			//~~~~~~~~~~~~~
			
			//check for any non whitespace
			bool FoundNonWhiteSpace = false;
			for(int32 v = 0; v < Each.Len(); v++)
			{
				if(Each[v] != ' ' && Each[v] != '\n')
				{
					FoundNonWhiteSpace = true;
					break;
				}
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			}
			
			if(FoundNonWhiteSpace)
			{
				StringArray.Add(Each);
			}
		}
	}
	else
	{
		StringArray.Append(FileArray);
	}
	
	ArraySize = StringArray.Num();
	return true; 
}

UObject* UARTStringOpBPFunctionLibrary::GetCDOFromAsset(UObject* InAsset)
{
	UBlueprint* _Blueprint = Cast<UBlueprint>(InAsset);
	if(!_Blueprint) return nullptr;

	UObject* _CDO = _Blueprint->GeneratedClass->GetDefaultObject();

	return(_CDO);
}

UObject* UARTStringOpBPFunctionLibrary::GetCDOFromClass(UClass* Class)
{
	if(!Class) return nullptr;

	UObject* _CDO = Class->GetDefaultObject();

	return(_CDO);
}

TArray<FString> UARTStringOpBPFunctionLibrary::GetAllProperties(UClass* Class)
{
	TArray<FString> Ret;
	if(Class == nullptr) return Ret;

	for(TFieldIterator<FProperty> It(Class); It; ++It)
	{
		FProperty* Property = *It;
		if(Property->HasAnyPropertyFlags(EPropertyFlags::CPF_Edit))
		{
			Ret.Add(Property->GetName());
		}
	}

	return Ret;
}

FGameplayModifierInfo UARTStringOpBPFunctionLibrary::MakeGameplayModifierInfo(FGameplayAttribute Attribute,
	TEnumAsByte<EGameplayModOp::Type> ModifierOp, float Magnitude)
{
	FGameplayModifierInfo Info;
	Info.Attribute = Attribute;
	Info.ModifierOp = ModifierOp;
	Info.ModifierMagnitude.ScalableFloatMagnitude.Value = Magnitude;

	return Info;
}

void UARTStringOpBPFunctionLibrary::SetDurationModifier(FGameplayModifierInfo& Info, float Duration)
{
	//Info.ModifierMagnitude.ScalableFloatMagnitude.Value = Duration;
}
