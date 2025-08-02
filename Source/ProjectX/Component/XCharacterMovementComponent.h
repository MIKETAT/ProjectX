#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ECustomMoveMode : uint8
{
	CMove_None UMETA(DisplyName = "None"),
	CMove_Hanging UMETA(DisplayName = "Hanging"),
	CMove_Default UMETA(DisplayName = "Default")
};

class FSavedMove_HangingMovement : public FSavedMove_Character
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

	// flags for activiting hanging
	uint8 bSavedWantsToHanging : 1;
};

class FNetworkPredictionData_Client_Character_HangingMovement : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_Character_HangingMovement(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class PROJECTX_API UXCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	friend FSavedMove_HangingMovement;

	UXCharacterMovementComponent(const FObjectInitializer& ObjInit);
	
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
	void SetWallNormal(FVector NewWallNormal);
	void SetHangInput(float Input);
	virtual void InitializeComponent() override;

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;
	void PhysHanging(float deltaTime, int32 Iterations);

// Variable
protected:
	UPROPERTY(Transient)
	uint8 bWantsToHanging : 1;

	UPROPERTY()
	float HangMoveInput = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true") )
	FVector WallNormal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb", meta = (AllowPrivateAccess = "true"))
	float HangMoveSpeed = 25.f;
};
