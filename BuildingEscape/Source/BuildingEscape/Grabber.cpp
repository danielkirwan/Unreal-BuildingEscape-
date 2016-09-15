// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingEscape.h"
#include "Grabber.h"

#define OUT
// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

///Look for attched handle
void UGrabber::FindPhysicsHandleComponent() {
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("No component found on %s"), *GetOwner()->GetName());		
	}
}

/// Lok for attache input component (Only appears at run time)
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent) {
		UE_LOG(LogTemp, Warning, TEXT("component found on %s"), *GetOwner()->GetName());
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("No input component found on %s"), *GetOwner()->GetName());
	}
}

void UGrabber::Grab() {
	///Line trace and see if we reach any actors with physics body collison channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();// point of object to grab
	auto ActorHit = HitResult.GetActor();

	///If we hit something then attach a physics handle
	if (ActorHit) {
	// attach physics handle
		PhysicsHandle->GrabComponent(
			ComponentToGrab, 
			NAME_None, //no bones needed
			ComponentToGrab->GetOwner()->GetActorLocation(),
			true); // allow rotation
	}
	
}

void UGrabber::Release() {
	//TODO release physics handle
	PhysicsHandle->ReleaseComponent();
}

/// Called every frame
void UGrabber::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	//if the physics handle is attached
	if (PhysicsHandle->GrabbedComponent) {
		//then move the object we're holding
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach()
{
	///Draw a red trace in the world to visualize the players sight
	/*DrawDebugLine(
	GetWorld(),
	PlayerViewpointLocation,
	LineTraceEnd,
	FColor(255, 0, 0),
	false,
	0.f,
	0,
	10.f
	);*/

	/// Ray-cast/Line trace out to reach distance
	FHitResult HitResult;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);

	///// see what we hit
	//AActor * ActorHit = Hit.GetActor();
	//if (ActorHit) {
	//	UE_LOG(LogTemp, Warning, TEXT("Line trace hit: %s"), *(ActorHit->GetName()));
	//}
	return HitResult;
}

FVector UGrabber::GetReachLineStart() {
	FVector PlayerViewpointLocation;
	FRotator PlayerViewpointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewpointLocation,
		OUT PlayerViewpointRotation);
	return PlayerViewpointLocation;
}

FVector UGrabber::GetReachLineEnd(){
	FVector PlayerViewpointLocation;
	FRotator PlayerViewpointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewpointLocation,
		OUT PlayerViewpointRotation);
	return PlayerViewpointLocation + PlayerViewpointRotation.Vector() * Reach;
}

