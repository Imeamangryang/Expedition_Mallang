// Fill out your copyright notice in the Description page of Project Settings.


#include "SlimeVacpack.h"

#include "NZW_TestSlime.h"
#include "SlimePlayer.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Slime/ASlimeActor.h"

// Sets default values
ASlimeVacpack::ASlimeVacpack()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//. 루트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	//. 1인칭 무기 & 3인칭 무기 생성
	WeaponFirst = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon First"));
	WeaponThird = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Third"));
	
	//. 무기 메쉬 입히기 (여기는 Skeletal Mesh)
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Weapon(TEXT("/Game/Weapons/GrenadeLauncher/Meshes/SKM_GrenadeLauncher.SKM_GrenadeLauncher"));
	if (Weapon.Succeeded()) 
	{
		WeaponFirst->SetSkeletalMeshAsset(Weapon.Object);
		WeaponThird->SetSkeletalMeshAsset(Weapon.Object);
	}
	
	//. 각각 맞는 설정으로 초기화
	WeaponFirst->SetupAttachment(RootComponent);
	WeaponFirst->SetOnlyOwnerSee(true);
	WeaponFirst->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	WeaponFirst->SetCollisionProfileName(FName("NoCollision"));
	
	WeaponThird->SetupAttachment(RootComponent);
	WeaponThird->SetOwnerNoSee(true);
	WeaponThird->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	WeaponThird->SetCollisionProfileName(FName("NoCollision"));
	
	//. 총구 설정
	Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(WeaponFirst);
	Muzzle->SetRelativeLocationAndRotation(FVector(0.0f, 23.0f, 11.0f), FRotator(0.0f, 90.0f, 0.0f));
	Muzzle->SetHiddenInGame(false);
	
	//. 흡입 입구 콜리전
	VacuumCollision = CreateDefaultSubobject<USphereComponent>(TEXT("VacuumCollision"));
	VacuumCollision->SetupAttachment(Muzzle);
	VacuumCollision->SetSphereRadius(50.f);
	VacuumCollision->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
	
	static ConstructorHelpers::FObjectFinder<UDataTable> DT_Vacable(TEXT("/Script/Engine.DataTable'/Game/_NZW/03_Data/DT_VacuumablInfo.DT_VacuumablInfo'"));
	if (DT_Vacable.Succeeded()) VacuumableDataTable = DT_Vacable.Object;
}

// Called when the game starts or when spawned
void ASlimeVacpack::BeginPlay()
{
	Super::BeginPlay();
	
	//. 충돌 채널 저장
	VacuumObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel2));
	VacuumObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	
	//. 오버랩 활성화
	VacuumCollision->OnComponentBeginOverlap.AddDynamic(this, &ASlimeVacpack::OnComponentBeginOverlap);
	VacuumCollision->SetGenerateOverlapEvents(false);
	
	//. Inventory 슬롯 4칸 고정
	Inventory.SetNum(4);
	
	//. Delegate를 위한 플레이어 저장
	SlimePlayer = Cast<ASlimePlayer>(GetOwner());
}

// Called every frame
void ASlimeVacpack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//. Item을 바라보면 정보를 띄우도록
	ShowItemInformation();
	
	//. bIsVacuuming은 RMB 누를 때 true, 뗄 때 false (Slime Player에서 제어)
	if (bIsVacuuming)
	{
		//. 감지 범위를 다시 스캔해서 목록 갱신 (벗어난 슬라임 자동 제거됨)
		VacuumDetecting();
		
		//. 감지된 슬라임이 있으면 흡입 물리 적용
		if (CurrentVacuumTargets.Num() > 0)
			Vacuuming(DeltaTime);
	}
}

void ASlimeVacpack::SelectSlot(int32 SlotNum)
{
	//. 선택한 슬롯 넘버
	SelectSlotNumber = SlotNum;
	
	//. UI 선택 Delegate Broadcast
	SlimePlayer->OnSelectSlot.Broadcast(SelectSlotNumber, PrevSelectSlot);
	
	//. UI Animation을 위한 저장
	PrevSelectSlot = SelectSlotNumber;
	
	// UE_LOG(LogTemp, Warning, TEXT("\nSlotNum: %d, ID: %s, Count: %d"), 
	// 	SelectSlotNumber, *Inventory[SelectSlotNumber].ID.ToString(), Inventory[SelectSlotNumber].Count);
}

int32 ASlimeVacpack::FindEmptySlot(FName ID)
{	
	// for (int32 i = 0; i < 4; i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("SlotNum: %d, ID: %s, Count: %d"), i, *Inventory[i].ID.ToString(), Inventory[i].Count);
	
	for (int32 i = 0; i < 4; i++)
	{
		//. ID가 같은 경우
		if (Inventory[i].ID.IsEqual(ID))
		{
			//. 개수가 20개 이하인 경우
			if (Inventory[i].Count < 20) //! Max개수 변수화
				return i;
			
			//. 개수가 20개 초과인 경우
			return INDEX_NONE;	
		}
		
		//. 비어있는 슬롯칸이 있는 경우
		if (Inventory[i].Count <= 0)
			return i;
	}

	//. 슬롯이 꽉 차있는 경우	
	return INDEX_NONE;	
}

void ASlimeVacpack::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//. Interface를 갖고있는 Actor인지 확인하고 -> 아이템 아이디 가져오는 Interface
	if (!OtherActor->Implements<UVacuumableInterface>()) return;
	FName ID = IVacuumableInterface::Execute_GetID(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("ID: %s"), *ID.ToString());
	
	//. 채울 수 있는 슬롯 확인
	int32 SlotNum = FindEmptySlot(ID);
	
	UE_LOG(LogTemp, Warning, TEXT("SlotNum: %d"), SlotNum);
	
	//. 없는 경우
	if (SlotNum == INDEX_NONE) return;
	
	//. 이미 FindEmptySlot에서 개수 확인했으니  습득 했다면 개수 ++
	Inventory[SlotNum].ItemClass = OtherActor->GetClass();
	Inventory[SlotNum].ID = ID;
	Inventory[SlotNum].Count++;
	
	//. 들어가는 슬롯 확인
	SelectSlot(SlotNum);
	// UE_LOG(LogTemp, Warning, TEXT("\nSlotNum: %d, ID: %s, Count: %d"), 
	// 	SlotNum, *Inventory[SlotNum].ID.ToString(), Inventory[SlotNum].Count);
	
	//. 습득 Delegate 실행 (UI에 띄우기 위한 매개변수)
	SlimePlayer->OnVacuuming.Broadcast(ID, Inventory[SlotNum].Count, SlotNum);
	
	//. 습득한 액터 파괴
	OtherActor->Destroy();
}

void ASlimeVacpack::ShowItemInformation()
{
	//. 마우스 포인터로 가리키면 정보 출력
	FHitResult HitResult;
	FVector Start = Muzzle->GetComponentLocation();
	FVector End = Start + (Muzzle->GetForwardVector() * 500.f); //! LineTrace 거리 변수화
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel2);
	ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	
	// DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 2.0f);
	bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ObjParams, Params);
	
	//. 감지된 액터가 없거나, 있더라도 VacuumableInterface 가 없는 액터라면 출력 없애고 넘어감
	if (!bHit || !HitResult.GetActor()->Implements<UVacuumableInterface>())
	{
		SlimePlayer->OnItemInfo.Broadcast("0");
		return;
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("ID: %s"), *HitResult.GetActor()->GetName());
	FName ID = IVacuumableInterface::Execute_GetID(HitResult.GetActor());
	SlimePlayer->OnItemInfo.Broadcast(ID);
}

// 감지거리 안에 들어온 Vacuumable Actor 찾기
void ASlimeVacpack::VacuumDetecting()
{
	if (!GetWorld()) return;
	
	VacuumCollision->SetGenerateOverlapEvents(true);
	// DrawDebugSphere(GetWorld(), VacuumCollision->GetComponentLocation(), 50, 16, FColor::Green);
	
	const FVector Origin = Muzzle->GetComponentLocation();
	const FVector Forward = Muzzle->GetForwardVector();
	
	// DrawDebugSphere(GetWorld(), Origin, DetectRange, 16, FColor::Red);
	DrawDebugCone(GetWorld(), Origin, Forward, DetectRange, FMath::DegreesToRadians(DetectFOV), FMath::DegreesToRadians(DetectFOV), 24, FColor::Green);
	
	//. 제외할 Actors
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	if (GetOwner()) IgnoreActors.Add(GetOwner());
	
	//. 받아올 Actors
	TArray<AActor*> OutActors;
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Origin, DetectRange, VacuumObjectTypes, AActor::StaticClass(), IgnoreActors,OutActors);
	
	//. Detect 결과를 기반으로 목록을 매 프레임 완전히 갱신
	CurrentVacuumTargets.Empty();
	
	if (!bHit) return;
	
	// FOV 체크: 노즐 Forward가 슬라임 방향과의 각도 비교
	// cos(FOV각도)보다 Dot값이 크면 = 각도가 더 좁다 = 시야각 안에 있다
	const float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(DetectFOV));
	for (AActor* OverlapResult : OutActors)
	{
		AActor* Target = OverlapResult;
		FVector Dir = (Target->GetActorLocation() - Origin).GetSafeNormal();
		float Dot = FVector::DotProduct(Forward, Dir);
		
		if (Dot >= CosHalfFOV)
		{
			CurrentVacuumTargets.Add(Target);
			DrawDebugSphere(GetWorld(), Target->GetActorLocation(), 100, 16, FColor::Blue);
		}
	}
}

// 흡입 물리 계산 (스프링-댐퍼 + 소용돌이)
void ASlimeVacpack::Vacuuming(float DeltaTime)
{
	// UE_LOG(LogTemp, Warning, TEXT("이제 흡입 물리 ~ Time"));
	
	//* ─────────────────────────────────────────────────────────────────
	//* 1. 노즐 기준 좌표 분해
	//* ─────────────────────────────────────────────────────────────────
	//* 슬라임의 위치를 노즐을 기준으로 "축 방향"과 "측면 방향" 두 개로 쪼갬
	//* 왜 쪼개냐? → 축 방향에는 "빨아들이는 힘"을, 측면에는 "스프링 흔들림 힘"을 각자 다르게 줘야 하기 때문
	//*
	//* 예시로 이해: 진공청소기 앞에 공이 있다고 할 때
	//*   [노즐→공]이 완전히 일직선이면 → 측면 성분 = 0, 축 성분만 존재
	//*   공이 옆으로 비껴있으면 → 측면 성분이 생기고 스프링 힘으로 흔들리며 중심으로 끌림
	//*──────────────────────────────────────────────────────────────────

	FVector MuzzleOrigin = Muzzle->GetComponentLocation();	// 노즐 위치 (흡입 기준점)
	FVector Forward = -Muzzle->GetForwardVector();			// 노즐이 바라보는 방향 (흡입 중심 축)

	// 이번 프레임에 유효하지 않는 슬라임 제거 (도중 삭제된 Actor 방어)
	CurrentVacuumTargets.RemoveAll( [](AActor* A) { return !IsValid(A); } );

	for (AActor* VacuumableActor : CurrentVacuumTargets)
	{
		if (!IsValid(VacuumableActor)) continue;

		//* ── 기본 정보 수집 ──
		FVector VacuumableLocation = VacuumableActor->GetActorLocation();
		float VacuumableMass = 1.0f; //! 나중에 Vacuumable 클래스(슬라임 베이스)에서 받아와야 함

		//* ─────────────────────────────────────────────────────────────────
		//* 속도 상태 불러오기
		//* ─────────────────────────────────────────────────────────────────
		//* 슬라임마다 "지금 얼마나 빠르게 움직이고 있는지"를 기억해야 함
		//* 이 값이 있어야 스프링-댐퍼가 관성 + 오버슈트 효과를 낼 수 있음
		//* 없으면(처음 잡히는 슬라임) 속도 0으로 초기화
		//*
		//* 측면(LateralVec)은 3D 벡터가 필요하고, 축 방향(AxisDist)은 스칼라(float)만 필요함
		//* 두 개를 같은 벡터에 섞으면 계산이 꼬이기 때문에 TMap을 분리해서 각자 저장
		
		// -- 측면 속도 초기화 (없으면) --
		if (!VacuumLateralVelocities.Contains(VacuumableActor))
		{
			VacuumLateralVelocities.Add(VacuumableActor, FVector::ZeroVector);
		}
		FVector& LateralVel = VacuumLateralVelocities[VacuumableActor];	// 측면 속도 벡터(참조)

		// -- 축 방향 속도 초기화 (없으면) --
		if (!VacuumAxisVelocities.Contains(VacuumableActor))
		{
			VacuumAxisVelocities.Add(VacuumableActor, 0.0f);
		}
		float& AxisVel = VacuumAxisVelocities[VacuumableActor];	// 축 방향 속도 스칼라(참조)

		//* ─────────────────────────────────────────────────────────────────
		//* 2. 벡터 분해 : ToMuzzle → 축 성분 + 측면 성분
		//* ─────────────────────────────────────────────────────────────────
		//* 슬라임 위치에서 노즐까지의 벡터를 구한 뒤
		//* Forward 방향으로 얼마나 향하고 있는지(내적 = 스칼라), 나머지는 옆 성분(벡터)으로 분리
		//*
		//* 수식 이해:
		//*   ToMuzzle = "슬라임 → 노즐" 방향 벡터 (길이 = 거리)
		//*   AxisDist  = ToMuzzle · Forward = 축 방향 거리 (스칼라)
		//*              → Forward와 같은 방향이면 양수, 반대면 음수
		//*   LateralVec = ToMuzzle - Forward * AxisDist
		//*              = 전체 벡터에서 축 성분을 빼면 → 순수하게 옆으로만 이동한 벡터

		FVector ToMuzzle   = MuzzleOrigin - VacuumableLocation;	// 슬라임→노즐 벡터
		float   AxisDist   = FVector::DotProduct(ToMuzzle, Forward);	// 노즐 축 방향 거리
		FVector LateralVec = ToMuzzle - Forward * AxisDist;		// 순수 측면 벡터 (옆으로 벗어난 정도)

		//* ─────────────────────────────────────────────────────────────────
		//* 3. 스프링-댐퍼 힘 (측면 흔들림)
		//* ─────────────────────────────────────────────────────────────────
		//* 슬라임이 중심축에서 벗어났을 때 중심으로 끌려오면서도
		//* 관성 때문에 중심을 지나쳐 반대쪽까지 갔다가 다시 돌아오는 진동 효과
		//*
		//* 멤버변수
		//* 스프링(SpringK): 멀수록 강하게 당기는 힘. F_spring = k × 거리
		//*   → 스프링처럼 늘어날수록 더 세게 잡아당김
		//*   → 클수록 진동이 더 빠르고 팽팽함
		//* 댐퍼(DamperC): 속도가 빠를수록 반대 방향으로 힘을 주는 제동력. F_damp = c × 속도
		//*   → 자동차 쇼크업소버처럼 빠를수록 더 강하게 브레이크를 걸어 진동을 줄임
		//*   → 클수록 진동이 빨리 소멸 (DamperC가 너무 크면 진동 없이 그냥 끌려옴)
		//*
		//* 뉴턴 제2법칙: F = m × a → a = F / m
		//*   → 질량이 크면 같은 힘으로도 가속도가 작아짐
		//*   → 무거운 슬라임은 덜 흔들림
		
		//. 측면 가속도 = (스프링 힘 + 댐퍼 힘) / 질량
		//*   LateralVec * SpringK : 현재 측면 거리만큼 중심 방향으로 힘 (스프링)
		//*   -LateralVel * DamperC : 현재 측면 속도 반대 방향으로 힘 (감쇠 브레이크)
		//*   둘 다 벡터이므로 방향이 자동으로 맞춰짐
		FVector LateralAccel = (LateralVec * SpringK - LateralVel * DamperC) / VacuumableMass;

		//* ─────────────────────────────────────────────────────────────────
		//* 4. 축 방향 흡입 힘
		//* ─────────────────────────────────────────────────────────────────
		//* 노즐 방향으로 슬라임을 빨아당기는 힘
		//* AxisDist가 작아질수록(노즐에 가까울수록) 더 빠르게 끌려오면 됨
		//* 
		//* 멤버변수 
		//* VacForce : 기본 흡입 세기 (항상 Forward 방향으로 당김)
		//* AxisDamping : 축 방향으로 너무 빠르게 달려와서 노즐을 뚫고 지나가지 않게 제동
		//*   → AxisVel이 클수록(빠를수록) 당기는 힘을 약하게 만들어 속도 조절
		//*
		//* 결과: 처음엔 느리게 접근하다가 중간에 가속, 노즐 근처에서 감속하는 느낌

		//. 축 방향 가속도 스칼라 (Forward 벡터 방향)
		//*   VacForce: 항상 양수 = 항상 노즐 쪽으로 당기는 힘
		//*   -AxisVel * AxisDamping: 축 방향 속도가 빠를수록 당기는 힘을 약화시켜 제동
		float AxisAccel = (VacForce /*- AxisVel * AxisDamping*/) / VacuumableMass;

		//* ─────────────────────────────────────────────────────────────────
		//* 5. 소용돌이 힘 (Swirl) - 중심축 기준 빙글빙글 회전
		//* ─────────────────────────────────────────────────────────────────
		//* 슬라임이 직선으로 빨려오는 게 아니라 나선형으로 빙글빙글 돌며 들어오는 효과
		//*
		//* 핵심 개념: 외적(Cross Product)
		//*   두 수직인 벡터를 외적하면 두 벡터에 모두 수직인 방향이 나옴
		//*   Forward × LateralNormal = 中心軸 주위를 도는 접선 방향
		//*
		//* 시각화:
		//*   Forward = 노즐 전방 (Z축처럼 위가 Forward)
		//*   LateralNormal = 슬라임이 중심에서 벗어난 방향 (X축처럼)
		//*   Cross(Forward, LateralNormal) = 그 두 방향에 수직인 방향 = Y축 방향 (회전 접선)
		//*   → 이 방향으로 힘을 주면 슬라임이 원을 그리며 돎!
		//*
		//* LateralNormal이 거의 0이면(중심에 딱 있으면) 소용돌이 힘도 0으로 자연스럽게 없어짐

		FVector LateralNormal = LateralVec.GetSafeNormal();	// 측면 방향 단위 벡터 (길이=1)
		//. 접선 방향 = Forward × 측면노멀 → 중심축을 기준으로 90도 돌아간 방향
		FVector TangentDir  = FVector::CrossProduct(Forward, LateralNormal);
		//. 소용돌이 가속도 = 접선 방향 × (소용돌이 힘 / 질량)
		FVector SwirlAccel  = TangentDir * (SwirlStrength / VacuumableMass);

		//* ─────────────────────────────────────────────────────────────────
		//* 6. 부양력 (중력 상쇄 + 위로 띄우는 힘)
		//* ─────────────────────────────────────────────────────────────────
		//* 문제 상황:
		//*   흡입 중 슬라임이 뜨지 않고 바닥에서만 끌려오는 이유 →
		//*   AxisVel은 Forward 방향(수평)으로만 당기고,
		//*   LateralVel에 중력 상쇄가 없어서 중력이 계속 Z방향으로 아래로 당기기 때문
		//*
		//* 해결 구조:
		//*   LateralVel은 3D 벡터이므로 Z(수직) 성분도 포함하여 추적함
		//*   → 여기에 위쪽 방향(+Z) 가속도를 더해주면 중력을 이길 수 있음
		//*
		//* UE 기본 중력 가속도 = 약 -980 cm/s² (GetGravityZ()로 가져옴, 음수)
		//* -GravityZ = +980 → 중력과 정확히 반대 방향 = 중력 상쇄
		//* 멤버변수
		//* LiftForce = 추가로 더 위로 띄우는 힘 (중력 상쇄만으론 제자리이므로)
		//*
		//* 최종 부양 가속도 = (-중력 + 추가부양력) / 질량
		//*   → 질량이 클수록 같은 힘에 덜 뜸 (무거운 슬라임은 천천히 뜸)

		float GravityZ  = GetWorld()->GetGravityZ();	// 보통 -980.f (아래 방향이라 음수)

		//. 부양 가속도 = Z방향(위쪽) 으로: 중력 상쇄분 + 추가 부양력
		//*   -GravityZ : 중력이 -980이면, -(-980) = +980 → 중력과 정확히 상쇄
		//*   LiftForce : 상쇄 후에도 추가로 위로 밀어올리는 힘
		FVector LiftAccel = FVector(0.f, 0.f, (-GravityZ + LiftForce) / VacuumableMass);

		//* ─────────────────────────────────────────────────────────────────
		//* 7. 오일러 적분 (속도 + 위치 업데이트)
		//* ─────────────────────────────────────────────────────────────────
		//* 매 프레임 "이번 프레임 가속도 × DeltaTime"만큼 속도를 쌓고,
		//* "이번 프레임 속도 × DeltaTime"만큼 위치를 이동시킴
		//*
		//* 왜 DeltaTime을 곱하냐? → 프레임 독립성
		//*   60fps : DeltaTime ≈ 0.016초, 1초에 60번 더해짐
		//*   30fps : DeltaTime ≈ 0.033초, 1초에 30번 더해짐
		//*   두 경우 모두 1초 뒤 결과가 동일하게 됨
		//*
		//* 오일러 적분 수식:
		//*   시간:  t0       t0+Δt     t0+2Δt
		//*   가속도: a        a         a
		//*   속도:   v0   →  v0+aΔt	 → v0+2aΔt   ← 이게 속도 적분
		//*   위치:   x0   →  x0+v0Δt → ...       ← 이게 위치 적분

		// -- 측면 속도 업데이트 : 스프링-댐퍼 + 소용돌이 + 부양력 --
		//* LateralVel은 3D 벡터(X,Y,Z 모두 포함)이므로
		//* LiftAccel(위쪽 Z방향)을 여기에 더하면 수직 방향 부양이 자연스럽게 적용됨
		LateralVel += (LateralAccel + SwirlAccel + LiftAccel) * DeltaTime;

		// -- 축 방향 속도 업데이트 --
		//* 이번 프레임 축 속도 = 지난 프레임 축 속도 + 흡입 가속도 × DeltaTime
		AxisVel += AxisAccel * DeltaTime;

		// -- 최종 이동 벡터 계산 --
		//* 측면 이동 = 측면 속도 방향으로 이만큼 (Z 부양 포함)
		//* 축 이동   = 노즐 Forward 방향으로 이만큼
		FVector NewLocation = VacuumableLocation
			+ LateralVel * DeltaTime						// 측면(좌우 흔들림 + 소용돌이 + 부양) 이동
			+ Forward * (AxisVel * DeltaTime);	// 축 방향(노즐 쪽으로) 이동

		VacuumableActor->SetActorLocation(NewLocation, true);	// true = 충돌 체크

		//todo : 슬라임이 VacuumCollision 안에 들어오면 실제로 "흡수" 처리 (목록에서 제거 + 수납)
	}
}

// RMB 뗐을 때 한 번 호출 → 흡입 완전 중단
void ASlimeVacpack::StopVacuuming()
{
	// 흡입 플래그 끄기 → Tick에서 VacuumDetecting / Vacuumming이 더 이상 실행 안 됨
	bIsVacuuming = false;
	VacuumCollision->SetGenerateOverlapEvents(false);

	// 감지 목록 초기화
	// → 다음에 다시 RMB를 누를 때 깨끗한 상태에서 시작
	CurrentVacuumTargets.Empty();

	// 슬라임별 속도 상태 초기화
	// → 이걸 안 지우면, 나중에 같은 슬라임을 다시 흡입할 때
	//   이전에 가속되어 있던 속도가 남아서 처음부터 튀어나가는 현상이 생김
	VacuumLateralVelocities.Empty();	// 측면(스프링-댐퍼) 속도
	VacuumAxisVelocities.Empty();		// 축 방향(흡입) 속도
}

// LMB 클릭 시 선택 되어있는 슬롯에 아이템이 있고 개수가 충분하면 하나씩 발사 (Delay 0.5초)
void ASlimeVacpack::FireVacuumable()
{
	//. 선택 되어있는 슬롯에 아이템이 존재 하지 않으면 return
	if (Inventory[SelectSlotNumber].Count <= 0)	return;
	
	FVector Start = Muzzle->GetComponentLocation() + Muzzle->GetForwardVector() * 100.f; //! FireSpawnPoint 변수화 
	AActor* Item = GetWorld()->SpawnActor<AActor>(
		Inventory[SelectSlotNumber].ItemClass, Start, FRotator::ZeroRotator);
	
	if (AASlimeActor* Slime = Cast<AASlimeActor>(Item))
	{
		Slime->ID = Inventory[SelectSlotNumber].ID;
		Slime->ApplySlimeMovementImpulse(Muzzle->GetForwardVector(), 10000.0f, 0.f);
	}
	else if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Item->GetRootComponent()))
	{
		IVacuumableInterface::Execute_SetID(Item, Inventory[SelectSlotNumber].ID);
		Primitive->AddImpulse(Muzzle->GetForwardVector() * 1000.0f,NAME_None, true); //! ImpulseForce 변수화
	}
	
	//. 개수 제거 후 Delegate 실행 (UI에 띄우기 위한 매개변수)
	Inventory[SelectSlotNumber].Count--;
	
	if (Inventory[SelectSlotNumber].Count <= 0)
	{
		Inventory[SelectSlotNumber].ItemClass = nullptr;
		Inventory[SelectSlotNumber].ID = "100";
		Inventory[SelectSlotNumber].Count = 0;
	}

	// UE_LOG(LogTemp, Error, TEXT("\nSlotNum: %d, ID: %s, Count: %d"), 
	// 	SelectSlotNumber, *Inventory[SelectSlotNumber].ID.ToString(), Inventory[SelectSlotNumber].Count);
	SlimePlayer->OnVacuuming.Broadcast(Inventory[SelectSlotNumber].ID, Inventory[SelectSlotNumber].Count, SelectSlotNumber);
}
