// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "LukeGameAbilityTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMyTwoParamDelegate, float, FirstParamName, int32, SecondParamName);

/**
 * 
 */
UCLASS()
class LUKEGAME_API ULukeGameAbilityTask : public UAbilityTask
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "ExecuteMyTask", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static ULukeGameAbilityTask* CreateMyTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, float examplevariable);

	UPROPERTY(BlueprintAssignable)
		FMyTwoParamDelegate OnCalled;

	virtual void Activate() override;


};
