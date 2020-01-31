// Copyright © 2019 Created by Brian Faubion

#pragma once

#include "HeightMap.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Terrain.generated.h"

struct ComponentData
{
	void Allocate(int32 Size);

	TArray<FVector> Vertices;
	TArray<FVector2D> UV0;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<int32> Triangles;
};

UCLASS()
class DYNAMICTERRAIN_API ATerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATerrain();

	/// Engine Functions ///

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/// Accessor Functions ///

	// Set the materials for the terrain
	UFUNCTION(BlueprintCallable)
		void SetMaterials(UMaterial* terrain_material = nullptr, UMaterial* border_material = nullptr);

	// Rebuild the terrain completely
	UFUNCTION(BlueprintCallable)
		void RebuildAll();

	UFUNCTION(BlueprintPure)
		UHeightMap* GetMap() const;
	UFUNCTION(BlueprintPure)
		int32 GetComponentSize() const;
	UFUNCTION(BlueprintPure)
		int32 GetXWidth() const;
	UFUNCTION(BlueprintPure)
		int32 GetYWidth() const;

	/// Generator Functions ///

	// Flatten the heightmap
	UFUNCTION(BlueprintCallable)
		void GenerateFlat();
	// Create a sloping heightmap
	UFUNCTION(BlueprintCallable)
		void GenerateSlope(float height);
	// Create a heightmap from plasma noise
	UFUNCTION(BlueprintCallable)
		void GeneratePlasma(int32 scale);
	// Create a heightmap from perlin noise
	UFUNCTION(BlueprintCallable)
		void GeneratePerlin(int32 frequency, int32 octaves, float persistence);

	// Create a mesh from a subsection of the heightmap
	void GenerateMeshSection(int32 X, int32 Y, ComponentData& Data, bool CreateTriangles) const;

protected:
	/// Map Rebuilding ///

	// Create a blank heightmap big enough to accomodate every component
	void RebuildHeightmap();
	// Rebuild or reload the procedural mesh component
	void RebuildMesh();

	// Refresh the materials on component meshes
	void ApplyMaterials();

	/// Properties ///

	// The procedural meshes used for the terrain mesh
	UPROPERTY(EditAnywhere)
		TArray<UProceduralMeshComponent*> Meshes;

	// Set to true when a corresponding section in Meshes needs to update
	UPROPERTY(EditAnywhere)
		TArray<bool> UpdateSection;

	// The material to apply to the terrain
	UPROPERTY(EditAnywhere)
		UMaterial* TerrainMaterial = nullptr;
	// The material for the outer border
	UPROPERTY(EditAnywhere)
		UMaterial* BorderMaterial = nullptr;

	// If set to true the terrain will automatically rebuild itself when altered in the editor
	UPROPERTY(EditAnywhere)
		bool AutoRebuild = false;

	// Set to true to create borders on the map mesh
	UPROPERTY(EditAnywhere)
		bool Border = true;

	// The number of vertices per component section
	UPROPERTY(EditAnywhere)
		int32 ComponentSize = 64;
	// The number of component sections in the mesh
	UPROPERTY(EditAnywhere)
		int32 XWidth = 1;
	UPROPERTY(EditAnywhere)
		int32 YWidth = 1;
	// The maximum height of the heightmap measured from the lowest point to the highest
	UPROPERTY(EditAnywhere)
		float Height = 512.0f;

	// The number of threads to use when generating a new map
	UPROPERTY(EditAnywhere)
		int32 WorkerThreads = 4;

	// Set to true when the map mesh needs to be rebuilt
	UPROPERTY(VisibleAnywhere)
		bool DirtyMesh = true;

	// The heightmap used to store terrain data
	UPROPERTY(VisibleAnywhere)
		UHeightMap* Map;

	// A buffer to store data for generated components
	ComponentData ComponentBuffer;
};
