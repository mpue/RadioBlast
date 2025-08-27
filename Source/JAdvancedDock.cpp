/*
  ==============================================================================

	JAdvancedDock.cpp - Enhanced Edition
	Created: 6 Jul 2016 10:27:02pm
	Author:  jim
	Enhanced: Modern UI with animations, gradients, and improved UX
	Extended: Added support for adding components to new columns

  ==============================================================================
*/

#include "JAdvancedDock.h"

// Modern color palette
namespace ModernColors
{
	const Colour primaryBg = Colour(0xff1a1a1a);      // Dark background
	const Colour secondaryBg = Colour(0xff2d2d2d);    // Slightly lighter
	const Colour accent = Colour(0xff4a9eff);         // Modern blue
	const Colour accentHover = Colour(0xff6bb6ff);    // Lighter blue
	const Colour surface = Colour(0xff363636);        // Panel surfaces
	const Colour border = Colour(0xff404040);         // Subtle borders
	const Colour text = Colour(0xffe0e0e0);           // Light text
	const Colour textSecondary = Colour(0xffb0b0b0);  // Secondary text
	const Colour success = Colour(0xff4caf50);        // Success green
	const Colour warning = Colour(0xffff9800);        // Warning orange
}

/**
Enhanced row management with smooth animations and modern styling
*/
class JAdvancedDock::RowType
{
public:
	RowType()
	{
		layout = std::make_unique<StretchableLayoutManager>();
		fadeAnimator.reset(new ComponentAnimator());
	}

	RowType(RowType&& rhs)
	{
		resizers = std::move(rhs.resizers);
		columns = std::move(rhs.columns);
		layout = std::move(rhs.layout);
		fadeAnimator = std::move(rhs.fadeAnimator);
	}

	RowType& operator=(RowType&& rhs)
	{
		resizers = std::move(rhs.resizers);
		columns = std::move(rhs.columns);
		layout = std::move(rhs.layout);
		fadeAnimator = std::move(rhs.fadeAnimator);
		return *this;
	}

	typedef std::vector<DockableComponentWrapper*> TabDockType;

	void add(TabDockType&& newTabDock, int position, Component* parent, double initialWidth = -1.0)
	{
		columns.insert(columns.begin() + position, std::move(newTabDock));

		// Set initial width if specified
		if (initialWidth > 0.0 && !newTabDock.empty())
		{
			newTabDock[0]->setSize(static_cast<int>(initialWidth), newTabDock[0]->getHeight());
		}

		// Animate new component appearance
		if (!newTabDock.empty())
		{
			auto* comp = newTabDock[0];
			comp->setAlpha(0.0f);
			fadeAnimator->animateComponent(comp, comp->getBounds(), 1.0f, 300, false, 0.8, 0.1);
		}

		rebuildResizers(parent);
	}

	/**
	Called from the docks resize method.
	*/
	void layoutColumns(const Rectangle<int>& area)
	{
		std::vector<Component*> comps;

		for (int i = 0; i < columns.size(); ++i)
		{
			comps.push_back(columns[i].front());

			if (i < resizers.size())
				comps.push_back(resizers[i].get());
		}

		if (comps.size() == 1)
		{
			comps[0]->setBounds(area);
		}
		else
		{
			layout->layOutComponents(comps.data(), comps.size(),
				area.getX(), area.getY(), area.getWidth(), area.getHeight(), false, true);
		}

		/* Make all other tabs match our new size...*/
		for (auto& c : columns)
		{
			if (c.size() > 1)
				for (auto& t : c)
					t->setBounds(c[0]->getBounds());
		}
	}

	// Enhanced resizer bar with modern styling
	class EnhancedResizerBar : public StretchableLayoutResizerBar
	{
	public:
		EnhancedResizerBar(StretchableLayoutManager* layout, int index, bool vertical)
			: StretchableLayoutResizerBar(layout, index, vertical), isHovered(false)
		{
			setMouseCursor(vertical ? MouseCursor::LeftRightResizeCursor : MouseCursor::UpDownResizeCursor);
		}

		void paint(Graphics& g) override
		{
			auto bounds = getLocalBounds().toFloat();

			// Modern gradient background
			ColourGradient gradient(
				isHovered ? ModernColors::accent.withAlpha(0.8f) : ModernColors::border.withAlpha(0.6f),
				bounds.getCentreX(), bounds.getY(),
				isHovered ? ModernColors::accentHover.withAlpha(0.6f) : ModernColors::border.withAlpha(0.3f),
				bounds.getCentreX(), bounds.getBottom(),
				false
			);

			g.setGradientFill(gradient);
			g.fillRoundedRectangle(bounds.reduced(1.0f), 2.0f);

			// Subtle highlight
			if (isHovered)
			{
				g.setColour(ModernColors::accent.withAlpha(0.3f));
				g.fillRoundedRectangle(bounds.reduced(2.0f), 1.0f);
			}
		}

		void mouseEnter(const MouseEvent&) override
		{
			isHovered = true;
			repaint();
		}

		void mouseExit(const MouseEvent&) override
		{
			isHovered = false;
			repaint();
		}

	private:
		bool isHovered;
	};

	void rebuildResizers(Component* parent)
	{
		const double resizerWidth = 8.0;
		resizers.clear();

		int itemIndex = 0;

		for (int pos = 0; pos < columns.size(); ++pos)
		{
			double columnWidth = columns[pos][0]->getWidth();
			// Use a minimum width if the component width is too small
			if (columnWidth < 10.0)
				columnWidth = 250.0; // Increased default width

			layout->setItemLayout(itemIndex++, 50.0, 2000.0, columnWidth);

			if (pos < columns.size() - 1)
			{
				auto resizer = std::make_unique<EnhancedResizerBar>(layout.get(), pos * 2 + 1, true);
				parent->addAndMakeVisible(resizer.get());
				resizers.push_back(std::move(resizer));
				layout->setItemLayout(itemIndex++, resizerWidth, resizerWidth, resizerWidth);
			}
		}
	}

	/** Sets up the correct tab configuration for a docked component that needs to display tabs */
	static void configureTabs(const TabDockType& vector)
	{
		int tabXPos = 0;
		const int tabSpacing = 4; // Increased spacing for modern look

		struct FrontComponent
		{
			int zOrder{ -1 };
			int x{ 0 };
			DockableComponentWrapper* component{ nullptr };
		};

		FrontComponent frontComponent;

		if (vector.size() < 2)
			return;

		for (auto& dockedCompWrapper : vector)
		{
			if (dockedCompWrapper->isVisible())
			{
				auto parent = dockedCompWrapper->getParentComponent();
				auto myIndex = parent->getIndexOfChildComponent(dockedCompWrapper);

				if (myIndex > frontComponent.zOrder)
				{
					frontComponent.zOrder = myIndex;
					frontComponent.component = dockedCompWrapper;
					frontComponent.x = tabXPos;
				}

				dockedCompWrapper->setShowTabButton(true, tabXPos, false);
				tabXPos += dockedCompWrapper->getTabWidth() + tabSpacing;
			}
		}

		if (frontComponent.zOrder != -1)
			frontComponent.component->setShowTabButton(true, frontComponent.x, true);
	}

	std::unique_ptr<StretchableLayoutManager> layout;
	std::vector<TabDockType> columns;
	std::vector<std::unique_ptr<EnhancedResizerBar>> resizers;
	std::unique_ptr<ComponentAnimator> fadeAnimator;
};

/**
Modern placement dialog with glassmorphism effects and smooth animations
*/
class AdvancedDockPlacementDialog : public Component, private Timer
{
public:
	AdvancedDockPlacementDialog() : pulsePhase(0.0f)
	{
		for (int i = AdvancedDockPlaces::top; i <= AdvancedDockPlaces::centre; ++i)
			buttons.add(new PlacementButton(AdvancedDockPlaces::Places(i)));

		for (auto b : buttons)
			addAndMakeVisible(b);

		setAlpha(0.0f);
		startTimer(50); // Smooth animation timer
	}

	void setVisible(bool shouldBeVisible) override
	{
		if (shouldBeVisible != isVisible())
		{
			Component::setVisible(shouldBeVisible);

			if (shouldBeVisible)
			{
				// Fade in with scale animation
				setAlpha(0.0f);
				setTransform(AffineTransform::scale(0.8f).translated(getBounds().getCentreX() * 0.2f, getBounds().getCentreY() * 0.2f));

				Desktop::getInstance().getAnimator().animateComponent(this, getBounds(), 1.0f, 300, false, 0.8, 0.1);
				setTransform(AffineTransform());
			}
			else
			{
				// Fade out
				Desktop::getInstance().getAnimator().animateComponent(this, getBounds(), 0.0f, 200, true, 1.0, 0.0);
			}
		}
	}

	void setLeftRightVisible(bool nowVisible)
	{
		buttons[AdvancedDockPlaces::left]->setVisible(nowVisible);
		buttons[AdvancedDockPlaces::right]->setVisible(nowVisible);
	}

	void setTopBottomVisible(bool nowVisible)
	{
		buttons[AdvancedDockPlaces::top]->setVisible(nowVisible);
		buttons[AdvancedDockPlaces::bottom]->setVisible(nowVisible);
	}

	void setMousePosition(const Point<int>& screenPos)
	{
		auto p = getLocalPoint(nullptr, screenPos);

		for (auto b : buttons)
			b->setIsMouseOver(b->contains(b->getLocalPoint(this, p)));

		repaint();
	}

	class PlacementButton : public Component
	{
	public:
		PlacementButton(AdvancedDockPlaces::Places place) : place(place), mouseOver(false), glowIntensity(0.0f)
		{
		}

		void paint(Graphics& g) override
		{
			auto bounds = getLocalBounds().toFloat();
			auto reducedBounds = bounds.reduced(3.0f);

			// Glassmorphism background
			g.setColour(ModernColors::surface.withAlpha(0.7f));
			g.fillRoundedRectangle(reducedBounds, 6.0f);

			// Border with glow effect
			if (mouseOver)
			{
				// Outer glow
				g.setColour(ModernColors::accent.withAlpha(glowIntensity * 0.5f));
				g.drawRoundedRectangle(reducedBounds.expanded(2.0f), 8.0f, 3.0f);

				// Inner highlight
				g.setColour(ModernColors::accentHover.withAlpha(0.8f));
				g.fillRoundedRectangle(reducedBounds, 6.0f);
			}

			// Clean border
			g.setColour(ModernColors::border.withAlpha(0.8f));
			g.drawRoundedRectangle(reducedBounds, 6.0f, 1.0f);

			// Icon representation
			auto iconBounds = reducedBounds.reduced(8.0f);
			g.setColour(mouseOver ? ModernColors::text : ModernColors::textSecondary);

			switch (place)
			{
			case AdvancedDockPlaces::top:
				g.fillRoundedRectangle(iconBounds.removeFromTop(3.0f), 1.5f);
				break;
			case AdvancedDockPlaces::bottom:
				g.fillRoundedRectangle(iconBounds.removeFromBottom(3.0f), 1.5f);
				break;
			case AdvancedDockPlaces::left:
				g.fillRoundedRectangle(iconBounds.removeFromLeft(3.0f), 1.5f);
				break;
			case AdvancedDockPlaces::right:
				g.fillRoundedRectangle(iconBounds.removeFromRight(3.0f), 1.5f);
				break;
			case AdvancedDockPlaces::centre:
				g.fillEllipse(iconBounds.reduced(2.0f));
				break;
			}
		}

		void setIsMouseOver(bool isOver)
		{
			if (isOver != mouseOver)
			{
				mouseOver = isOver;
				repaint();
			}
		}

		void setGlowIntensity(float intensity)
		{
			glowIntensity = intensity;
			if (mouseOver) repaint();
		}

		bool mouseOver;
		float glowIntensity;
		AdvancedDockPlaces::Places place;
	};

	void resized() override
	{
		auto area = getLocalBounds().toFloat().reduced(padding);
		auto h3 = area.getHeight() / 3.0f;
		auto w3 = area.getWidth() / 3.0f;

		topArea = area.removeFromTop(h3).translated(h3, 0.0f).withWidth(h3);
		buttons[AdvancedDockPlaces::top]->setBounds(topArea.toNearestInt());

		midArea = area.removeFromTop(h3);

		auto midAreaChop = midArea;

		buttons[AdvancedDockPlaces::left]->setBounds(midAreaChop.removeFromLeft(w3).toNearestInt());
		centreArea = midAreaChop.removeFromLeft(w3);
		buttons[AdvancedDockPlaces::centre]->setBounds(centreArea.toNearestInt());
		buttons[AdvancedDockPlaces::right]->setBounds(midAreaChop.toNearestInt());

		bottomArea = area.translated(h3, 0.0f).withWidth(h3);
		buttons[AdvancedDockPlaces::bottom]->setBounds(bottomArea.toNearestInt());
	}

	AdvancedDockPlaces::Places getSelectionForCoordinates(Point<int> position) const
	{
		for (auto b : buttons)
			if (b->getBounds().contains(position) && b->isVisible())
				return b->place;

		return AdvancedDockPlaces::none;
	}

	void paint(Graphics& g) override
	{
		// Modern backdrop with blur effect
		g.setColour(ModernColors::primaryBg.withAlpha(0.85f));

		if (buttons[AdvancedDockPlaces::top]->isVisible() && buttons[AdvancedDockPlaces::left]->isVisible())
		{
			g.fillRoundedRectangle(topArea.expanded(padding), rounding);
			g.fillRoundedRectangle(midArea.expanded(padding), rounding);
			g.fillRoundedRectangle(bottomArea.expanded(padding), rounding);
		}
		else
		{
			g.fillRoundedRectangle(centreArea.expanded(padding), rounding);
		}

		// Subtle border
		g.setColour(ModernColors::border.withAlpha(0.5f));
		auto bounds = getLocalBounds().toFloat().reduced(1.0f);
		g.drawRoundedRectangle(bounds, rounding + padding, 1.0f);
	}

private:
	void timerCallback() override
	{
		pulsePhase += 0.1f;
		float intensity = (sin(pulsePhase) + 1.0f) * 0.5f;

		for (auto button : buttons)
			button->setGlowIntensity(intensity);
	}

	Rectangle<float> topArea, midArea, bottomArea, centreArea;
	OwnedArray<PlacementButton> buttons;
	const float rounding = 8.0f;
	const float padding = 6.0f;
	float pulsePhase;
};

JAdvancedDock::JAdvancedDock(DockableWindowManager& manager_) : DockBase(manager_, this), manager(manager_)
{
	placementDialog = new AdvancedDockPlacementDialog();
	addChildComponent(placementDialog);

	// Set modern background
	setOpaque(true);
}

JAdvancedDock::~JAdvancedDock()
{
}

JAdvancedDock::WindowLocation::WindowLocation(int y, int x, int t) : y(y), x(x), tab(t)
{
}

JAdvancedDock::WindowLocation JAdvancedDock::getWindowLocationAtPoint(const Point<int>& screenPosition)
{
	auto localPos = getLocalPoint(nullptr, screenPosition);

	for (int y = 0; y < rows.size(); ++y)
	{
		auto& row = rows[y];

		for (int x = 0; x < row.columns.size(); ++x)
		{
			auto& col = row.columns[x];

			for (int z = 0; z < col.size(); ++z)
			{
				auto& tab = col[z];

				if (tab->getBounds().contains(localPos))
					return { y, x, z };
			}
		}
	}

	return { 0,0,0 };
}

Rectangle<int> JAdvancedDock::getWindowBoundsAtPoint(const Point<int>& p)
{
	auto loc = getWindowLocationAtPoint(p);

	if (rows.size() == 0)
		return getLocalBounds();

	return rows[loc.y].columns[loc.x][loc.tab]->getBounds();
}

void JAdvancedDock::showDockableComponentPlacement(DockableComponentWrapper* component, Point<int> screenPosition)
{
	placementDialog->setTopBottomVisible(rows.size() > 0);
	placementDialog->setLeftRightVisible(rows.size() > 0);

	placementDialog->setVisible(true);
	placementDialog->toFront(false);
	placementDialog->setBounds(getWindowBoundsAtPoint(screenPosition).withSizeKeepingCentre(100, 100));
	placementDialog->setMousePosition(screenPosition);
}

void JAdvancedDock::hideDockableComponentPlacement()
{
	placementDialog->setVisible(false);
}

void JAdvancedDock::startDockableComponentDrag(DockableComponentWrapper* component)
{
}

void JAdvancedDock::insertNewDock(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc, double initialWidth)
{
	auto& row = rows[loc.y];
	RowType::TabDockType newTabDock;
	newTabDock.push_back(comp);

	// Set the component width before adding to dock
	if (initialWidth > 0.0)
	{
		comp->setSize(static_cast<int>(initialWidth), comp->getHeight());
	}

	row.add(std::move(newTabDock), loc.x, this, initialWidth);
}

void JAdvancedDock::insertNewRow(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc)
{
	RowType newRow;
	newRow.columns.push_back(RowType::TabDockType());
	newRow.columns[0].push_back(comp);
	rows.insert(rows.begin() + loc.y, std::move(newRow));
	rebuildRowResizers();
}

void JAdvancedDock::insertToNewTab(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc)
{
	auto& location = rows[loc.y].columns[loc.x];
	location.push_back(comp);
	RowType::configureTabs(location);
}

void JAdvancedDock::insertWindow(const Point<int>& screenPos, AdvancedDockPlaces::Places places, DockableComponentWrapper* comp)
{
	auto loc = getWindowLocationAtPoint(screenPos);

	switch (places)
	{
	case AdvancedDockPlaces::top:
		insertNewRow(comp, loc);
		break;

	case AdvancedDockPlaces::bottom:
		loc.y += rows.size() > 0 ? 1 : 0;
		insertNewRow(comp, loc);
		break;

	case AdvancedDockPlaces::left:
		insertNewDock(comp, loc, 200);
		break;

	case AdvancedDockPlaces::right:
		loc.x++;
		insertNewDock(comp, loc, 200);
		break;

	case AdvancedDockPlaces::centre:
		if (rows.size() > 0)
			insertToNewTab(comp, loc);
		else
			insertNewRow(comp, loc);
		break;

	case AdvancedDockPlaces::none:
		jassertfalse;
		break;
	}
}

void JAdvancedDock::addComponentToDock(Component* comp)
{
	auto dockable = manager.createDockableComponent(comp);
	addAndMakeVisible(dockable);
	auto loc = WindowLocation{ 0, 0, 0 };

	if (rows.size() > 0)
		insertToNewTab(dockable, loc);
	else
		insertNewRow(dockable, loc);

	resized();
}

void JAdvancedDock::addComponentToNewRow(Component* component, int rowPosition)
{
	auto dockable = manager.createDockableComponent(component);
	addAndMakeVisible(dockable);

	if (rowPosition > rows.size())
		rowPosition = rows.size();

	auto loc = WindowLocation{ rowPosition, 0, 0 };

	insertNewRow(dockable, loc);

	resized();
}

void JAdvancedDock::addComponentToNewColumn(Component* component, int rowIndex, int columnPosition, double width)
{
	auto dockable = manager.createDockableComponent(component);
	addAndMakeVisible(dockable);

	// Ensure we have at least one row
	if (rows.size() == 0)
	{
		addComponentToNewRow(component, 0);
		return;
	}

	// Clamp row index to valid range
	if (rowIndex >= rows.size())
		rowIndex = rows.size() - 1;
	if (rowIndex < 0)
		rowIndex = 0;

	auto& row = rows[rowIndex];

	// Clamp column position to valid range
	if (columnPosition > row.columns.size())
		columnPosition = row.columns.size();
	if (columnPosition < 0)
		columnPosition = 0;

	auto loc = WindowLocation{ rowIndex, columnPosition, 0 };
	insertNewDock(dockable, loc, width);

	resized();
}

void JAdvancedDock::addComponentToNewColumnInFirstRow(Component* component, int columnPosition, double width)
{
	addComponentToNewColumn(component, 0, columnPosition, width);
}

void JAdvancedDock::addComponentToNewColumnAtEnd(Component* component, int rowIndex, double width)
{
	if (rows.size() == 0)
	{
		addComponentToNewRow(component, 0);
		return;
	}

	// Clamp row index to valid range
	if (rowIndex >= rows.size())
		rowIndex = rows.size() - 1;
	if (rowIndex < 0)
		rowIndex = 0;

	auto& row = rows[rowIndex];
	addComponentToNewColumn(component, rowIndex, row.columns.size(), width);
}

void JAdvancedDock::addComponentToNewColumnBeforeExisting(Component* component, int rowIndex, int beforeColumnIndex, double width)
{
	addComponentToNewColumn(component, rowIndex, beforeColumnIndex, width);
}

void JAdvancedDock::addComponentToNewColumnAfterExisting(Component* component, int rowIndex, int afterColumnIndex, double width)
{
	addComponentToNewColumn(component, rowIndex, afterColumnIndex + 1, width);
}

// OVERLOADED METHODS WITHOUT WIDTH (use default sizing)

void JAdvancedDock::addComponentToNewColumn(Component* component, int rowIndex, int columnPosition)
{
	addComponentToNewColumn(component, rowIndex, columnPosition, -1.0);
}

void JAdvancedDock::addComponentToNewColumnInFirstRow(Component* component, int columnPosition)
{
	addComponentToNewColumn(component, 0, columnPosition, -1.0);
}

void JAdvancedDock::addComponentToNewColumnAtEnd(Component* component, int rowIndex)
{
	addComponentToNewColumnAtEnd(component, rowIndex, -1.0);
}

void JAdvancedDock::addComponentToNewColumnBeforeExisting(Component* component, int rowIndex, int beforeColumnIndex)
{
	addComponentToNewColumn(component, rowIndex, beforeColumnIndex, -1.0);
}

void JAdvancedDock::addComponentToNewColumnAfterExisting(Component* component, int rowIndex, int afterColumnIndex)
{
	addComponentToNewColumn(component, rowIndex, afterColumnIndex + 1, -1.0);
}

// METHODS FOR SETTING COLUMN WIDTH AFTER CREATION

void JAdvancedDock::setColumnWidth(int rowIndex, int columnIndex, double width)
{
	if (!isValidColumnIndex(rowIndex, columnIndex))
		return;

	auto& column = rows[rowIndex].columns[columnIndex];
	if (!column.empty())
	{
		// Set the width for all components in the column
		for (auto& comp : column)
		{
			comp->setSize(static_cast<int>(width), comp->getHeight());
		}

		// Rebuild the row's resizers to update layout
		rows[rowIndex].rebuildResizers(this);
		resized();
	}
}

double JAdvancedDock::getColumnWidth(int rowIndex, int columnIndex) const
{
	if (!isValidColumnIndex(rowIndex, columnIndex))
		return 0.0;

	auto& column = rows[rowIndex].columns[columnIndex];
	if (!column.empty())
		return static_cast<double>(column[0]->getWidth());

	return 0.0;
}

// UTILITY METHODS FOR COLUMN MANAGEMENT

int JAdvancedDock::getColumnCountInRow(int rowIndex) const
{
	if (rowIndex < 0 || rowIndex >= rows.size())
		return 0;

	return rows[rowIndex].columns.size();
}

int JAdvancedDock::getRowCount() const
{
	return rows.size();
}

bool JAdvancedDock::isValidRowIndex(int rowIndex) const
{
	return rowIndex >= 0 && rowIndex < rows.size();
}

bool JAdvancedDock::isValidColumnIndex(int rowIndex, int columnIndex) const
{
	if (!isValidRowIndex(rowIndex))
		return false;

	return columnIndex >= 0 && columnIndex < rows[rowIndex].columns.size();
}

// EXISTING METHODS (unchanged)

bool JAdvancedDock::attachDockableComponent(DockableComponentWrapper* component, Point<int> screenPosition)
{
	auto placement = placementDialog->getSelectionForCoordinates(placementDialog->getLocalPoint(nullptr, screenPosition));

	if (placement != AdvancedDockPlaces::none)
	{
		addAndMakeVisible(component);
		insertWindow(screenPosition, placement, component);
		resized();
		return true;
	}

	return false;
}

void JAdvancedDock::detachDockableComponent(DockableComponentWrapper* component)
{
	for (int y = 0; y < rows.size(); ++y)
	{
		auto& row = rows[y];

		for (int x = 0; x < row.columns.size(); ++x)
		{
			auto& col = row.columns[x];

			for (int z = 0; z < col.size(); ++z)
			{
				auto& tab = col[z];

				if (tab == component)
				{
					col.erase(col.begin() + z);

					if (col.size() == 1)
					{
						/* remove tab buttons if we don't need them any more */
						col[0]->setShowTabButton(false, 0, false);
					}
					else if (col.size() == 0)
					{
						/* remove tabs, rows and columns if now empty... */
						row.columns.erase(row.columns.begin() + x);

						if (row.columns.size() == 0)
							rows.erase(rows.begin() + y);
					}

					row.rebuildResizers(this);
					rebuildRowResizers();
					resized();

					return;
				}
			}
		}
	}
}

void JAdvancedDock::revealComponent(DockableComponentWrapper* dockableComponent)
{
	dockableComponent->toFront(true);

	for (auto& r : rows)
		for (auto& c : r.columns)
			RowType::configureTabs(c);

	resized();
}

void JAdvancedDock::rebuildRowResizers()
{
	const double resizerSize = 8.0;
	resizers.clear();

	int itemIndex = 0;

	for (int pos = 0; pos < rows.size(); ++pos)
	{
		layout.setItemLayout(itemIndex++, 50.0, 2000.0, rows[0].columns[0][0]->getHeight());

		if (pos < rows.size() - 1)
		{
			auto resizer = std::make_unique<StretchableLayoutResizerBar>(&layout, pos * 2 + 1, false);
			resizer->setMouseCursor(MouseCursor::UpDownResizeCursor);
			addAndMakeVisible(resizer.get());
			resizers.push_back(std::move(resizer));
			layout.setItemLayout(itemIndex++, resizerSize, resizerSize, resizerSize);
		}
	}
}

void JAdvancedDock::layoutRows(const Rectangle<int>& area)
{
	std::vector<Component*> comps;

	for (int i = 0; i < rows.size(); ++i)
	{
		comps.push_back(rows[i].columns.front().front());

		if (i < resizers.size())
			comps.push_back(resizers[i].get());
	}

	if (comps.size() == 1)
	{
		comps[0]->setBounds(area); // layoutComponents doesn't seem to cope with only one component passed to it.
	}
	else
	{
		layout.layOutComponents(comps.data(), comps.size(),
			area.getX(), area.getY(), area.getWidth(), area.getHeight(), true, false);
	}
}

void JAdvancedDock::resized()
{
	if (rows.size() == 0)
		return;

	auto area = getLocalBounds();

	for (auto& resizer : resizers)
		resizer->setSize(area.getWidth(), 8);

	layoutRows(area);

	for (auto& row : rows)
	{
		if (row.columns.size() == 0)
			continue; // shouldn't happen, but just for safety...

		auto area2 = row.columns[0][0]->getBounds().withWidth(area.getWidth()).withX(0); // the first component will have had bounds set by the row resizer so we copy these over.

		row.layoutColumns(area2);
	}
}

void JAdvancedDock::paint(Graphics& g)
{
	// Modern gradient background
	if (rows.size() == 0)
	{
		// Create attractive empty state
		ColourGradient gradient(
			ModernColors::primaryBg, 0.0f, 0.0f,
			ModernColors::secondaryBg, 0.0f, getHeight() * 0.6f,
			false
		);

		g.setGradientFill(gradient);
		g.fillAll();

		// Modern welcome message
		auto bounds = getLocalBounds();
		auto textArea = bounds.reduced(40);

		g.setColour(ModernColors::accent.withAlpha(0.1f));
		g.fillRoundedRectangle(textArea.toFloat(), 12.0f);

		g.setColour(ModernColors::border);
		g.drawRoundedRectangle(textArea.toFloat(), 12.0f, 1.0f);

		g.setColour(ModernColors::text);
		g.setFont(Font(24.0f, Font::bold));
		g.drawText("Advanced Dock", textArea.removeFromTop(40), Justification::centred);

		g.setColour(ModernColors::textSecondary);
		g.setFont(Font(14.0f));
		g.drawText("Drag components here to create your layout", textArea, Justification::centred);

		// Subtle decoration
		auto decorArea = bounds.reduced(bounds.getWidth() / 3, bounds.getHeight() / 3);
		g.setColour(ModernColors::accent.withAlpha(0.05f));
		g.fillEllipse(decorArea.toFloat());
	}
	else
	{
		// Subtle background for active dock
		g.setColour(ModernColors::primaryBg);
		g.fillAll();

		// Add subtle grid pattern for visual interest
		g.setColour(ModernColors::border.withAlpha(0.1f));
		auto bounds = getLocalBounds();

		// Vertical lines
		for (int x = 50; x < bounds.getWidth(); x += 50)
		{
			g.drawVerticalLine(x, 0.0f, static_cast<float>(bounds.getHeight()));
		}

		// Horizontal lines
		for (int y = 50; y < bounds.getHeight(); y += 50)
		{
			g.drawHorizontalLine(y, 0.0f, static_cast<float>(bounds.getWidth()));
		}
	}
}