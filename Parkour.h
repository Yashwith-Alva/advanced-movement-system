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
#define LOG
#endif

// DRAW MACROS
#ifdef DRAW_MACROS
float MacroDuration = 8.f;
#define DLOG(x, c) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, c, x)
#define BOX(x, y, c) DrawDebugBox(GetWorld(), x, y, c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define ARROW(x, y, d, c) DrawDebugDirectionalArrow(GetWorld(), x, y, d, c, false, MacroDuration);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, c, !MacroDuration, MacroDuration);
#define BOXROTATE(x, y, r, c) DrawDebugBox(GetWorld(), x, y, r, c, false, MacroDuration);
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

