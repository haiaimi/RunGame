// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/FireCameraShake.h"




UFireCameraShake::UFireCameraShake()
{
	OscillationDuration = 0.2f;
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.15f;
	FROscillator RotOscillator;
	RotOscillator.Pitch.Amplitude = 0.4f;
	RotOscillator.Pitch.Frequency = 20.f;
	RotOscillator.Roll.Amplitude = 0.4f;
	RotOscillator.Roll.Frequency = 30.f;

	RotOscillation = RotOscillator;
	FOVOscillation.Amplitude = 0.5f;
	FOVOscillation.Frequency = 20.f;
	FOVOscillation.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;
}
