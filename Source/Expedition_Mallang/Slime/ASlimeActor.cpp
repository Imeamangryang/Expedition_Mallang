// Fill out your copyright notice in the Description page of Project Settings.
#include "Slime/ASlimeActor.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"

// Sets default values
AASlimeActor::AASlimeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetNotifyRigidBodyCollision(true);
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetSimulatePhysics(false);
	SphereCollision->SetEnableGravity(false);
	SphereCollision->SetupAttachment(DynamicMeshComp);
	SphereCollision->SetSphereRadius(SphereRadius);
	SphereCollision->SetLineThickness(1.0f);
	SphereCollision->bHiddenInGame = false;
	
	// Sphere Collision 오버랩 이벤트 바인딩
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AASlimeActor::OnSphereOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AASlimeActor::OnSphereEndOverlap);
	
	// DynamicMeshComponent는 시각적 표현만 담당
	DynamicMeshComp = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComp"));
	DynamicMeshComp->SetupAttachment(RootComponent);
	DynamicMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DynamicMeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	DynamicMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	DynamicMeshComp->SetMobility(EComponentMobility::Movable);
	
	// Static Mesh 경로 설정
	SourceMesh = Cast<UStaticMesh>(StaticLoadObject(
		UStaticMesh::StaticClass(),
		nullptr,
		TEXT("/Engine/BasicShapes/Sphere.Sphere")
	));

	// Material Instance 경로 설정
	SourceMaterial = Cast<UMaterialInterface>(StaticLoadObject(
		UMaterialInterface::StaticClass(),
		nullptr,
		TEXT("/Game/Assets/Models/cute_enemy_slime_gltf_extracted/scene/Materials/SmallSlime1.SmallSlime1")
	));
}

// Called when the game starts or when spawned
void AASlimeActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Particle과 Constraint 초기화
	InitializeParticlesAndConstraints();
}

void AASlimeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (!SourceMesh) return;

	UE::Geometry::FDynamicMesh3 DynMesh;

	// dynamic mesh로 변환
	ConvertStaticMeshToDynamicMesh(SourceMesh, DynMesh);

	// DynamicMeshComponent에 적용
	DynamicMeshComp->GetDynamicMesh()->SetMesh(MoveTemp(DynMesh));
	DynamicMeshComp->SetComplexAsSimpleCollisionEnabled(true, true);
	DynamicMeshComp->NotifyMeshUpdated();
	
	// Material 적용
	if (SourceMaterial)
	{
		DynamicMeshComp->SetMaterial(0, SourceMaterial);
	}
}

// Called every frame
void AASlimeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector Center = ComputeParticleCenter();
	
	// Actor를 중심 위치로 이동
	FVector ActorWorldPos = GetActorLocation();
	SetActorLocation(ActorWorldPos + Center);
	
	// Particle이 초기화되지 않았으면 반환
	if (Particles.Num() == 0)
	{
		return;
	}
	
	CurrentTime += DeltaTime;
	
	// Particle들을 반대로 이동 (로컬 유지)
	for (FSlimeParticle& P : Particles)
	{
		P.Position -= Center;
		P.CollisionLambda = 0.0f;
	}
	
	// Lambda 초기화
	for (FDistanceConstraint& C : Constraints)
	{
		C.Lambda = 0.0f;
	}
	VolumeLambda = 0.0f;
	
	/*
	 * for all Particles i : 
	 * v_i = v_i + g * dt		| 속도 업데이트
	 * p_i_prev = p_i			| 현재 위치 저장
	 * p_i = p_i + v_i * dt		| 위치 업데이트
	 */
	for (FSlimeParticle& P : Particles)
	{
		P.Velocity.Z += Gravity * DeltaTime;
		P.PrevPosition = P.Position;
		P.Position += P.Velocity * DeltaTime;
	}
	
	/*
	 * 제약 조건 해결 (SolverIterations 만큼 반복)
	 * for all Constraints c :
	 *	SolveConstraint(c)
	 */
	for (int32 Iter = 0; Iter < SolverIterations; Iter++)
	{
		// 거리 제약 조건 Solve
		SolveDistanceConstraints(Constraints, DeltaTime);
		
		// 부피 보존 제약 조건 Solve
		SolveVolumeConstraints(DeltaTime);
		
		// 충돌 조건 해결
		SolveCollision(DeltaTime);
	}
	
	/*
	 * for all Particles i :
	 * v_i = (p_i - p_i_prev) / dt	| 속도 업데이트
	 */
	for (FSlimeParticle& P : Particles)
	{
		P.Velocity = (P.Position - P.PrevPosition) / DeltaTime;
	}
	
	DynamicMeshComp->GetDynamicMesh()->EditMesh(
	[this](FDynamicMesh3& Mesh)
	{
		for (int32 vid : Mesh.VertexIndicesItr())
		{
			Mesh.SetVertex(vid, static_cast<FVector3d>(Particles[vid].Position));
		}
	},
	EDynamicMeshChangeType::GeneralEdit,
	EDynamicMeshAttributeChangeFlags::VertexPositions
	);

	DynamicMeshComp->NotifyMeshUpdated();
	DynamicMeshComp->UpdateCollision();
}

// StaticMesh를 DynamicMesh로 변환하는 유틸리티 함수
void AASlimeActor::ConvertStaticMeshToDynamicMesh(const UStaticMesh* StaticMesh, UE::Geometry::FDynamicMesh3& OutMesh)
{
	if (!StaticMesh) return;

	// LOD0 사용
	const FMeshDescription* MeshDesc = StaticMesh->GetMeshDescription(0);

	if (!MeshDesc) return;

	FStaticMeshAttributes Attributes(const_cast<FMeshDescription&>(*MeshDesc));
	auto VertexPositions = Attributes.GetVertexPositions();

	// VertexID → DynamicMesh VertexID 매핑
	TMap<FVertexID, int> VertexMap;

	// 버텍스 복사
	for (const FVertexID VertexID : MeshDesc->Vertices().GetElementIDs())
	{
		FVector3f Pos = VertexPositions[VertexID];
		int NewID = OutMesh.AppendVertex(static_cast<FVector3d>(Pos));
		VertexMap.Add(VertexID, NewID);
	}

	// 트라이앵글 복사
	for (const FTriangleID TriID : MeshDesc->Triangles().GetElementIDs())
	{
		TArrayView<const FVertexInstanceID> InstanceIDs =
			MeshDesc->GetTriangleVertexInstances(TriID);

		int V0 = VertexMap[
			MeshDesc->GetVertexInstanceVertex(InstanceIDs[0])];
		int V1 = VertexMap[
			MeshDesc->GetVertexInstanceVertex(InstanceIDs[1])];
		int V2 = VertexMap[
			MeshDesc->GetVertexInstanceVertex(InstanceIDs[2])];

		OutMesh.AppendTriangle(V0, V1, V2);
	}
}

// Particle과 Constraint 초기화 함수
void AASlimeActor::InitializeParticlesAndConstraints()
{
	Particles.Empty();
	Constraints.Empty();
	Triangles.Empty();

	FDynamicMesh3* Mesh = DynamicMeshComp->GetDynamicMesh()->GetMeshPtr();
	if (!Mesh) return;

	// Particle 생성
	for (int32 vid : Mesh->VertexIndicesItr())
	{
		FVector3d Pos = Mesh->GetVertex(vid);

		FSlimeParticle P;
		P.Position = (FVector)Pos;
		P.PrevPosition = P.Position;
		P.Velocity = FVector::ZeroVector;

		Particles.Add(P);
	}
	
	// Constraint 생성
	for (int32 eid : Mesh->EdgeIndicesItr())
	{
		UE::Geometry::FIndex2i Edge = Mesh->GetEdgeV(eid);

		int32 A = Edge.A;
		int32 B = Edge.B;

		float RestLength = FVector::Distance(
			Particles[A].Position,
			Particles[B].Position
		);

		FDistanceConstraint C;
		C.A = A;
		C.B = B;
		C.RestLength = RestLength;

		Constraints.Add(C);
	}
	
	// Triangle 생성
	for (int32 tid : Mesh->TriangleIndicesItr())
	{
		UE::Geometry::FIndex3i Tri = Mesh->GetTriangle(tid);

		FTriangle T;
		T.A = Tri.A;
		T.B = Tri.B;
		T.C = Tri.C;

		Triangles.Add(T);
	}
	RestVolume = ComputeVolume();
}

// 거리 제약 조건 해결 함수
void AASlimeActor::SolveDistanceConstraints(TArray<FDistanceConstraint>& Constraint, float DeltaTime)
{
	/*
	 * XPBD 기반 거리 제약 조건 해결
	 * 
	 * 각 Edge에 대해:
	 * 1. 현재 거리와 Rest 거리의 차이를 계산 → Constraint 값 C(p)
	 * 2. Gradient 계산 → ∇C = (p1 - p2) / |p1 - p2|
	 * 3. XPBD Compliance를 통한 강성 조절 → α = compliance / Δt²
	 * 4. Lagrange Multiplier 증분 계산 → Δλ = -(C + α·λ) / (w1 + w2 + α)
	 * 5. λ 누적 후 위치 보정 → Δp = Δλ · ∇C · w
	 */
	for (FDistanceConstraint& C : Constraints)
	{
		FSlimeParticle& P1 = Particles[C.A];
		FSlimeParticle& P2 = Particles[C.B];

		FVector Delta = P1.Position - P2.Position;
		float Length = Delta.Size();

		// 매우 짧은 거리는 무시
		if (Length < KINDA_SMALL_NUMBER)
			continue;

		FVector Grad = Delta / Length;
		float Cval = Length - C.RestLength;

		float w1 = 1.0f;
		float w2 = 1.0f;

		float Alpha = DistanceCompliance / (DeltaTime * DeltaTime);
		float DeltaLambda = (-Cval - Alpha * C.Lambda) / (w1 + w2 + Alpha);

		C.Lambda += DeltaLambda;

		FVector Correction = DeltaLambda * Grad;

		P1.Position += w1 * Correction;
		P2.Position -= w2 * Correction;
	}
}

// 부피 보존 제약 조건 해결 함수
void AASlimeActor::SolveVolumeConstraints(float DeltaTime)
{
	/*
	 * XPBD 기반 부피 보존 제약 조건 해결
	 * 
	 * 1. 현재 부피와 Rest 부피의 차이 계산 → Constraint 값 C(p) = V_current - V_rest
	 * 2. 각 삼각형에 대해 부피 Gradient 계산 → ∇V = (B × C) / 6 등
	 * 3. 모든 파티클의 Gradient를 누적하여 전체 Gradient 벡터 생성
	 * 4. Denominator 계산 → Σ(w_i · |∇V_i|²)
	 * 5. XPBD Compliance를 통한 강성 조절 → α = compliance / Δt²
	 * 6. Lagrange Multiplier 증분 계산 → Δλ = -(C + α·λ) / (Sum + α)
	 * 7. λ 누적 후 모든 파티클의 위치 보정 → Δp_i = w_i · ∇V_i · Δλ
	 */
	float CurrentVolume = ComputeVolume();
	float Constraint = CurrentVolume - RestVolume;

	if (FMath::Abs(Constraint) < 1e-4f)
		return;

	TArray<FVector> Gradients;
	Gradients.Init(FVector::ZeroVector, Particles.Num());

	float Sum = 0.0f;

	for (const FTriangle& Tri : Triangles)
	{
		int32 ia = Tri.A;
		int32 ib = Tri.B;
		int32 ic = Tri.C;

		const FVector& A = Particles[ia].Position;
		const FVector& B = Particles[ib].Position;
		const FVector& Cpos = Particles[ic].Position;

		FVector GradA, GradB, GradC;
		ComputeTriangleGradients(A, B, Cpos, GradA, GradB, GradC);

		Gradients[ia] += GradA;
		Gradients[ib] += GradB;
		Gradients[ic] += GradC;
	}

	for (int32 i = 0; i < Particles.Num(); i++)
	{
		float InvMass = 1.0f / Particles[i].Mass;
		Sum += InvMass * Gradients[i].SizeSquared();
	}

	if (Sum < 1e-6f)
		return;

	float Alpha = VolumeCompliance / (DeltaTime * DeltaTime);
	float DeltaLambda = -(Constraint + Alpha * VolumeLambda) / (Sum + Alpha);

	VolumeLambda += DeltaLambda;

	for (int32 i = 0; i < Particles.Num(); i++)
	{
		float InvMass = 1.0f / Particles[i].Mass;

		Particles[i].Position += InvMass * Gradients[i] * DeltaLambda;
	}
}

// 충돌 해결 함수
void AASlimeActor::SolveCollision(float DeltaTime)
{
	if (OverlappingActors.Num() == 0)
		return;
	
	TArray<FHitResult> Hits;

	FVector Start = SphereCollision->GetComponentLocation();
	FVector End = Start;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	bool bHit = GetWorld()->SweepMultiByObjectType(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(SphereRadius),
		Params
	);
	
	if (!bHit) return;
	
	bool bImpulseHit = false;
	FVector ImpulseContactPoint;
	FVector ImpulseContactNormal;
	float ImpulseStrength = 0.0f;
	
	FTransform ActorTransform = GetActorTransform();

	/*
	 * 충돌 처리 로직
	 * 
	 * 1. 모든 Hit에 대해 반복:
	 *    - 충돌 지점과 법선 벡터 추출
	 *    - 액터 타입에 따라 임펄스 강도 결정 (플레이어: 300, 슬라임: 150)
	 */
	for (const FHitResult& Hit : Hits)
	{
		FVector ContactPoint = Hit.ImpactPoint;
		FVector ContactNormal = Hit.ImpactNormal;
		
		if (Hit.GetActor()->IsA<APawn>())
		{
			bImpulseHit = true;
			ImpulseContactPoint = ContactPoint;
			ImpulseContactNormal = ContactNormal;
			ImpulseStrength = 300.0f;
		}
		else if (Hit.GetActor()->IsA<AASlimeActor>())
		{
			bImpulseHit = true;
			ImpulseContactPoint = ContactPoint;
			ImpulseContactNormal = ContactNormal;
			ImpulseStrength = 150.0f;
		}
		
		/*
		* 2. 각 파티클에 대해:
		*    - 파티클이 충돌면 안쪽에 있는지 검사 (d = (p - cp) · n < 0)
		*    - XPBD로 위치 보정: Δλ = -(d + α·λ) / (w + α)
		*    - 마찰 적용: 접선 방향 이동량에 마찰 계수를 곱해 감쇠
		*    move = p - p_prev
		*    tan = move - (move · n) * n
		*    move -= tan * friction
		*    wPos = prevPos + move
		*/
		for (FSlimeParticle& P : Particles)
		{
			FVector WorldPos = ActorTransform.TransformPosition(P.Position);
			float d = FVector::DotProduct(WorldPos - ContactPoint, ContactNormal);

			if (d < 0.0f)
			{
				float InvMass = 1.0f / P.Mass;
				float Compliance = CollisionCompliance;
				float Alpha = Compliance / (DeltaTime * DeltaTime);
				float& Lambda = P.CollisionLambda;

				float DeltaLambda = -(d + Alpha * Lambda) / (InvMass + Alpha);
				Lambda += DeltaLambda;

				WorldPos += InvMass * DeltaLambda * ContactNormal;
				
				FVector PrevWorldPos = ActorTransform.TransformPosition(P.PrevPosition);
				FVector Move = WorldPos - PrevWorldPos;
				FVector TangentMove = Move - FVector::DotProduct(Move, ContactNormal) * ContactNormal;
				Move -= TangentMove * Friction;
				WorldPos = PrevWorldPos + Move;
				
				P.Position = ActorTransform.InverseTransformPosition(WorldPos);
			}
		}
	}

	/*
	 * 임펄스 적용 (충돌한 액터가 플레이어나 다른 슬라임인 경우)
	 * - 충돌 지점으로부터의 거리 기반 가중치 계산
	 *	distance = |p - cp|
	 *	weight = clamp(1 - distance / radius, 0, 1)
	 *	
	 * - 가중치가 적용된 임펄스를 파티클에 추가
	 * impulse = n * strength * weight * dt
	 */
	if (bImpulseHit)
	{
		for (FSlimeParticle& P : Particles)
		{
			FVector WorldPos = ActorTransform.TransformPosition(P.Position);
			
			float Dist = FVector::Distance(WorldPos, ImpulseContactPoint);
			float Weight = FMath::Clamp(1.0f - Dist / SphereRadius, 0.0f, 1.0f);
			FVector Impulse = ImpulseContactNormal * ImpulseStrength * Weight * DeltaTime;

			WorldPos += Impulse; 
			
			P.Position = ActorTransform.InverseTransformPosition(WorldPos);
		}
	}
	
	// Overlapping 된 액터 개수 출력
	// UE_LOG(LogTemp, Warning, TEXT("Overlapping Actors: %d"), OverlappingActors.Num());
}

// Sphere Collision의 중심 위치 계산 함수
FVector AASlimeActor::ComputeParticleCenter()
{
	if (Particles.Num() == 0)
		return FVector::ZeroVector;

	FVector Sum = FVector::ZeroVector;

	for (const FSlimeParticle& P : Particles)
	{
		Sum += P.Position;
	}

	return Sum / Particles.Num();
}

// 부피 계산 함수
float AASlimeActor::ComputeVolume()
{
	float Volume = 0.0f;

	for (const FTriangle& Tri : Triangles)
	{
		const FVector& A = Particles[Tri.A].Position;
		const FVector& B = Particles[Tri.B].Position;
		const FVector& C = Particles[Tri.C].Position;

		Volume += FVector::DotProduct(A, FVector::CrossProduct(B, C)) / 6.0f;
	}

	return Volume;
}

// 삼각형의 부피 그래디언트 계산 함수
void AASlimeActor::ComputeTriangleGradients(const FVector& A, const FVector& B, const FVector& C, FVector& GradA,
	FVector& GradB, FVector& GradC)
{
	GradA = FVector::CrossProduct(B, C) / 6.0f;
	GradB = FVector::CrossProduct(C, A) / 6.0f;
	GradC = FVector::CrossProduct(A, B) / 6.0f;
}

void AASlimeActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	OverlappingActors.Add(OtherActor);
}

void AASlimeActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}
	
	OverlappingActors.Remove(OtherActor);
}
