// Reforger Inventory Plus - Capacity Bar UI Component
// Renders color-coded weight/capacity bars with real-time updates
// Green (< 70%), Yellow (70-90%), Red (> 90%)

class RIP_CapacityBarUI : ScriptedWidgetComponent
{
	// Widget references - overall bar
	protected ImageWidget m_wOverallBarFill;
	protected ImageWidget m_wOverallBarBg;
	protected TextWidget m_wOverallWeightText;

	// Widget references - container for per-storage bars
	protected Widget m_wStorageBarsContainer;

	// State
	protected RIP_InventoryPlusComponent m_RIPComponent;
	protected ref array<ref RIP_StorageBarEntry> m_aStorageBars;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Find overall bar widgets
		m_wOverallBarFill = ImageWidget.Cast(w.FindAnyWidget("RIP_OverallBarFill"));
		m_wOverallBarBg = ImageWidget.Cast(w.FindAnyWidget("RIP_OverallBarBg"));
		m_wOverallWeightText = TextWidget.Cast(w.FindAnyWidget("RIP_OverallWeightText"));
		m_wStorageBarsContainer = w.FindAnyWidget("RIP_StorageBarsContainer");

		m_aStorageBars = {};

		// Subscribe to weight indicator updates
		m_RIPComponent = RIP_InventoryPlusComponent.GetInstance();
		if (m_RIPComponent)
		{
			RIP_WeightIndicatorManager weightManager = m_RIPComponent.GetWeightManager();
			if (weightManager)
				weightManager.m_OnIndicatorsUpdated.Insert(OnIndicatorsUpdated);
		}

		// Initial update
		OnIndicatorsUpdated();
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_RIPComponent)
		{
			RIP_WeightIndicatorManager weightManager = m_RIPComponent.GetWeightManager();
			if (weightManager)
				weightManager.m_OnIndicatorsUpdated.Remove(OnIndicatorsUpdated);
		}

		ClearStorageBars();
		super.HandlerDeattached(w);
	}

	// Callback when weight indicators update
	protected void OnIndicatorsUpdated()
	{
		if (!m_RIPComponent)
			return;

		RIP_WeightIndicatorManager weightManager = m_RIPComponent.GetWeightManager();
		if (!weightManager)
			return;

		// Update overall bar
		UpdateBar(m_wOverallBarFill, m_wOverallBarBg, weightManager.GetOverallRatio(), weightManager.GetOverallColor());

		// Update overall weight text
		if (m_wOverallWeightText)
			m_wOverallWeightText.SetText(weightManager.GetOverallDisplayString());

		// Update per-storage bars
		UpdateStorageBars(weightManager);
	}

	// Update a bar's fill and color
	protected void UpdateBar(ImageWidget fill, ImageWidget bg, float ratio, int color)
	{
		if (fill)
		{
			// Set the fill width as a proportion of the background width
			fill.SetColor(color);

			// Use size manipulation to represent fill amount
			float maxWidth = 1.0;  // Normalized, the layout handles actual size
			fill.SetSize(ratio * maxWidth, 1.0);
		}

		if (bg)
		{
			bg.SetColor(RIP_Constants.COLOR_CAPACITY_BG);
		}
	}

	// Update per-storage capacity bars
	protected void UpdateStorageBars(RIP_WeightIndicatorManager weightManager)
	{
		array<ref RIP_StorageInfo> storageInfos = weightManager.GetStorageInfos();

		// Ensure we have the right number of bar entries
		while (m_aStorageBars.Count() < storageInfos.Count())
		{
			RIP_StorageBarEntry entry = new RIP_StorageBarEntry();
			m_aStorageBars.Insert(entry);
		}

		// Update each storage bar
		for (int i = 0; i < storageInfos.Count(); i++)
		{
			RIP_StorageInfo info = storageInfos[i];
			RIP_StorageBarEntry entry = m_aStorageBars[i];

			entry.m_sName = info.m_sStorageName;
			entry.m_fRatio = Math.Max(info.m_fWeightRatio, info.m_fSlotRatio);
			entry.m_iColor = info.m_iBarColor;
			entry.m_sDisplayText = info.GetDisplayString();
		}
	}

	// Clear all dynamic storage bar widgets
	protected void ClearStorageBars()
	{
		m_aStorageBars.Clear();
	}

	// Get storage bar entries (for external rendering if needed)
	array<ref RIP_StorageBarEntry> GetStorageBars()
	{
		return m_aStorageBars;
	}
}

// Helper class for per-storage bar data
class RIP_StorageBarEntry
{
	string m_sName;
	float m_fRatio;
	int m_iColor;
	string m_sDisplayText;

	void RIP_StorageBarEntry()
	{
		m_sName = "";
		m_fRatio = 0;
		m_iColor = RIP_Constants.COLOR_CAPACITY_LOW;
		m_sDisplayText = "";
	}
}
