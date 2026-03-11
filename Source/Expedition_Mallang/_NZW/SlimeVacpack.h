// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlimeVacpack.generated.h"

class UArrowComponent;
class USphereComponent;
class UNiagaraComponent;

struct FSlot;

UCLASS()
class EXPEDITION_MALLANG_API ASlimeVacpack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASlimeVacpack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
/*! 함수 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponFirstMesh() const { return WeaponFirst; }
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponThirdMesh() const { return WeaponThird; }
	
	UFUNCTION()
	void ClearInventorySlot();
	
	UFUNCTION()
	void SaveInventorySlot();
	UFUNCTION()
	void LoadInventorySlot();
	
	UFUNCTION()
	void SetWaveCannonForce(float Force) { WaveCannonForce = Force; }
	
	// 감지 범위 안의 Vacuumable Actor 목록을 매 프레임 갱신
	UFUNCTION()
	void VacuumDetecting();
	// 감지된 슬라임에 흡입 물리(스프링-댐퍼+소용돌이) 적용
	UFUNCTION()
	void Vacuuming(float DeltaTime);
	// 흡입 중단 → 목록 + 슬라임별 속도 상태 전부 초기화
	UFUNCTION()
	void StopVacuuming();
	// 총과 충돌 시 Invectory Slot에 추가 
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void ShowItemInformation();
	
	UFUNCTION()
	int32 FindEmptySlot(FName ID);
	
	UFUNCTION()
	void SelectSlot(int32 SlotNum);
	
	UFUNCTION()
	void FireVacuumable();
	
	UFUNCTION()
	void WaveCannon();
	
/*! 변수 */
public:
	UPROPERTY()
	class ASlimePlayer* SlimePlayer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* WeaponFirst;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* WeaponThird;	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* Muzzle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* VacuumCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UNiagaraComponent* StormComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UNiagaraComponent* WaveComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Inventory", meta=(AllowPrivateAccess="true"))
	TArray<FSlot> Inventory; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Inventory", meta=(AllowPrivateAccess="true"))
	int32 SelectSlotNumber = 0;
	int32 PrevSelectSlot = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Detect", meta=(AllowPrivateAccess="true"))
	bool bIsVacuuming = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Detect", meta=(AllowPrivateAccess="true"))
	float DetectRange = 1200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Detect", meta=(AllowPrivateAccess="true"))
	float DetectFOV = 30.0f;
	
	UPROPERTY()
	TArray<AActor*> CurrentVacuumTargets;	// Detect 한 Actor들
	TArray<TEnumAsByte<EObjectTypeQuery>> VacuumObjectTypes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Vacuuming", meta=(AllowPrivateAccess="true"))
	float SpringK = 4.0f;					// 스프링 세기: 클수록 중심으로 더 빠르게 수렴
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Vacuuming", meta=(AllowPrivateAccess="true"))
	float DamperC = 2.0f;					// 댐퍼 세기: 클수록 진동이 빨리 사라짐
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Vacuuming", meta=(AllowPrivateAccess="true"))
	float VacForce = 1200.0f;			// 흡입 세기 (클수록 빠르게 당김)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Vacuuming", meta=(AllowPrivateAccess="true"))
	float AxisDamping = 2.0f;			// 축 방향 과속 방지 댐핑
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Vacuuming", meta=(AllowPrivateAccess="true"))
	float SwirlStrength = 500.0f;	// 소용돌이 세기 (클수록 더 빠르게 회전)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Vacuuming", meta=(AllowPrivateAccess="true"))
	float LiftForce = 400.0f;			// 중력을 상쇄하고도 남는 추가 부양력
	
	// 슬라임의 측면(좌우) 속도 벡터 - 스프링-댐퍼 계산에 사용
	// FVector를 쓰는 이유: 측면은 3D 공간에서 어떤 방향이든 될 수 있어서 벡터가 필요함
	UPROPERTY()
	TMap<AActor*, FVector> VacuumLateralVelocities;

	// 슬라임의 축 방향(앞뒤) 속도 스칼라 - 노즐 쪽으로 빨려 들어오는 속도
	// float를 쓰는 이유: 축 방향은 Forward 벡터 하나로 이미 방향이 정해져 있어서
	// 얼마나 빠른지(크기)만 저장하면 됨
	UPROPERTY()
	TMap<AActor*, float> VacuumAxisVelocities;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|Fire", meta=(AllowPrivateAccess="true"))
	float FireForce = 1000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|WaveCannon", meta=(AllowPrivateAccess="true"))
	float WaveCannonRange = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|WaveCannon", meta=(AllowPrivateAccess="true"))
	float WaveCannonForce = 2000.0f;
	
	/** Sound */
	UPROPERTY(EditAnywhere, Category = "SlimePlayer|Sound")
	USoundBase* FireSound;
};
