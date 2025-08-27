/*

  ==============================================================================

	JAdvancedDock.h - Enhanced Edition
	Created: 6 Jul 2016 10:27:02pm
	Author:  jim
	Enhanced: Modern UI with animations, gradients, and improved UX
	Extended: Added support for adding components to new columns

  ==============================================================================
*/

#ifndef ADVANCEDDOCK_H_INCLUDED
#define ADVANCEDDOCK_H_INCLUDED

#include "JDockableWindows.h"

namespace AdvancedDockPlaces
{
	enum Places
	{
		top, left, right, bottom, centre,
		none
	};
};

// Forward declarations
class AdvancedDockPlacementDialog;
class ModernTitleBar;

/**
The advanced dock allows vertical and horizontal splits, as well as tabs.
Enhanced with modern UI elements, smooth animations, and improved visual styling.
*/
class JAdvancedDock
	:
	public Component,
	DockBase,
	public DragAndDropContainer
{
public:
	JAdvancedDock(DockableWindowManager& manager_);

	~JAdvancedDock();

	/** Adds a component to the dock so it's visible to the user.

	 We assume you are managing the components lifetime. However an optional
	 change could be to have the DockManager manage them.
	 */
	void addComponentToDock(Component* component);

	/** Adds a component to the dock so it's visible to the user.

	 @param rowPosition - where to insert the new row. If rowPosition is -1 it
	 will be inserted at the bottom. Rows are numbered from the top down. A row
	 position of 0 will insert the component at the top. */
	void addComponentToNewRow(Component* component, int rowPosition);

	// Column management methods with optional width specification
	void addComponentToNewColumn(Component* component, int rowIndex, int columnPosition);
	void addComponentToNewColumn(Component* component, int rowIndex, int columnPosition, double width);
	void addComponentToNewColumnInFirstRow(Component* component, int columnPosition);
	void addComponentToNewColumnInFirstRow(Component* component, int columnPosition, double width);
	void addComponentToNewColumnAtEnd(Component* component, int rowIndex);
	void addComponentToNewColumnAtEnd(Component* component, int rowIndex, double width);
	void addComponentToNewColumnBeforeExisting(Component* component, int rowIndex, int beforeColumnIndex);
	void addComponentToNewColumnAfterExisting(Component* component, int rowIndex, int afterColumnIndex);
	void addComponentToNewColumnBeforeExisting(Component* component, int rowIndex, int beforeColumnIndex, double width);
	void addComponentToNewColumnAfterExisting(Component* component, int rowIndex, int afterColumnIndex, double width);

	// Column width management
	void setColumnWidth(int rowIndex, int columnIndex, double width);
	double getColumnWidth(int rowIndex, int columnIndex) const;

	// Layout information queries
	int getColumnCountInRow(int rowIndex) const;
	int getRowCount() const;
	bool isValidRowIndex(int rowIndex) const;
	bool isValidColumnIndex(int rowIndex, int columnIndex) const;

	// Title bar management
	void setDockTitle(const String& title);
	void setDockActive(bool isActive);
	String getDockTitle() const;

	// Component overrides
	void resized() override;
	void paint(Graphics& g) override;

private:
	struct WindowLocation
	{
		WindowLocation(int y, int x, int t);
		int y{ 0 };
		int x{ 0 };
		int tab{ 0 };
	};

	WindowLocation getWindowLocationAtPoint(const Point<int>& screenPosition);
	Rectangle<int> getWindowBoundsAtPoint(const Point<int>& p);

	/**
	Insert a new window in to the right place in our dock...
	*/
	void insertWindow(const Point<int>& screenPos, AdvancedDockPlaces::Places places, DockableComponentWrapper* comp);

	// DockBase overrides
	void showDockableComponentPlacement(DockableComponentWrapper* component, Point<int> screenPosition) override;
	void hideDockableComponentPlacement() override;
	void startDockableComponentDrag(DockableComponentWrapper* component) override;
	bool attachDockableComponent(DockableComponentWrapper* component, Point<int> screenPosition) override;
	void detachDockableComponent(DockableComponentWrapper* component) override;
	void revealComponent(DockableComponentWrapper* dockableComponent) override;

	// Internal insertion methods
	void insertNewDock(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc, double width);
	void insertNewRow(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc);
	void insertToNewTab(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc);

	// Forward declarations for internal classes
	class RowType;

	// Layout management
	void rebuildRowResizers();
	void layoutRows(const Rectangle<int>& area);

	// Member variables
	std::vector<RowType> rows;
	std::vector<std::unique_ptr<StretchableLayoutResizerBar>> resizers;
	StretchableLayoutManager layout;

	DockableWindowManager& manager;
	AdvancedDockPlacementDialog* placementDialog;
	ModernTitleBar* titleBar;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JAdvancedDock)
};

#endif  // ADVANCEDDOCK_H_INCLUDED