// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitControl.h"
#include "OptimalSolutionWorker.h"
#include "gurobi_c++.h"
#include <string>
#include "Formation.h"
#include "RobotPawn.h"
#include "MasterController.h"

FOptimalSolutionWorker* FOptimalSolutionWorker::Runnable = NULL;

FOptimalSolutionWorker::FOptimalSolutionWorker(TArray<FVector>& InSolution, TArray<TWeakObjectPtr<ARobotPawn>>& InRobots, UFormation * InFormation, AMasterController * InPC)
	: MyPlayerController(InPC)
	, Robots(InRobots)
	, Formation(InFormation)
{
	Solution = &InSolution;
	Thread = FRunnableThread::Create(this, TEXT("FOptimalSolutionWorker"), 0, TPri_BelowNormal);
}

FOptimalSolutionWorker::~FOptimalSolutionWorker()
{
	delete Thread;
	Thread = NULL;
}

bool FOptimalSolutionWorker::Init()
{
	Solution->Empty(Robots.Num());
	UE_LOG(LogTemp, Warning, TEXT("Init done"));
	return true;
}

uint32 FOptimalSolutionWorker::Run()
{
	FPlatformProcess::Sleep(0.03f);

	if (!Formation)
	{
		return -1;
	}
		

	int32 Num = Robots.Num();
	int32 NumOfFullRows = Num / Formation->GetColumnCount();
	int32 NumOfRobotsInLastRow = Num % Formation->GetColumnCount();

	TArray<FVector> FormationLocations = Formation->GetFirstXRows(NumOfFullRows);
	int32 Middle = (Formation->GetColumnCount() - 1) / 2;
	FVector2D Center = FVector2D(NumOfFullRows, Middle);
	//int32 MaxIndex = NumOfRobotsInLastRow % 2 == 0 ? (NumOfRobotsInLastRow / 2) - 1 : NumOfRobotsInLastRow / 2;
	for (int32 I = -(NumOfRobotsInLastRow - 1) / 2; I <= NumOfRobotsInLastRow / 2; I++)
	{
		FormationLocations.Add(Formation->GetWorldLocation(Center + FVector2D(0.0f, I)));
	}
	
	if (FormationLocations.Num() != Num)
	{
		UE_LOG(LogTemp, Warning, TEXT("FormationLocs.Num %d != Robots.Num %d"), FormationLocations.Num(), Num);
	}

	double* Costs;
	Costs = new double[Num * Num];
	
	for (int32 I = 0 ; I < Num; I++)
	{
		FVector RobotLocation = Robots[I]->GetActorLocation();
		for (int32 J = 0; J < Num; J++)
		{
			Costs[I * Num + J] = FVector::DistSquaredXY(RobotLocation, FormationLocations[J]);
		}
	}
	GRBEnv Env = GRBEnv();
	Env.set(GRB_IntParam_OutputFlag, 0);
	GRBModel Model = GRBModel(Env);

	GRBVar* Vars = new GRBVar[Num * Num];
	for (int32 I = 0; I < Num*Num; I++)
	{
		GRBVar X = Model.addVar(0.0, 1.0, 1.0, GRB_CONTINUOUS, "X" + std::to_string(I));
		Vars[I] = X;
	}
	for (int32 I = 0; I < Num; I++)
	{
		GRBLinExpr Xij = 0.0;
		GRBLinExpr Xji = 0.0;
		for (int32 J = 0; J < Num; J++)
		{
			Xij += Vars[I * Num + J];
			Xji += Vars[J * Num + I];
		}
		Model.addConstr(Xij == 1.0);
		Model.addConstr(Xji == 1.0);
	}
	GRBLinExpr Objective = 0.0;
	Objective.addTerms(Costs, Vars, Num * Num);
	Model.setObjective(Objective, GRB_MINIMIZE);
	Model.optimize();
	for (int32 I = 0; I < Num; I++)
	{
		int32 SolutionIndex = -1;
		for (int32 J = 0; J < Num; J++)
		{
			float VarX = Vars[I * Num + J].get(GRB_DoubleAttr_X);
			if (FMath::Abs(1.0f - VarX) < 0.001f)
			{
				SolutionIndex = J;
				break;
			}
		}
		if (SolutionIndex == -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("error in model"));
			return -1;
		}
		Solution->Add(FormationLocations[SolutionIndex]);
	}
	delete[]Costs;
	delete[]Vars;
	return 0;
}

void FOptimalSolutionWorker::Stop()
{
}

void FOptimalSolutionWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

FOptimalSolutionWorker * FOptimalSolutionWorker::JoyInit(TArray<FVector> & InSolution, TArray<TWeakObjectPtr<ARobotPawn>>& InRobots, UFormation* InFormation, AMasterController* InPC)
{
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FOptimalSolutionWorker(InSolution, InRobots, InFormation, InPC);
	}
	return Runnable;
}

void FOptimalSolutionWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FOptimalSolutionWorker::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return false;
}
