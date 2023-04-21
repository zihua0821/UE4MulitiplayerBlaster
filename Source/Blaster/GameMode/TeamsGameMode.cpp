#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	//开启团队游戏
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//玩家登入时，动态为两个队伍分配玩家
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	//玩家登出时将玩家移出对应队伍
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	//如果伤害来源是自己，则将伤害降低一半
	if (VictimPState == AttackerPState)
	{
		return BaseDamage * 0.5f;
	}
	//如果伤害来源是队友，则阻止伤害
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	//玩家淘汰时为对立队伍增加分数
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	//比赛开始时平衡为两队分配玩家
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState);
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
