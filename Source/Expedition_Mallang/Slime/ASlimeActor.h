// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "GameFramework/Actor.h"
#include "ASlimeActor.generated.h"

class UDynamicMeshComponent;
class UMaterialInterface;
class USphereComponent;

struct FSlimeParticle
{
	FVector Position;
	FVector PrevPosition;
	FVector Velocity;
	float Mass = 1.0f;
	
	float CollisionLambda = 0.0f;   // 누적 라그랑주 승수
};

struct FDistanceConstraint
{
	int32 A;
	int32 B;
	float RestLength;
	
	float Lambda = 0.0f;			// 누적 라그랑주 승수
};

struct FTriangle
{
	int32 A;
	int32 B;
	int32 C;
};

UCLASS()
class EXPEDITION_MALLANG_API AASlimeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASlimeActor();

	// StaticMesh를 DynamicMesh로 변환하는 유틸리티 함수
	static void ConvertStaticMeshToDynamicMesh(const UStaticMesh* StaticMesh, UE::Geometry::FDynamicMesh3& OutMesh);

	// Particle과 Constraint 초기화
	void InitializeParticlesAndConstraints();
	
	void SolveDistanceConstraints(TArray<FDistanceConstraint>& Constraint, float DeltaTime);
	void SolveVolumeConstraints(float DeltaTime);
	void SolveCollision(float DeltaTime);
	
	FVector ComputeParticleCenter();
	float ComputeVolume();
	static void ComputeTriangleGradients(const FVector& A, const FVector& B, const FVector& C, FVector& GradA, FVector& GradB, FVector& GradC);
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 소스 메쉬 (에디터에서 할당)
	UPROPERTY(EditAnywhere)
	UStaticMesh* SourceMesh;
	
	// 메테리얼 (에디터에서 할당)
	UPROPERTY(EditAnywhere)
	UMaterialInterface* SourceMaterial;
	
	// 런타임용 DynamicMeshComponent
	UPROPERTY(VisibleAnywhere)
	UDynamicMeshComponent* DynamicMeshComp;
	
	// 물리 충돌용 Sphere Component
	UPROPERTY(VisibleAnywhere, Category = "Physics")
	USphereComponent* SphereCollision;
	
	float CurrentTime = 0.0f;
	
	TArray<FSlimeParticle> Particles;
	TArray<FDistanceConstraint> Constraints;
	TArray<FTriangle> Triangles;

	// Sphere Collision 반지름
	UPROPERTY(EditAnywhere, Category = "Physics")
	float SphereRadius = 60.0f;
	
	// 솔버 반복 횟수
	UPROPERTY(EditAnywhere, Category = "Slime")
	int32 SolverIterations = 5;
	
	// 부피 보존 강도
	UPROPERTY(EditAnywhere, Category = "Slime", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float VolumeStiffness = 0.2f; 
	
	// 부피 컴플라이언스
	UPROPERTY(EditAnywhere, Category = "Slime", meta = (UIMin = "0.0", UIMax = "0.001", SliderExponent = "6"))
	float VolumeCompliance = 1e-6f;

	// 거리 컴플라이언스
	UPROPERTY(EditAnywhere, Category = "Slime", meta = (UIMin = "0.0", UIMax = "0.001", SliderExponent = "6"))
	float DistanceCompliance = 1e-6f;

	// 충돌 컴플라이언스
	UPROPERTY(EditAnywhere, Category = "Slime", meta = (UIMin = "0.0", UIMax = "0.001", SliderExponent = "6"))
	float CollisionCompliance = 1e-6f; 
	
	float RestVolume = 0.0f;			// 초기 부피
	float VolumeLambda = 0.0f;			// 누적 라그랑주 승수
	
	float Friction = 0.1f;				// 마찰 계수
	float Gravity = -980.f;				// 중력 가속도 
	
	UPROPERTY()
	TSet<TObjectPtr<AActor>> OverlappingActors;
};