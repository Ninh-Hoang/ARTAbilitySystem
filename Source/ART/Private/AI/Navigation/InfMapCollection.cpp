// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Navigation/InfMapCollection.h"

#include "AI/Navigation/InfGraphInterface.h"
#include "AI/Navigation/InfMap.h"
#include "AI/Navigation/InfMapFunctionLibrary.h"
#include "AI/Navigation/InfMapInterface.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AInfMapCollection::AInfMapCollection()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	NodeGraphRef = nullptr;
	Collection.Empty();
}

// Called when the game starts or when spawned
void AInfMapCollection::BeginPlay()
{
	//map are define at construction time
	if (NodeGraphRef == nullptr || Collection.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AInfluenceMapCollection::BeginPlay() : ConstructInfluenceMapData is empty. Press the bConstructInfluenceMaps radio button."));
	}
}

void AInfMapCollection::OnConstruction(const FTransform& Transform)
{
	UInfMapFunctionLibrary::DestroyAllButFirstSpawnActor(this, StaticClass());
	Super::OnConstruction(Transform);
}

void AInfMapCollection::PostLoad()
{
	Super::PostLoad();
	
	if (NodeGraphRef == nullptr)
		NodeGraphRef = Cast<IInfGraphInterface>(NodeGraphInterface.GetObject());

	if (Collection.Num() == 0)
	{
		for (const auto& Pair : IMapInterfaceCollection)
		{
			Collection.Add(Pair.Key, Cast<IInfMapInterface>(Pair.Value.GetObject()));
		}
	}
}

IInfMapInterface* AInfMapCollection::GetMapSafe(const FGameplayTag& MapTag) const
{
	check(Collection.Num() > 0);

	if (!Collection.Contains(MapTag))
	{
		UE_LOG(LogTemp, Error, TEXT("AInfluenceMapCollection::GetMapSafe : %s does not exist"), *MapTag.ToString());
		return nullptr;
	}

	return Collection[MapTag];
}

void AInfMapCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInfMapCollection, bConstructInfluenceMaps))
	{
		bConstructInfluenceMaps = false;

		{
			//destroy all map
			TArray<AActor*> GetAllActorsResults;
			UGameplayStatics::GetAllActorsWithInterface(this, UInfMapInterface::StaticClass(), GetAllActorsResults);
			if (GetAllActorsResults.Num() > 0)
			{
				for (AActor* InfluenceMap : GetAllActorsResults)
					InfluenceMap->Destroy();
			}

			//get graph
			GetAllActorsResults.Empty();
			UGameplayStatics::GetAllActorsWithInterface(this, UInfGraphInterface::StaticClass(), GetAllActorsResults);
			if (GetAllActorsResults.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("AInfluenceMapCollection::PostEditChangeProperty() : UInfMapInterface not found "));
				return;
			}
			IInfGraphInterface* NodeGraphInterfaceObj = Cast<IInfGraphInterface>(GetAllActorsResults[0]);
			NodeGraphInterface.SetObject(NodeGraphInterfaceObj->_getUObject());
			NodeGraphRef = NodeGraphInterfaceObj;
		}

		//reset collection
		Collection.Empty();
		Collection.Reserve(ConstructInfluenceMapData.Num());
		IMapInterfaceCollection.Empty();
		IMapInterfaceCollection.Reserve(ConstructInfluenceMapData.Num());


		//for each tag in map tag array
		for (const FGameplayTag ConstructMapName : ConstructInfluenceMapData)
		{
			// check if tag is duplicate, if so, skip to next one
			if (Collection.Num() != 0 && Collection.Contains(ConstructMapName))
			{
				UE_LOG(LogTemp, Error, TEXT("AInfluenceMapCollection::PostEditChangeProperty() : Already added [%s] Influence Map"), *ConstructMapName.ToString());
				continue;
			}

			//spawn a map actor with the name of tag
			AInfMap* NewMap = GetWorld()->SpawnActor<AInfMap>(AInfMap::StaticClass());
			NewMap->Initialize(Cast<IInfCollectionInterface>(this));
			NewMap->SetFolderPath(FName(GetFolderPath().ToString() + "/Influence Map"));

#if WITH_EDITOR
			NewMap->SetActorLabel(ConstructMapName.ToString() + " Influence Map");
#endif

			IMapInterfaceCollection.Add(ConstructMapName, NewMap);
			Collection.Add(ConstructMapName, NewMap);
		}
	}
}
