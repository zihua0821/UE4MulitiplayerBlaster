// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdownText(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Server, Reliable)
	void ClientRequestServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
