#include "AI/Order/ARTOrderPreviewData.h"

TSubclassOf<AARTOrderPreview> FARTOrderPreviewData::GetOrderPreviewClass()
{
	return OrderPreviewClass;
}

EARTOrderPreviewSnapMode FARTOrderPreviewData::GetSnapMode()
{
	return SnapMode;
}
