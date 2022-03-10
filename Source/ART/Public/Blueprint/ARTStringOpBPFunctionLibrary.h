#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ARTStringOpBPFunctionLibrary.generated.h"

UCLASS()
class ART_API UARTStringOpBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category="CSV Operation")
	static bool LoadStringArrayFromFile(TArray<FString>& StringArray, int32& ArraySize, FString FullFilePath, bool ExcludeEmptyLines);

	UFUNCTION(BlueprintPure, Category="AssetTools")
	static UObject* GetCDOFromAsset(UObject* InAsset);

	UFUNCTION(BlueprintPure, Category="AssetTools")
	static UObject* GetCDOFromClass(UClass* Class);

	UFUNCTION(BlueprintPure, Category="AssetTools")
	static TArray<FString> GetAllProperties(UClass* Class);

	UFUNCTION(BlueprintPure, Category="AssetTools | GameplayEffect")
	static FGameplayModifierInfo MakeGameplayModifierInfo(FGameplayAttribute Attribute, TEnumAsByte<EGameplayModOp::Type> ModifierOp, float Magnitude);
	
	UFUNCTION(BlueprintCallable, Category="AssetTools | GameplayEffect")
	static void SetDurationModifier(UPARAM(ref) FGameplayModifierInfo& Info, float Duration);
};	