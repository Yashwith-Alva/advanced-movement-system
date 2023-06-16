// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if 0
	#define DRAW_MACROS
#endif

// LOG MACROS
#ifndef DRAW_MACROS
#define LOG(x, c) GEngine->AddOnScreenDebugMessage(-1, 4.f, c, x)
#else
#define LOG(x, c)
#endif

// DRAW MACROS
#ifdef DRAW_MACROS
float ScreenTime = 8.f;
#define DLOG(x, c) GEngine->AddOnScreenDebugMessage(-1, ScreenTime ? ScreenTime : -1.f, c, x)
#define BOX(x, y, c) DrawDebugBox(GetWorld(), x, y, c, !ScreenTime, ScreenTime);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !ScreenTime, ScreenTime);
#define ARROW(x, y, d, c) DrawDebugDirectionalArrow(GetWorld(), x, y, d, c, false, ScreenTime);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !ScreenTime, ScreenTime);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, c, !ScreenTime, ScreenTime);
#define BOXROTATE(x, y, r, c) DrawDebugBox(GetWorld(), x, y, r, c, false, ScreenTime);
#else
#define DLOG(x, c)
#define BOX(x, y, c)
#define LINE(x1, x2, c)
#define ARROW(x, y, d, c)
#define POINT(x, c)
#define CAPSULE(x, c)
#define BOXROTATE(x, y, r, c)
#endif

// Draw Clearance
#if 0
#define DRAW_CLEARANCE
#endif

