/*******************************************************************************
  The MIT License (MIT)

  Copyright (c) 2015 OC3 Entertainment, Inc.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*******************************************************************************/

#include "FaceFX.h"

#define LOCTEXT_NAMESPACE "FaceFX"

UFaceFXActor::UFaceFXActor(const class FObjectInitializer& PCIP) : Super(PCIP)
{
}

const FFaceFXAnimData* UFaceFXActor::GetAnimation(const FName& AnimGroup, const FName& AnimName) const
{
	const bool IsNoneGroup = AnimGroup.IsNone();

	for(auto AnimSet : AnimationSets)
	{
		if(AnimSet && (IsNoneGroup || *AnimSet == AnimGroup))
		{
			auto Result = AnimSet->FindAnimation(AnimName);
			if(IsNoneGroup && !Result)
			{
				//animation not found and no group specified -> look further
				continue;
			}
			return Result;
		}
	}
	return nullptr;
}

#if WITH_EDITOR

void UFaceFXActor::Serialize(FArchive& Ar)
{
	if(Ar.IsSaving())
	{
		if(Ar.IsCooking())
		{
			ClearPlatformData(Ar, PlatformData);
		}

		//cleanup references to deleted assets
		for(int32 i=AnimationSets.Num()-1; i>=0; --i)
		{
			if(!AnimationSets[i])
			{
				AnimationSets.RemoveAt(i);
			}
		}
	}

	Super::Serialize(Ar);
}

#endif //WITH_EDITOR

int32 UFaceFXActor::GetAnimationCount() const
{
	int32 Result = 0;
	for(auto AnimSet : AnimationSets)
	{
		if(AnimSet)
		{
			Result += AnimSet->GetAnimationCount();
		}
	}
	return Result;
}

/** 
* Gets the details in a human readable string representation
* @param outDetails The resulting details string
*/
void UFaceFXActor::GetDetails(FString& OutDetails) const
{
	const bool bIsValid = IsValid();

	OutDetails = LOCTEXT("DetailsSetHeader", "FaceFX Set").ToString() + TEXT("\n\n");
	OutDetails += LOCTEXT("DetailsSource", "Source: ").ToString() + AssetName + TEXT("\n");
	
	if(bIsValid)
	{
		const int32 EstSize = GetData().ActorRawData.Max() + GetData().BonesRawData.Max() + GetData().Ids.GetTypeSize() * GetData().Ids.Max();
		OutDetails += LOCTEXT("DetailsSize", "Estimated Size: ").ToString() + FString::FromInt(EstSize) + TEXT(" Bytes\n");
	}
	OutDetails += LOCTEXT("DetailsAnimations", "Animation Count: ").ToString() + FString::FromInt(GetAnimationCount()) + TEXT("\n\n");


	//Animation references
	TArray<FString> AnimRefs;
	for(UFaceFXAnimSet* AnimRef : AnimationSets)
	{
		if(AnimRef)
	{
			AnimRefs.Add(AnimRef->GetName());
		}
	}

	if(AnimRefs.Num() > 0)
	{
		AnimRefs.Sort();

		OutDetails += LOCTEXT("DetailsSetAnimRefs", "Animation References").ToString() + FString::Printf(TEXT(" (%i)\n"), AnimRefs.Num());
		OutDetails += TEXT("-----------------------------\n");
		for(auto& AnimRef : AnimRefs)
		{
			OutDetails += AnimRef + TEXT("\n");
		}

		OutDetails += TEXT("\n");
	}

	//Ids
	if(bIsValid)
	{
		auto& Ids = PlatformData[0].Ids;

		TArray<FString> SortedBones;
		SortedBones.Reserve(Ids.Num());

		OutDetails += LOCTEXT("DetailsSetIds", "IDs").ToString() + FString::Printf(TEXT(" (%i)\n"), Ids.Num());
		OutDetails += TEXT("-----------------------------\n");

		for(auto& Id : Ids)
		{
			SortedBones.Add(Id.Name.GetPlainNameString());
		}

		SortedBones.Sort();
		for(auto& Id : SortedBones)
		{
			OutDetails += Id + TEXT("\n");
		}
	}
	else
	{
		OutDetails += LOCTEXT("DetailsNotLoaded", "No FaceFX data").ToString();
	}
}

void UFaceFXActor::GetAnimationGroups(TArray<FName>& OutGroups) const
{
	for(auto AnimSet : AnimationSets)
	{
		if(AnimSet)
		{
			OutGroups.AddUnique(AnimSet->GetGroup());
		}
	}
}

UFaceFXAnimSet* UFaceFXActor::GetAnimationSet(const FName& Group) const
{
	for(auto AnimSet : AnimationSets)
	{
		if(AnimSet && *AnimSet == Group)
		{
			return AnimSet;
		}
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE