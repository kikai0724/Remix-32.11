#include "pch.h"
#include "Shared.h"
#include "SDK/SDK/SlidingRuntime_classes.hpp"

INIT_POSTLOAD(UFortMovementComp_Character);

struct FCharacterNetworkMoveData
{
    void* Vft;
    uint32 NetworkMoveType;
    float TimeStamp;
};

static void (*ServerMove_PerformMovementOG)(UCharacterMovementComponent*, const FCharacterNetworkMoveData*);
static void ServerMove_PerformMovement(UCharacterMovementComponent* CharacterMovement, const FCharacterNetworkMoveData* MoveData)
{
    auto Movement = (UFortMovementComp_Character*)CharacterMovement;
    auto Pawn = Movement && MoveData && Movement->IsA(UFortMovementComp_Character::StaticClass()) ? Movement->PawnOwner : nullptr;

    if (!Pawn || !Pawn->IsA(AFortPawn::StaticClass()))
        return ServerMove_PerformMovementOG(CharacterMovement, MoveData);

    static auto GetMovementModeExtensionLogic = (UObject** (*)(AActor*, UObject**, uint32))(ImageBase + 0x2AAABA0);
    static auto MakeMovementModeExtensionContext = (FMMERuntimeContext* (*)(FMMERuntimeContext*, UObject*))(ImageBase + 0x1BCE080);
    static auto PushMovementModeExtension = (void (*)(UObject*, FMMERuntimeContext*))(ImageBase + 0x351D4B8);
    static auto GetPredictionData_Server_Character = (uint8* (*)(UCharacterMovementComponent*))(ImageBase + 0x6DD54B0);
    static auto IsClientTimeStampValid = (bool (*)(UCharacterMovementComponent*, float, uint8*, bool*))(ImageBase + 0x6DD5F04);
    static auto bEndMMEOnServerIfNoUpdateReceived = (bool*)(ImageBase + 0x12C6028C);
    auto ActiveMMEs = *(TArray<TWeakObjectPtr<UFortMovementMode_BaseExtRuntimeData>>*)((uint8*)MoveData + 0xC8);
    auto& Received = Movement->ReceivedPackMovementModeExtensionRuntimeData;
    auto& Pushed = Movement->PushedMovementModeExtensionRuntimeData;
    auto& NewlyAdded = Movement->NewlyAddedMovementModeExtensionRuntimeData;
    auto& AddedLastTick = Movement->MovementModeExtensionRuntimeAddedLastTick;
    auto& ActiveLayered = Movement->ActiveLayeredMovementModeRuntimeData;
    auto bTimeStampResetDetected = false;
    auto bNewestMove = IsClientTimeStampValid(CharacterMovement, MoveData->TimeStamp, GetPredictionData_Server_Character(CharacterMovement), &bTimeStampResetDetected);

    AddedLastTick.Clear();

    for (auto& WeakRuntime : ActiveMMEs)
    {
        auto Runtime = WeakRuntime.Get();
        if (!Runtime || Runtime->IsA(UFortMovementMode_SlidingRuntimeData::StaticClass()))
            continue;

        bool bAlreadyPushed = false;
        for (auto PushedRuntime : Pushed)
        {
            if (PushedRuntime && PushedRuntime->LogicID.MovementMode_ExtCRC_ID == Runtime->LogicID.MovementMode_ExtCRC_ID && PushedRuntime->RuntimeID == Runtime->RuntimeID)
            {
                bAlreadyPushed = true;
                break;
            }
        }

        if (bAlreadyPushed)
            continue;

        UObject* Logic = nullptr;
        GetMovementModeExtensionLogic(Pawn, &Logic, Runtime->LogicID.MovementMode_ExtCRC_ID);
        if (!Logic || !((bool (*)(UObject*))Logic->VTable[0x7C])(Logic))
            continue;

        Pushed.Add(Runtime);
        NewlyAdded.Add(Runtime);
        AddedLastTick.Add(Runtime);

        FMMERuntimeContext Context{};
        MakeMovementModeExtensionContext(&Context, Runtime);
        PushMovementModeExtension(Logic, &Context);
    }

    for (auto Runtime : Pushed)
    {
        if (!Runtime || Runtime->IsA(UFortMovementMode_SlidingRuntimeData::StaticClass()))
            continue;

        UFortMovementMode_BaseExtRuntimeData* Source = nullptr;

        for (auto Candidate : Received)
        {
            if (Candidate && Candidate->LogicID.MovementMode_ExtCRC_ID == Runtime->LogicID.MovementMode_ExtCRC_ID && Candidate->RuntimeID == Runtime->RuntimeID)
                Source = Candidate;
        }

        if (Source)
        {
            Runtime->bForceManualActivationNetworked = Source->bForceManualActivationNetworked;
            Runtime->bForceManualEndNetworked = Source->bForceManualEndNetworked;
        }
        else if (bNewestMove && *bEndMMEOnServerIfNoUpdateReceived)
            Runtime->bForceManualEnd = true;
    }

    for (int i = 0; i < Pushed.Num(); i++)
    {
        auto Runtime = Pushed[i];
        if (!Runtime || Runtime->IsA(UFortMovementMode_SlidingRuntimeData::StaticClass()))
            continue;

        bool bIsActive = false;
        for (auto& WeakRuntime : ActiveMMEs)
        {
            auto ActiveRuntime = WeakRuntime.Get();
            if (ActiveRuntime && ActiveRuntime->LogicID.MovementMode_ExtCRC_ID == Runtime->LogicID.MovementMode_ExtCRC_ID && ActiveRuntime->RuntimeID == Runtime->RuntimeID)
            {
                bIsActive = true;
                break;
            }
        }

        if (bIsActive)
            continue;

        for (int j = 0; j < ActiveLayered.Num(); j++)
        {
            if (ActiveLayered[j] == Runtime)
            {
                ActiveLayered.Remove(j);
                break;
            }
        }

        for (int j = 0; j < NewlyAdded.Num(); j++)
        {
            if (NewlyAdded[j] == Runtime)
            {
                NewlyAdded.Remove(j);
                break;
            }
        }

        Pushed.Remove(i--);
    }

    if (Pushed.Num() == 0 && Movement->ActiveMovementModeExtension)
    {
        auto ActiveRuntime = *(UObject**)((uint8*)Movement + 0x5528);
        if (ActiveRuntime)
        {
            FMMERuntimeContext Context{};
            MakeMovementModeExtensionContext(&Context, ActiveRuntime);
            Movement->ActiveMovementModeExtension->BP_OnExtensionDeactivate(Context);
        }

        ((void (*)(UFortMovementMode_BaseExtLogic*))Movement->ActiveMovementModeExtension->VTable[0x59])(Movement->ActiveMovementModeExtension);
        Movement->ActiveMovementModeExtension = nullptr;
        *(UObject**)((uint8*)Movement + 0x5528) = nullptr;
    }

    return ServerMove_PerformMovementOG(CharacterMovement, MoveData);
}

void UFortMovementComp_Character__PostLoad()
{
    Hook(ImageBase + 0x6DE05DC, ServerMove_PerformMovement, ServerMove_PerformMovementOG);
}
