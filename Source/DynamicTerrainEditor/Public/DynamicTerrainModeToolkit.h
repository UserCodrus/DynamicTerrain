#pragma once

#include "Tools.h"

#include "Toolkits/BaseToolkit.h"
#include "IDetailsView.h"

class FDynamicTerrainModeToolkit : public FModeToolkit
{
public:
	/// Engine Functions ///

	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override;

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override;

	/// Command List Delegates ///

	void ChangeMode(TerrainModeID ModeID);
	bool IsModeEnabled(TerrainModeID ModeID);
	bool IsModeActive(TerrainModeID ModeID);

	void ChangeTool(TerrainToolID ToolID);
	bool IsToolEnabled(TerrainToolID ToolID);
	bool IsToolActive(TerrainToolID ToolID);

	void ChangeBrush(TerrainBrushID BrushID);
	bool IsBrushEnabled(TerrainBrushID BrushID);
	bool IsBrushActive(TerrainBrushID BrushID);

	void RefreshDetails();

protected:
	TSharedPtr<IDetailsView> DetailsPanel;
	TSharedPtr<SWidget> ToolkitWidget;
};