#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/TimerHandle.h"
#include <Interfaces/GTWBossAIInterface.h>
#include "GTWBossPhase1Character.generated.h"

class ATriggerBox;
class UBoxComponent;
class UAnimMontage;
class AGTWBossHitObject;
class UGTWP1TornadoPresentationWidget;
class UGTWP1BossHitWidget;
class AGTWP1TornadoJumpPad;
struct FCollisionQueryParams;

DECLARE_MULTICAST_DELEGATE(FOnObjectHitDelegate);

/**
 * P1 보스의 실제 이동, 범위 제한, 공격 생성과 전투 종료 정리를 담당
 * BP Service 추격 요청, BT Task 공격 요청.
 */
UCLASS()
class GTW_API AGTWBossPhase1Character : public ACharacter, public IGTWBossAIInterface
{
	GENERATED_BODY()

public:
	AGTWBossPhase1Character();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Boss|Difficulty")
	FORCEINLINE int32 GetRequiredSubmissionCount() const { return RequiredSubmissionCount; }
	/** HUD 표시용 적중 횟수. 페이즈 전환은 기존 GameMode가 판단한다. */
	UFUNCTION(BlueprintPure, Category = "Boss|HitReaction")
	FORCEINLINE int32 GetDisplayedHitCount() const
	{
		return DisplayedHitCount;
	}
	UFUNCTION(BlueprintPure, Category = "Boss|HitReaction")
	FORCEINLINE bool IsReactingToHit() const
	{
		return bReactingToHit;
	}

	// Begin IGTWBossAIInterface: GameMode가 보스 행동 시작/종료를 컨트롤하기 위한 Interface
	virtual void StartCombat() override;
	virtual void StopCombat() override;
	// End IGTWBossAIInterface

	FOnObjectHitDelegate OnObjectHit;

protected:
	/** 인트로/BeginPlay 전에 4층 기록을 읽고 이번 레벨의 장애물 생성량을 결정한다. */
	virtual void PostInitializeComponents() override;

	/** 레벨 종료나 보스 제거 시에도 전투 정리가 실행. */
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
public:
	/** BT Task의 공격 요청 진입점. DropAttack 정상 종료 후 기믹을 생성한다.
	 *  몽타주가 없거나 재생에 실패하면 기믹을 즉시 생성한다. */
	UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
	void PlayDropObstacles(APawn* TargetPlayer);

	/** 점프 → 공중 킥 → 착지를 재생한다. BT Task는 모션 종료까지 기다린다. */
	UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
	bool PlayTornadoKick();

	UFUNCTION(BlueprintPure, Category = "Boss|Attack")
	FORCEINLINE bool IsPerformingTornadoKick() const
	{
		return bTornadoKickPlaying;
	}

	/** Notify State만 판정을 켜고 갱신한다. 종료/중단 시에는 항상 판정을 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Boss|TornadoKick")
	void BeginTornadoKickDamage();
	UFUNCTION(BlueprintCallable, Category = "Boss|TornadoKick")
	void UpdateTornadoKickDamage();
	UFUNCTION(BlueprintCallable, Category = "Boss|TornadoKick")
	void EndTornadoKickDamage();
	void CancelTornadoKick();

	/** 준비 중인 몽타주가 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Boss|Attack")
	FORCEINLINE bool IsPreparingAttack() const
	{
		return PreparedMontage != nullptr;
	}

	/** BP Service가 전달한 목표로 이동하고 플레이어 쪽으로 회전 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Movement")
	void UpdatePursuit(APawn* TargetPlayer, const FVector& ChaseLocation, float DeltaSeconds);

	/** GameMode BP에서 호출한다. 범위/컨트롤러 검증 → 이동 준비 → BT 실행 순서다. */
	UFUNCTION(BlueprintCallable, Category = "Boss|Encounter")
	void StartEncounter();

	/** 현재 전투가 진행 중인지 BP와 AIController에서 조회 */
	UFUNCTION(BlueprintPure, Category = "Boss|Encounter")
	FORCEINLINE bool IsEncounterRunning() const
	{
		return bRunning;
	}

	/** 플레이어 위치를 사각형 내부의 추격 목표로 변환 */
	UFUNCTION(BlueprintPure, Category = "Boss|Movement")
	FVector GetChaseLocation(const FVector& PlayerPosition) const;

	/** AI와 준비 모션을 중지하고 생성한 기믹을 제거한 뒤 전투 전 이동 모드를 복구 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Encounter")
	void StopEncounter();

public:
	/** 레벨에 배치한 고정 Trigger Box. 위치/회전/Box Extent로 사각 이동 범위를 편집 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Boss|Movement")
	TObjectPtr<ATriggerBox> MovementBoundsActor;

	/** 사각형 내부 목표로 접근하는 속도. 기본 언리얼 단위 기준 cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Movement", meta = (ClampMin = "0"))
	float ChaseSpeed = 250.f;

	/** 플레이어와의 수평 거리가 이 값 이하면 이동을 멈춤 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Movement", meta = (ClampMin = "0"))
	float StopDistance = 100.f;

	/** 개발자용 노란 원으로 표시하는 플레이어 주변 생성 반경. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "1"))
	float PlayerRadius = 600.f;

	/** BP Service가 BB에 전달하고 BT의 Wait가 사용하는 공격 요청 사이의 대기 시간(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "0.1"))
	float DropInterval = 3.f;

	/** 플레이어 발밑을 기준으로 기믹을 생성할 바닥을 찾는 깊이(cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "1"))
	float GroundSearchDepth = 500.f;

	/** 기믹 생성 Transform에 적용할 균일 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "0.1"))
	float GroundAttackScale = 1.f;

	/** 개발자용 이동 범위(하늘색 Box)와 플레이어 생성 반경(노란 원) 표시 여부. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Debug")
	bool bDrawDebug = true;

	/** BP_P1Boss에서 선택하는 기믹 생성 준비 몽타주 1개.
	 *  다른 몽타주를 폴더에 추가하는 것만으로는 공격 종류가 늘어나지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Attack")
	TObjectPtr<UAnimMontage> DropAttack;

	/** P1 전용 몽타주. 킥 구간의 P1TornadoKick Notify State로만 타격한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|TornadoKick")
	TObjectPtr<UAnimMontage> TornadoKick;

	/** 말풍선/바람 연출만 담당하는 P1 위젯. BP 자식으로 문구와 색상을 바꿀 수 있다. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|TornadoKick")
	TSubclassOf<UGTWP1TornadoPresentationWidget> TornadoPresentationClass;

	/** 피격 모션과 슬롯은 AM_P1HitReaction 에셋에서 편집한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|HitReaction")
	TObjectPtr<UAnimMontage> HitReaction;
	UPROPERTY(EditDefaultsOnly, Category = "Boss|HitReaction", meta = (ClampMin = "0.1", Units = "s"))
	float HitReactionDuration = 2.f;

	/** HUD 모양과 표시 그래프는 이 위젯 BP에서 편집한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|HitReaction")
	TSubclassOf<UGTWP1BossHitWidget> HitStatusWidgetClass;

	/** 점프/착지에 부드럽게 적용하는 추가 높이(월드 cm). 기본 메시 위치는 종료 시 복구한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|TornadoKick", meta = (ClampMin = "0", Units = "cm"))
	float TornadoKickExtraHeight = 600.f;

	/** INI의 AirKick 재생 배율. 작을수록 느리며 점프/착지는 원래 속도를 유지한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|TornadoKick")
	float TornadoKickPlayRate = 1.f;

	/** 다리 판정의 월드 반경(cm). 보스의 액터 스케일을 다시 곱하지 않는다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|TornadoKick", meta = (ClampMin = "1", Units = "cm"))
	float TornadoKickRadius = 100.f;

	/** INI의 넉백 강도. 기존 플레이어 인터페이스에서 무적 여부를 검사한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|TornadoKick")
	float TornadoKickPower = 0.f;

	/** BossPhase1.ini의 킥 차례 실행 확률(0~1). 선택하지 않은 차례는 기존 Wait로 넘어간다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|TornadoKick")
	float TornadoKickChance = 1.f;

	/** 2층 점프 발판을 상속한 P1 BP. 점프/사운드는 원본, 제자리 뜀 보정은 자식 BP가 담당한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|TornadoKick|Escape")
	TSubclassOf<AGTWP1TornadoJumpPad> TornadoEscapePadClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|TornadoKick|Escape")
	int32 TornadoEscapePadCount = 0;

	/** INI의 랜덤 배치 최대 반경(cm). 플레이어 공격 반경보다 바깥에는 생성하지 않는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|TornadoKick|Escape", meta = (Units = "cm"))
	float TornadoEscapePadDistance = 0.f;

	/** INI에서 읽은 점프 강도. P1 발판이 부모의 JumpPower에 적용한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|TornadoKick|Escape")
	float TornadoEscapePadJumpPower = 0.f;

	/** 현재 SK_Boss의 무릎/발 뼈. 다른 메시를 쓰면 BP에서 해당 뼈 이름으로 바꾼다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|TornadoKick")
	FName LeftKickKnee = TEXT("LeftLeg");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|TornadoKick")
	FName LeftKickFoot = TEXT("LeftFoot");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|TornadoKick")
	FName RightKickKnee = TEXT("RightLeg");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|TornadoKick")
	FName RightKickFoot = TEXT("RightFoot");

	/** 생성할 기믹 BP 목록. 에셋 선택은 BP_P1Boss에서만 관리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	TArray<TSubclassOf<AActor>> AttackActorClasses;

	/** 한 공격에서 순차적으로 생성할 기믹 개수. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "1", ClampMax = "32"))
	int32 AttackActorCount = 3;

	/** INI에서 읽은 시간 기준. 수치 편집은 Saved/Config/BossPhase1.ini에서만 한다. 0이면 보정하지 않는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|Difficulty", meta = (Units = "s"))
	float FastClearTimeThreshold = 0.f;

	/** INI에서 읽은 제출 목표 감소 비율(0~1). 0.25이면 1/4 감소한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|Difficulty")
	float FastClearSubmissionReduction = 0.f;

	/** 적용에 사용한 4층 슈팅 클리어 시간. -1은 기록 없음이며 기본 제출 목표를 사용한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|Difficulty")
	double FourthFloorClearTime = -1.0;

	/** 이번 전투의 실제 QR 제출 목표. GameMode·HUD·인트로가 이 값을 함께 사용한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|Difficulty")
	int32 RequiredSubmissionCount = 1;

	/** INI에서 읽은 혜택 적용 전 제출 목표. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|Difficulty")
	int32 BaseRequiredSubmissionCount = 1;

	/** 인트로 BP는 이 판별 결과만 읽는다. 시간 기준 비교를 대사 그래프에 중복 작성하지 않는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Boss|Difficulty")
	bool bHasFastClearBenefit = false;

	/** 장애물이 한 프레임에 몰리지 않도록 생성 사이에 두는 간격(초). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "0.05", Units = "s"))
	float AttackSpawnInterval = 0.35f;

	/** 자체 수명이 없는 기믹에만 적용하는 생성 후 삭제 시간(초). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack", meta = (ClampMin = "0.1", Units = "s"))
	float AttackActorLifeSpan = 12.f;

public:
	/** 플레이어가 접촉하면 보스에게 날아가는 타격 BP. BP_P1Boss에서 지정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|HitObject")
	TSubclassOf<AGTWBossHitObject> HitObjectClass;

	/** 전투 시작 시 1개를 생성하고, 이후 이 간격으로 부족한 타격 액터를 보충한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitObject", meta = (ClampMin = "0.5", Units = "s"))
	float HitObjectSpawnInterval = 5.f;

	/** 대기 중이거나 보스로 날아가는 타격 액터의 최대 개수. 클리어 횟수는 GameMode가 관리한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitObject", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxHitObjectCount = 3;

	/** 줍지 않은 액터도 누적되지 않도록 제거하는 시간. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|HitObject", meta = (ClampMin = "5", Units = "s"))
	float HitObjectLifeSpan = 30.f;

private:
	/** P1에서 생성하는 타격 액터에만 적용한다. 원본 기믹/BP 머티리얼은 변경하지 않는다. */
	UPROPERTY()
	TObjectPtr<class UMaterialInterface> HitObjectMaterial;
	/** 메시 없이 Niagara만 사용하는 타격 BP의 QR 표시용 기본 도형. */
	UPROPERTY()
	TObjectPtr<class UStaticMesh> HitObjectDisplayMesh;

	/** 패키지 기본 INI로 외부 설정을 처음 만들고, 이후에는 외부 파일만 읽는다. */
	void LoadBossSettings();
	void SpawnTornadoEscapePads();
	void ClearTornadoEscapePads();
	TArray<TWeakObjectPtr<AActor>> TornadoEscapePads;

	UPROPERTY(Transient)
	TObjectPtr<UGTWP1BossHitWidget> HitWidget;
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> HitReactionMontage;
	int32 DisplayedHitCount = 0;
	int32 PendingHitNotifications = 0;
	bool bReactingToHit = false;
	FTimerHandle HitReactionTimer;
	FDelegateHandle KickPoseFinalizedHandle;
	void FinishHitReaction();
	/** 공격 종료 시 보스가 직접 제거한다. 화면 밖에서도 연출이 남지 않는다. */
	UPROPERTY(Transient)
	TObjectPtr<UGTWP1TornadoPresentationWidget> TornadoPresentation;

	/** 기존 바닥 검색을 재사용해 플레이어 주변 발판에 타격 액터를 생성한다. */
	void SpawnHitObject();

	/** 실제 도착 알림만 GameMode의 기존 적중 카운트로 전달한다. */
	void OnHitObjectReachedBoss(AGTWBossHitObject* HitObject);

	FTimerHandle HitObjectSpawnTimer;
	TArray<TWeakObjectPtr<AGTWBossHitObject>> SpawnedHitObjects;

private:
	/** 위치를 캡슐 크기만큼 안쪽으로 줄인 사각형 내부로 제한하고 시작 높이를 유지. */
	FVector ConstrainPosition(const FVector& Position) const;

	/** 몽타주를 재생하고 종료 콜백을 연결 */
	bool PlayAttackAnim(UAnimMontage* Montage, APawn* TargetPlayer);

	/** 준비한 몽타주가 정상 종료했을 때만 기믹 생성 */
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 대기 중인 공격 정보를 비우고 콜백을 해제한 뒤 에셋의 블렌드 시간으로 모션을 중지 */
	void CancelPreparedAttack();

	void UpdateTornadoKickMotion();
	void ResetTornadoKickMotion();

	/** 추격 요청 유무와 관계없이 전투 중 현재 위치가 범위 밖이면 보정 */
	void EnforceArenaBounds();

	/** Tick에서 개발자용 사각 범위와 플레이어 주변 생성 반경을 표시한다. */
	void DrawArenaDebug() const;

	/** Box의 반크기에서 보스 캡슐 반지름을 제외한 로컬 이동 가능 범위 계산 */
	FVector GetMovementExtent() const;

	/** 에디터에서 연결한 Trigger Box의 BoxComponent를 읽음 */
	UBoxComponent* GetMovementBounds() const;

	/** 플레이어 주변의 바닥 위치에 랜덤 기믹을 생성하고 종료 시 정리할 목록에 기록 */
	void SpawnAttackActors(APawn* TargetPlayer);
	void SpawnNextAttackActor();
	FTimerHandle AttackSpawnTimer;
	TWeakObjectPtr<APawn> AttackSpawnTarget;
	int32 RemainingAttackActors = 0;

	/** BP에서 지정한 클래스 목록 중 비어 있거나 추상 클래스인 항목을 제외 */
	TArray<TSubclassOf<AActor>> GetAttackClasses() const;

	/** 반경 안의 후보 한 곳을 바닥 검사한다. 성공 시 OutLocation에 생성 위치를 기록 */
	bool FindAttackSpawnLocation(APawn* TargetPlayer, const FCollisionQueryParams& GroundQuery, FVector& OutLocation) const;
	bool FindGroundLocation(const FVector& CandidateLocation, APawn* TargetPlayer, const FCollisionQueryParams& GroundQuery, FVector& OutLocation) const;

private:
	/** 준비 모션 시작 당시의 공격 대상. */
	TWeakObjectPtr<APawn> PreparedTarget;

	/** 현재 준비 중인 몽타주 */
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> PreparedMontage;

	bool bTornadoKickPlaying = false;
	bool bTornadoDamageActive = false;
	bool bTornadoPlayerHit = false;
	FVector TornadoMeshBaseLocation = FVector::ZeroVector;
	/** 왼쪽 무릎·발, 오른쪽 무릎·발, 왼쪽·오른쪽 허벅지의 이전 프레임 위치. */
	FVector PreviousKickBones[6];
	void SweepTornadoKickLeg(const FVector& PreviousStart, const FVector& PreviousEnd, const FVector& Start, const FVector& End);
	void TraceTornadoKick(const FVector& Start, const FVector& End);

	/** 중복 시작과 종료 후 공격을 막기 위한 전투 진행 상태. */
	bool bRunning = false;

	/** 전투 시작 시 저장한 보스의 월드 Z 좌표. 수평 이동 중 유지할 높이다. */
	float BossHeight = 0.f;

	/** 전투 종료 시 복구할 CharacterMovement의 이전 이동 모드. */
	uint8 PreviousMovementMode = 0;

	/** 이전 모드가 커스텀 이동인 경우 함께 복구할 세부 모드 값. */
	uint8 PreviousCustomMode = 0;

	/** 보스가 생성한 기믹들의 약한 참조 목록. */
	TArray<TWeakObjectPtr<AActor>> SpawnedAttackActors;

};
