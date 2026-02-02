// Reforger Inventory Plus - Quick Transfer UI Component
// Category-based quick transfer buttons for moving items with one click
// Categories: Weapons, Ammo, Medical, Equipment, All

class RIP_QuickTransferUI : ScriptedWidgetComponent
{
	// Transfer direction buttons (To Inventory / To Vicinity)
	protected ButtonWidget m_wBtnPickUpWeapons;
	protected ButtonWidget m_wBtnPickUpAmmo;
	protected ButtonWidget m_wBtnPickUpMedical;
	protected ButtonWidget m_wBtnPickUpEquipment;
	protected ButtonWidget m_wBtnPickUpAll;

	protected ButtonWidget m_wBtnDropWeapons;
	protected ButtonWidget m_wBtnDropAmmo;
	protected ButtonWidget m_wBtnDropMedical;
	protected ButtonWidget m_wBtnDropEquipment;
	protected ButtonWidget m_wBtnDropAll;

	// Status display
	protected TextWidget m_wTransferStatus;
	protected Widget m_wProgressBar;
	protected ImageWidget m_wProgressBarFill;

	// Category count labels
	protected TextWidget m_wCountWeapons;
	protected TextWidget m_wCountAmmo;
	protected TextWidget m_wCountMedical;
	protected TextWidget m_wCountEquipment;

	// State
	protected RIP_InventoryPlusComponent m_RIPComponent;
	protected BaseInventoryStorageComponent m_TargetStorage;  // Current vicinity/container storage

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Find pick-up buttons (vicinity -> inventory)
		m_wBtnPickUpWeapons = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnPickUpWeapons"));
		m_wBtnPickUpAmmo = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnPickUpAmmo"));
		m_wBtnPickUpMedical = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnPickUpMedical"));
		m_wBtnPickUpEquipment = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnPickUpEquipment"));
		m_wBtnPickUpAll = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnPickUpAll"));

		// Find drop buttons (inventory -> vicinity)
		m_wBtnDropWeapons = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnDropWeapons"));
		m_wBtnDropAmmo = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnDropAmmo"));
		m_wBtnDropMedical = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnDropMedical"));
		m_wBtnDropEquipment = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnDropEquipment"));
		m_wBtnDropAll = ButtonWidget.Cast(w.FindAnyWidget("RIP_BtnDropAll"));

		// Status widgets
		m_wTransferStatus = TextWidget.Cast(w.FindAnyWidget("RIP_TransferStatus"));
		m_wProgressBar = w.FindAnyWidget("RIP_TransferProgress");
		m_wProgressBarFill = ImageWidget.Cast(w.FindAnyWidget("RIP_TransferProgressFill"));

		// Count labels
		m_wCountWeapons = TextWidget.Cast(w.FindAnyWidget("RIP_CountWeapons"));
		m_wCountAmmo = TextWidget.Cast(w.FindAnyWidget("RIP_CountAmmo"));
		m_wCountMedical = TextWidget.Cast(w.FindAnyWidget("RIP_CountMedical"));
		m_wCountEquipment = TextWidget.Cast(w.FindAnyWidget("RIP_CountEquipment"));

		// Get RIP component and subscribe to events
		m_RIPComponent = RIP_InventoryPlusComponent.GetInstance();
		if (m_RIPComponent)
		{
			RIP_CategoryTransferManager transferManager = m_RIPComponent.GetTransferManager();
			if (transferManager)
			{
				transferManager.m_OnTransferStarted.Insert(OnTransferStarted);
				transferManager.m_OnTransferProgress.Insert(OnTransferProgress);
				transferManager.m_OnTransferCompleted.Insert(OnTransferCompleted);
				transferManager.m_OnTransferFailed.Insert(OnTransferFailed);
			}
		}

		// Hide progress initially
		if (m_wProgressBar)
			m_wProgressBar.SetVisible(false);

		UpdateCategoryCounts();
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_RIPComponent)
		{
			RIP_CategoryTransferManager transferManager = m_RIPComponent.GetTransferManager();
			if (transferManager)
			{
				transferManager.m_OnTransferStarted.Remove(OnTransferStarted);
				transferManager.m_OnTransferProgress.Remove(OnTransferProgress);
				transferManager.m_OnTransferCompleted.Remove(OnTransferCompleted);
				transferManager.m_OnTransferFailed.Remove(OnTransferFailed);
			}
		}

		super.HandlerDeattached(w);
	}

	// Set the target storage for transfers (called when vicinity/container selection changes)
	void SetTargetStorage(BaseInventoryStorageComponent storage)
	{
		m_TargetStorage = storage;
		UpdateCategoryCounts();
	}

	// Button click handler
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_RIPComponent || !m_TargetStorage)
			return false;

		// Pick up buttons (vicinity -> player inventory)
		if (w == m_wBtnPickUpWeapons) { DoTransfer(RIP_EItemCategory.WEAPONS, RIP_ETransferDirection.TO_INVENTORY); return true; }
		if (w == m_wBtnPickUpAmmo) { DoTransfer(RIP_EItemCategory.AMMO, RIP_ETransferDirection.TO_INVENTORY); return true; }
		if (w == m_wBtnPickUpMedical) { DoTransfer(RIP_EItemCategory.MEDICAL, RIP_ETransferDirection.TO_INVENTORY); return true; }
		if (w == m_wBtnPickUpEquipment) { DoTransfer(RIP_EItemCategory.EQUIPMENT, RIP_ETransferDirection.TO_INVENTORY); return true; }
		if (w == m_wBtnPickUpAll) { DoTransfer(RIP_EItemCategory.ALL, RIP_ETransferDirection.TO_INVENTORY); return true; }

		// Drop buttons (player inventory -> vicinity)
		if (w == m_wBtnDropWeapons) { DoTransfer(RIP_EItemCategory.WEAPONS, RIP_ETransferDirection.TO_VICINITY); return true; }
		if (w == m_wBtnDropAmmo) { DoTransfer(RIP_EItemCategory.AMMO, RIP_ETransferDirection.TO_VICINITY); return true; }
		if (w == m_wBtnDropMedical) { DoTransfer(RIP_EItemCategory.MEDICAL, RIP_ETransferDirection.TO_VICINITY); return true; }
		if (w == m_wBtnDropEquipment) { DoTransfer(RIP_EItemCategory.EQUIPMENT, RIP_ETransferDirection.TO_VICINITY); return true; }
		if (w == m_wBtnDropAll) { DoTransfer(RIP_EItemCategory.ALL, RIP_ETransferDirection.TO_VICINITY); return true; }

		return false;
	}

	// Execute a transfer operation
	protected void DoTransfer(RIP_EItemCategory category, RIP_ETransferDirection direction)
	{
		if (!m_RIPComponent)
			return;

		bool success = m_RIPComponent.TransferCategory(category, direction, m_TargetStorage);

		if (!success)
		{
			SetStatus("Transfer failed - check inventory space");
		}
	}

	// Update category item counts in the UI
	protected void UpdateCategoryCounts()
	{
		if (!m_RIPComponent)
			return;

		RIP_VicinityManager vicinityManager = m_RIPComponent.GetVicinityManager();
		if (!vicinityManager)
			return;

		if (m_wCountWeapons)
			m_wCountWeapons.SetText(vicinityManager.GetCategoryCount(RIP_EItemCategory.WEAPONS).ToString());

		if (m_wCountAmmo)
			m_wCountAmmo.SetText(vicinityManager.GetCategoryCount(RIP_EItemCategory.AMMO).ToString());

		if (m_wCountMedical)
			m_wCountMedical.SetText(vicinityManager.GetCategoryCount(RIP_EItemCategory.MEDICAL).ToString());

		if (m_wCountEquipment)
			m_wCountEquipment.SetText(vicinityManager.GetCategoryCount(RIP_EItemCategory.EQUIPMENT).ToString());
	}

	// Transfer event callbacks

	protected void OnTransferStarted(RIP_EItemCategory category, int itemCount)
	{
		string categoryName = RIP_Constants.GetCategoryName(category);
		SetStatus(string.Format("Transferring %1 %2 items...", itemCount, categoryName));

		if (m_wProgressBar)
			m_wProgressBar.SetVisible(true);

		SetButtonsEnabled(false);
	}

	protected void OnTransferProgress(int transferred, int total)
	{
		if (m_wProgressBarFill && total > 0)
		{
			float ratio = transferred / total;
			m_wProgressBarFill.SetSize(ratio, 1.0);
		}

		SetStatus(string.Format("Transferring... %1/%2", transferred, total));
	}

	protected void OnTransferCompleted(int totalTransferred)
	{
		SetStatus(string.Format("Transferred %1 items", totalTransferred));

		if (m_wProgressBar)
			m_wProgressBar.SetVisible(false);

		SetButtonsEnabled(true);
		UpdateCategoryCounts();
	}

	protected void OnTransferFailed(string reason)
	{
		SetStatus(string.Format("Transfer failed: %1", reason));

		if (m_wProgressBar)
			m_wProgressBar.SetVisible(false);

		SetButtonsEnabled(true);
	}

	// Helper: Set status text
	protected void SetStatus(string text)
	{
		if (m_wTransferStatus)
			m_wTransferStatus.SetText(text);
	}

	// Helper: Enable/disable all transfer buttons during operation
	protected void SetButtonsEnabled(bool enabled)
	{
		SetButtonEnabled(m_wBtnPickUpWeapons, enabled);
		SetButtonEnabled(m_wBtnPickUpAmmo, enabled);
		SetButtonEnabled(m_wBtnPickUpMedical, enabled);
		SetButtonEnabled(m_wBtnPickUpEquipment, enabled);
		SetButtonEnabled(m_wBtnPickUpAll, enabled);
		SetButtonEnabled(m_wBtnDropWeapons, enabled);
		SetButtonEnabled(m_wBtnDropAmmo, enabled);
		SetButtonEnabled(m_wBtnDropMedical, enabled);
		SetButtonEnabled(m_wBtnDropEquipment, enabled);
		SetButtonEnabled(m_wBtnDropAll, enabled);
	}

	protected void SetButtonEnabled(ButtonWidget btn, bool enabled)
	{
		if (btn)
			btn.SetEnabled(enabled);
	}
}
