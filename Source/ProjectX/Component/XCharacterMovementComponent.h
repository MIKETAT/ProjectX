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

UCLASS()
class PROJECTX_API UXCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	void SetWallNormal(FVector NewWallNormal);
	void SetHangInput(float Input);
	virtual void InitializeComponent() override;
protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;
	void PhysHanging(float deltaTime, int32 Iterations);
// Variable
private:
	UPROPERTY()
	float HangMoveInput = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector WallNormal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb", meta = (AllowPrivateAccess = "true"))
	float HangMoveSpeed = 25.f;
};
