#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Block.generated.h"

UCLASS()
class AI_PROJECT_API UBTTask_Block : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Block();

    // This is the equivalent of "Receive Execute AI" in Blueprints
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};