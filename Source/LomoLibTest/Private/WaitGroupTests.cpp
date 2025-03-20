
#include "LomoLibTest.h"
#include "LomoWaitGroup.h"
#include "Misc/AutomationTest.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	// UE5 specific flags
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
FWaitGroupTest,
"LomoLib.WaitGroup",  // 测试路径，方便分类和过滤
EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
);
#else
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaitGroupTest,
	"LomoLib.WaitGroup", // 测试路径，方便分类和过滤
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
);
#endif


bool FWaitGroupTest::RunTest(const FString& Parameters)
{
	TSharedRef<FLomoWaitGroup> WG = MakeShared<FLomoWaitGroup>(1);
	WG->Add();
	UE_LOG(LogLomoLibTests, Warning, TEXT("WG Add"));
	// 使用 Ticker 实现延迟
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda(
			[WG](float DeltaTime) -> bool
			{
				UE_LOG(LogLomoLibTests, Warning, TEXT("WG Done"));
				WG->Done();
				return false; // 返回 false 表示只执行一次
			}
		), 1.0f // 1秒后执行
	);

	WG->Next([]()
	{
		UE_LOG(LogLomoLibTests, Warning, TEXT("WG Complete"));
	});

	return true;
}