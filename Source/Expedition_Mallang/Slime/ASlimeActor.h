// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "_NZW/VacuumableInterface.h"
#include "GameFramework/Actor.h"
#include "ASlimeActor.generated.h"

class UDynamicMeshComponent;
class UMaterialInterface;
class USphereComponent;
class UNiagaraSystem;

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
	bool bBroken = false;			// 제약 조건 파손 여부
};

struct FTriangle
{
	int32 A;
	int32 B;
	int32 C;
};

UCLASS()
class EXPEDITION_MALLANG_API AASlimeActor : public AActor, public IVacuumableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AASlimeActor();

	// StaticMesh를 DynamicMesh로 변환하는 유틸리티 함수
	static void ConvertStaticMeshToDynamicMesh(const UStaticMesh* StaticMesh, UE::Geometry::FDynamicMesh3& OutMesh);

	// Particle과 Constraint 초기화
	void InitializeParticlesAndConstraints();
	
	//  제약 조건 해결 함수
	void SolveDistanceConstraints(TArray<FDistanceConstraint>& Constraint, float DeltaTime);
	void SolveVolumeConstraints(float DeltaTime);
	void SolveCollision(float DeltaTime);
	
	void RunXPBD(float DeltaTime, int32 LOD);
	int32 CalculateLOD() const;
	
	FVector ComputeParticleCenter();
	float ComputeVolume();
	static void ComputeTriangleGradients(const FVector& A, const FVector& B, const FVector& C, FVector& GradA, FVector& GradB, FVector& GradC);
	
	static void ShuffleConstraints(TArray<FDistanceConstraint>& Constraints);
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION(BlueprintCallable)
	void ApplySlimeMovementImpulse(const FVector& Direction, float MoveStrength, float JumpStrength);
	
	//. 인터페이스 함수
	virtual FName GetID_Implementation() override;
	virtual UClass* GetItemActorClass_Implementation() override;
	
	UFUNCTION(BlueprintCallable)
	void SetSlimeSphereRadius(float NewRadius);
	
	UFUNCTION(BlueprintCallable)
	void BurstSlime();
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slime")
	UDynamicMeshComponent* DynamicMeshComp;
	
	// 물리 충돌용 Sphere Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slime")
	USphereComponent* SphereCollision;
	
	TArray<FSlimeParticle> Particles;
	TArray<FDistanceConstraint> Constraints;
	TArray<FTriangle> Triangles;

	// Sphere Collision 반지름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slime")
	float SphereRadius = 60.0f;
	
	// 솔버 반복 횟수
	UPROPERTY(EditAnywhere, Category = "Slime")
	int32 SolverIterations_LOD0 = 5;
	
	UPROPERTY(EditAnywhere, Category = "Slime")
	int32 SolverIterations_LOD1 = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bProtectedZone = false;
	
	// 부피 컴플라이언스
	UPROPERTY(EditAnywhere, Category="Slime", meta=(ClampMin="0", ClampMax="10"))
	int32 VolumeStiffness = 6;
	float VolumeCompliance;

	// 거리 컴플라이언스
	UPROPERTY(EditAnywhere, Category="Slime", meta=(ClampMin="0", ClampMax="10"))
	int32 DistanceStiffness = 6;
	float DistanceCompliance;

	// 충돌 컴플라이언스
	UPROPERTY(EditAnywhere, Category="Slime", meta=(ClampMin="1", ClampMax="10"))
	int32 CollisionStiffness = 6;
	float CollisionCompliance; 
	
	
	float RestVolume = 0.0f;			// 초기 부피
	float VolumeLambda = 0.0f;			// 누적 라그랑주 승수
	
	float Friction = 0.1f;				// 마찰 계수
	float Gravity = -980.f;				// 중력 가속도 
	
	// 낙하 판정 
	bool bWasGrounded = false;
	int32 GroundContactParticleThreshold = 5;   // 최소 접촉 파티클 수
	float LandingVelocityThreshold = -150.f;    // 낙하 속도 조건
	
	UPROPERTY()
	TSet<TObjectPtr<AActor>> OverlappingActors;
	
	UPROPERTY(EditAnywhere, Category="Slime")
	TSubclassOf<AActor> CarrotClass;

	// 생성할 플롯 클래스
	UPROPERTY(EditAnywhere, Category="Slime")
	TSubclassOf<AActor> PlortClass;
	
	UPROPERTY(EditAnywhere, Category="Slime")
	UNiagaraSystem* CollisionFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Slime")
	USoundBase* LandingSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FX")
	USoundBase* JumpSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID = "101";
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ActorLOD = 0; 
};
