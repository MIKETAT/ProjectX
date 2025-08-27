#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XCharacterMovementComponent.generated.h"


struct FRootMotionSource_Hang;
class UClimbSettings;
class ACharacterBase;

UENUM(BlueprintType)
enum class ECustomMoveMode : uint8
{
	CMove_None UMETA(DisplyName = "None"),
	CMove_Hanging UMETA(DisplayName = "Hanging"),
	CMove_Default UMETA(DisplayName = "Default")
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHangStateChanged, bool, bNewState);

UCLASS()
class PROJECTX_API UXCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	// Inner Classes
	
	class FXSavedMove : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		// reset saved variables
		virtual void Clear() override;

		// store input command in the compressed flags
		virtual uint8 GetCompressedFlags() const override;

		// check to make sure the saved vairables are the same
		// 返回能否合并(NewMove与当前是否一致)
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		// set up move before send to server
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		// set variables on character movement component before making a predictive correction
		virtual void PrepMoveFor(ACharacter* C) override;

		// Flags need to be replicated
		uint8 bSavedWantsToRun : 1;
		uint8 bSavedWantsToSprint : 1;
		uint8 bSavedWantsToHanging : 1;
		uint8 bSavedPressXJump : 1;

		// Other Variables to Replay SaveMove
		uint8 bSavedTransitionFinished : 1;
	};

	class FXNetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
	{
	public:
		typedef FNetworkPredictionData_Client_Character Super;

		FXNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	friend FXSavedMove;

	UXCharacterMovementComponent(const FObjectInitializer& ObjInit);
	
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
	void SetRunning(bool NewValue) { bWantsToRun = NewValue; }
	void SetSprinting(bool NewValue) { bWantsToSprint = NewValue; }

	virtual bool IsCrouching() const override;
	bool HangTracer();
	bool CanHang(FHitResult& OutFrontHit, FHitResult& OutDownHit, FVector Offset = FVector::ZeroVector);
	
	FVector GetMantleStartLocation(FHitResult& FrontHit, FHitResult& DownHit) const;
	
	void SetHangInput(float Input);
	virtual void InitializeComponent() override;
	bool IsMovementMode(EMovementMode InMoveMode) const;
	bool IsCustomMovementMode(ECustomMoveMode InCustomMoveMode) const;

	// Property Access
	UFUNCTION(BlueprintPure, Category="Character Movement")
	bool GetWantsToHanging() const { return bWantsToHanging; }

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	void SetWantsToHanging(bool NewValue);
	
	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnHangStateChanged OnHangStateChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float MaxRunSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float MaxSprintSpeed = 700.f;

	UPROPERTY()
	FVector HandRelativeToCapsule{30.f, 2.f, 64.f};

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;
	virtual void PhysHanging(float deltaTime, int32 Iterations);
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMinAnalogSpeed() const override;
	
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual void MaintainHorizontalHangingVelocity();
private:
	// Helper Functions
	bool IsServer() const;
	float CapRadius() const;
	float CapHalfHeight() const;
// Variable
public:
	// Flags
	UPROPERTY(Category="Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 bWantsToHanging : 1;
	
	uint8 bWantsToRun : 1;
	uint8 bWantsToSprint : 1;
	// Other Variables
	uint8 bWantsTransitionFinished : 1;
	
protected:
	/*// Settings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UClimbSettings> ClimbSettings;*/
	
	UPROPERTY()
	TObjectPtr<ACharacterBase> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> TransitionMontage;
	
	TSharedPtr<FRootMotionSource_Hang> TransitionRMS;
	int32 TransitionRMS_ID;
	FName TransitionRMS_Name;

	UPROPERTY(EditAnywhere, Category = "Setting")
	TObjectPtr<UAnimMontage> Montage_BracedHang;

	UPROPERTY()
	bool bIngorePhysRotationDuringRMS{false};
	
	UPROPERTY()
	float HangMoveInput = 0.f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true") )
	FVector WallRightDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb", meta = (AllowPrivateAccess = "true"))
	float HangFriction{8.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb", meta = (AllowPrivateAccess = "true"))
	float HangBraking{2000.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb", meta = (AllowPrivateAccess = "true"))
	float MaxHangSpeed{200.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb", meta = (AllowPrivateAccess = "true"))
	float MaxHangAcceleration{2048.f};
};
