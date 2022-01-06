#include "AI/Order/ARTOrderErrorTags.h"
#include "AI/Order/ARTOrder.h"

//error tags
bool FARTOrderErrorTags::IsEmpty() const
{
    return MissingTags.IsEmpty() && BlockingTags.IsEmpty() && ErrorTags.IsEmpty();
}

FString FARTOrderErrorTags::ToString() const
{
    FString s;
    s += TEXT("Missing Tags: ");
    s += MissingTags.ToString();

    s += TEXT(", Blocking Tags: ");
    s += BlockingTags.ToString();

    s += TEXT(", Error Tags: ");
    s += ErrorTags.ToString();

    return s;
}
