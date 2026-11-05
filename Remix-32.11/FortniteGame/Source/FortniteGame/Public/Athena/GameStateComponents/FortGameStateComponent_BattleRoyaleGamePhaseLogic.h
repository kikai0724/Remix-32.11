#pragma once
#include "pch.h"

extern TArray<FVector> SafeZoneLocations;
extern bool bAdminSafeZonePaused;

void SetGamePhase(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic, EAthenaGamePhase GamePhase);
void SetGamePhaseStep(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic, EAthenaGamePhaseStep GamePhaseStep);
void StartAircraftPhase(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic);
void InitializeSafeZoneLocations(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic);
void SetAdminSafeZonePaused(bool bPaused);
