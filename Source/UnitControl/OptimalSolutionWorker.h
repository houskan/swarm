// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class AMasterController;
class UFormation;
class ARobotPawn;
/**
 * 
 */
class UNITCONTROL_API FOptimalSolutionWorker : public FRunnable
{

	static FOptimalSolutionWorker* Runnable;

	FRunnableThread* Thread;

	AMasterController* MyPlayerController;

	UFormation* Formation;

	TArray<TWeakObjectPtr<ARobotPawn>> Robots;

	TArray<FVector>* Solution;

public:

	bool IsFinished() const
	{
		if (!Solution) return false;
		return Solution->Num() == Robots.Num() && Solution->Num() != 0;
	}

	FOptimalSolutionWorker(TArray<FVector> & InSolution, TArray<TWeakObjectPtr<ARobotPawn>>& InRobots, UFormation* InFormation, AMasterController* InPC);
	~FOptimalSolutionWorker();

	// Begin FRunnable interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	//End FRunnable interface

	void EnsureCompletion();

	static FOptimalSolutionWorker* JoyInit(TArray<FVector> & InSolution, TArray<TWeakObjectPtr<ARobotPawn>>& InRobots, UFormation* InFormation, AMasterController* InPC);

	static void Shutdown();

	static bool IsThreadFinished();

};
